#include "SMPPGateManager.h"
#include "RequestTracker.h"
#include "MessageClassifier.h"
#include "SMPPGateFilterParser.h"
#include "RequestTracker.h"
#include "StatManager.h"
#include "PartnerManager.h"
#include "ConfigManager.h"

#include <algorithm>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

namespace sms {

    SMPPGateManager* SMPPGateManager::pInstance_;

    SMPPGateManager::SMPPGateManager():db( PGSqlConnPoolStats::get_mutable_instance().getdb() ) {
        boost::recursive_mutex::scoped_lock lck(lock);
        gen = boost::mt19937(static_cast<unsigned int>(std::time(0)));
        isSending = false;
        timerid = Timer::Instance()->addPeriodicEvent( boost::bind( &SMPPGateManager::gatesMapUpdate, this ), 60 );
        //updateid = Timer::Instance()->addPeriodicEvent( boost::bind( &SMPPGateManager::gatesPriorityUpdate, this ), 10 );
        sendid = Timer::Instance()->addPeriodicEvent( boost::bind( &SMPPGateManager::timeToSend, this ), 1 );
        gatesMapUpdate();
        timeToSend();
        kannel = boost::shared_ptr< HttpClient>( new HttpClient() );
    }

    SMPPGateManager::~SMPPGateManager() {
        Timer::Instance()->cancelEvent( timerid );
        Timer::Instance()->cancelEvent( updateid );
        Timer::Instance()->cancelEvent( sendid );
    }

    void SMPPGateManager::gatesMapUpdate() {
        std::ostringstream out;
        std::ostringstream gwlist_req;
        out << "SMPPGateManager::gatesMapUpdate() ";
        gwlist_req << "SELECT \"gName\" FROM gateways;";

        try {
            PGSql::ConnectionHolder cHold( db );
            ConnectionPTR conn = cHold.get();
            TransactionPTR tr = db.openTransaction( conn, "SMPPGateManager::gatesMapUpdate()" );
            Result res = tr->exec( gwlist_req.str() );
            tr->commit();

            for ( Result::size_type i = 0; i < res.size(); i++ ) {                
                boost::recursive_mutex::scoped_lock lck(lock);
                std::string gname = res[i][0].as<string>();
                SMPPGate g = SMPPGate::loadFromDb( db, gname );
                gmap[ gname ].reinit( gname, g.userName(), g.userPass(), g.gatePort(), g.gatePriority(), g.enabled(), g.gateRule(), g.gateProperties() );
            }
        } catch ( PGBrokenConnection& err ) {
            out << err.what();
            Logger::get_mutable_instance().smslogerr( out.str() );
        } catch ( PGSqlError& err ) {
            out << err.what();
            Logger::get_mutable_instance().smslogerr( out.str() );
        }
        out << "parsed;";
        Logger::get_mutable_instance().smsloginfo( out.str() );
    }

    std::list< string > SMPPGateManager::chooseGate( SMSRequest::PTR req, SMSMessage::ID msgid ) {
        SMPPGatesMap gateMap;
	bool isTrial = false;
        try {
            isTrial = PartnerManager::get_mutable_instance().findById( req->pid ).pIsTrial;
        } catch ( ... ) {}
        if ( isTrial ) {
	    std::list< string > trial_list;
            trial_list.push_back( "mt_null" );
	    return trial_list;
        }
        {
            boost::recursive_mutex::scoped_lock lck(lock);
            gateMap = gmap;
        }
        std::list< SMPPGatesMap::iterator > to_remove;
        std::list< string > to_remove_hist;

        // Удалим все "использованные" шлюзы

        SMSMessage::PTR msg = SMSMessageManager::get_mutable_instance().loadMessage( msgid );
        SMSMessage::HistoryType::iterator it;
        SMSMessage::HistoryType hist = msg->getHistory();
        for ( it = hist.begin(); it != hist.end(); it++ ) {
            if ( gateMap.find( it->gateway ) != gateMap.end() ) {
                to_remove_hist.push_back( it->gateway );
            }
        }
        for ( std::list< string >::iterator it = to_remove_hist.begin(); it != to_remove_hist.end(); it++ ) {
            gateMap.erase( *it );
        }
        to_remove_hist.clear();

        // Удалим все шлюзы, которые не подходят по фильтру
        for ( SMPPGatesMap::iterator itg = gateMap.begin(); itg != gateMap.end(); itg++ ) {
            if ( !canAccept( itg->first, req, msgid ) )
                to_remove.push_back( itg );
        }
        for ( std::list< SMPPGatesMap::iterator >::iterator it = to_remove.begin(); it != to_remove.end(); it++ ) {
            gateMap.erase( *it );
        }
        to_remove.clear();

        // Удалим все временно отключенные шлюзы, если такие есть - устанавливаем пометку
        bool disabled_exists = false;
        for ( SMPPGatesMap::iterator itg = gateMap.begin(); itg != gateMap.end(); itg++ ) {
            if ( !itg->second.enabled() ) {
                disabled_exists = true;
                to_remove.push_back( itg );
            }
        }
        for ( std::list< SMPPGatesMap::iterator >::iterator it = to_remove.begin(); it != to_remove.end(); it++ ) {
            gateMap.erase( *it );
        }
        to_remove.clear();

        std::list< std::string > gnames;
        for ( SMPPGatesMap::iterator itg = gateMap.begin(); itg != gateMap.end(); itg++ ) {
            gnames.push_back( itg->first );
        }

        // Отсортируем по приоритету
        std::vector< std::string > gnames_vec( gnames.begin(), gnames.end() );
        for ( int i = 0; i < gnames_vec.size(); i++ ) {
            int lmax = gateMap[ gnames_vec[i] ].gatePriority();
            int jmax = i;
            for ( int j = i; j < gnames_vec.size(); j++ ) {
                int l = gateMap[ gnames_vec[j] ].gatePriority();
                if ( l >= lmax ) {
                    lmax = gateMap[ gnames_vec[j] ].gatePriority();
                    jmax = j;
                }
            }
            string tmp;
            tmp = gnames_vec[ i ];
            gnames_vec[ i ] = gnames_vec[ jmax ];
            gnames_vec[ jmax ] = tmp;
        }

        return std::list< string > ( gnames_vec.begin(), gnames_vec.end() );
    }

    string SMPPGateManager::suggestUrl( std::string sg, SMSMessage::ID msgid ) {

        ConfigManager* cm = ConfigManager::Instance();

        string jserver = cm->getProperty<std::string>("smsgate.server");
        string jport = cm->getProperty<std::string>("smsgate.port");
        string jrcv = cm->getProperty<std::string>("smsgate.scriptname");
        string jrcvpass = cm->getProperty<std::string>("smsgate.rcvpass");

        std::ostringstream back_url;
        SMSMessage::PTR msg = SMSMessageManager::get_mutable_instance().loadMessage( msgid );
        back_url
                << "http://" << jserver << ":" << jport << jrcv
                << "?pass=" << jrcvpass << "&gate=" << sg
                << "&result=%d&id="
                << msg->getID().req << "." << msg->getID().msg_num;

        std::ostringstream out;
        out
                << "&user=" << utils::UrlEncodeString(gmap[ sg ].userName())
                << "&pass=" << utils::UrlEncodeString(gmap[ sg ].userPass())
                << "&dlr-mask=31&dlr-url="
                << utils::UrlEncodeString(back_url.str());

        return out.str();
    }

    void SMPPGateManager::send( SMSRequest::PTR req, SMSMessage::ID msgid, string gName ) {
        RequestTracker* trck = RequestTracker::Instance();

        SMSMessage::PTR msg = SMSMessageManager::get_mutable_instance().loadMessage( msgid );
        //string gName = *chooseGate( req, msgid ).begin();
        string idp = req->pid;
        int ma_p = 0;
        trck->del_queue.push( RequestTracker::SMSOperation::create<RequestTracker::OP_CheckACK>( std::make_pair( std::make_pair( req, msgid ), gName), idp, ma_p, RequestTracker::OP_CheckACKP ), ConfigManager::Instance()->getProperty<int>( "system.acktimeout" ) );

        std::ostringstream out;
        out << "OP_SendMessage ID=" << msg->getID().to_str() << " phase 3: sending to smsc [" << gName << "] ";
        int rdelay = ConfigManager::Instance()->getProperty<int>( "system.resendtimeout" ); // TODO

        string url = suggestUrl( gName, msgid );

        bool ack_found = false;
        SMSMessage::HistoryType hst = msg->getHistory();
        SMSMessage::HistoryType::iterator it;

        for ( it = hst.begin(); it != hst.end(); it++ ) {
            if ( ( it->op_code == 1 ) && ( it->op_direction == 1 ) && ( it->gateway == gName ) )
                ack_found = true;
        }

        if ( ack_found ) {
            out << "Using gateway that was already used gateway=[" << gName << "] ";
            Logger::get_mutable_instance().smslogwarn( out.str() );
            return;
        }

        try {
            string num = req->to[ msg->getID().msg_num ];

            if ( gName == "mt_skyline" ) req->from = "1312";
            if ( !msg->getMsgClass().operators.empty() && (msg->getMsgClass().operators.begin()->second.getCode() == "40102") ) req->from = "79852970920";

            bool isTrial = false;
            try {
                isTrial = PartnerManager::get_mutable_instance().findById( req->pid ).pIsTrial;
            } catch ( ... ) {}

            if ( isTrial ) {
                trck->registerDeliveryNotification( msgid, SMSMessage::Status::ST_BUFFERED, "mt_null" );
                trck->registerDeliveryNotification( msgid, SMSMessage::Status::ST_DELIVERED, "mt_null" );
            } else
            if ( gName == "mt_uaks" ) {
                std::ostringstream sms_send;
                sms_send << "<message mid=\"" << msg->getID().to_str() << "\" paid=\"2000\">"
                         << "<sn>" << req->from << "</sn>"
                         << "<sin>" << msg->getPhone() << "</sin>"
                         << "<body content-type=\"text/plain\">" << req->msg << "</body>"
                         << "</message>";
                HttpClient::Response resp =kannel->post( "http://smsonline:password@sdp1.cpa.net.ua:8080/cpa2/receiver", sms_send.str() );

                boost::xtime now;
                boost::xtime_get( &now, boost::TIME_UTC );

                if ( resp.body.find( "Accepted" ) != std::string::npos ) {
                    trck->registerDeliveryNotification( msgid, SMSMessage::Status::ST_BUFFERED, gName );
                } else
                if ( resp.body.find( "Rejected" ) != std::string::npos ) {
                    trck->registerDeliveryNotification( msgid, SMSMessage::Status::ST_REJECTED, gName );
                } else
                {
                    gmap[ gName ].suspend();
                    BOOST_THROW_EXCEPTION( HttpError() << throw_descr( resp.body.c_str() ) );
                }

            } else  
            if ( gName == "mt_smsonline" ) {
                std::ostringstream sms_send;
                sms_send << "http://sms.smsonline.ru/mt.cgi?user=green&pass=dj8940fdcec3&utf=1"
                         << "&to=" << msg->getPhone() 
			 << "&msg=" << req->msg;
                HttpClient::Response resp =kannel->get( sms_send.str() );

                boost::xtime now;
                boost::xtime_get( &now, boost::TIME_UTC );

                if ( resp.body.find( "<code>0</code>" ) != std::string::npos ) {
                    trck->registerDeliveryNotification( msgid, SMSMessage::Status::ST_BUFFERED, gName );
                    trck->registerDeliveryNotification( msgid, SMSMessage::Status::ST_DELIVERED, gName );
                } else
                if ( resp.body.find( "<code>" ) != std::string::npos ) {
                    trck->registerDeliveryNotification( msgid, SMSMessage::Status::ST_REJECTED, gName );
                } else
                {
                    gmap[ gName ].suspend();
                    BOOST_THROW_EXCEPTION( HttpError() << throw_descr( resp.body.c_str() ) );
                }

            } else  
	    {
                HttpClient::Response resp = kannel->get( "http://"+trck->kserver+":"+trck->kport+req->genRequestURL( num, msg->getID().msg_num ) + url );

                if ( resp.body.find( "0: Accepted for delivery" ) == std::string::npos ) {
                    gmap[ gName ].suspend();
                    BOOST_THROW_EXCEPTION( HttpError() << throw_descr( resp.body.c_str() ) );
                }
            }

            boost::xtime now;
            boost::xtime_get( &now, boost::TIME_UTC );

            trck->op_queue.push( RequestTracker::SMSOperation::create<RequestTracker::OP_NewHistoryElement>( std::make_pair(
                    msg->getID(),
                    SMSMessage::HistoryElement( 0, 0, SMSMessage::Status::ST_PREPARING, gName, now.sec ) ),
                                                                       idp, ma_p, RequestTracker::OP_NewHistoryElementP), ma_p, RequestTracker::OP_NewHistoryElementP );
            trck->del_queue.push( RequestTracker::SMSOperation::create<RequestTracker::OP_CheckDelivery>( std::make_pair( req, msgid ), idp, ma_p, RequestTracker::OP_CheckDeliveryP ), ConfigManager::Instance()->getProperty<int>( "system.deliverytimeout" ) );

        } catch ( HttpError& err ) {
            out << "Retrying;\n" << boost::diagnostic_information( err );
            trck->op_queue.push( RequestTracker::SMSOperation::create<RequestTracker::OP_SendMessage>( std::make_pair( req, msgid ), idp, ma_p, RequestTracker::OP_SendMessageP ) );
            Logger::get_mutable_instance().smslogwarn( out.str() );
            return;
        } catch ( std::exception& err ) {
            out << "Retrying;\n" << err.what();
            trck->op_queue.push( RequestTracker::SMSOperation::create<RequestTracker::OP_SendMessage>( std::make_pair( req, msgid ), idp, ma_p, RequestTracker::OP_SendMessageP ) );
            Logger::get_mutable_instance().smslogwarn( out.str() );
            return;
        } catch ( ... ) {
            out << "Retrying;\n" << "unknown error";
            trck->op_queue.push( RequestTracker::SMSOperation::create<RequestTracker::OP_SendMessage>( std::make_pair( req, msgid ), idp, ma_p, RequestTracker::OP_SendMessageP ) );
            Logger::get_mutable_instance().smslogwarn( out.str() );
            return;
        }

        out << "parsed";
        Logger::get_mutable_instance().smsloginfo( out.str() );
    }

    void SMPPGateManager::pushToQueue( SMSRequest::PTR req, SMSMessage::ID msgid ) {
        std::list< string > gates = chooseGate( req, msgid );      

        std::map< string, std::pair< double, double > > gatesPoints;
        std::vector< string > gs;
        sms::MessageClassifier::CountryInfo msg = sms::MessageClassifier::get_mutable_instance().getMsgClass( req->to[ msgid.msg_num ] );

        for ( std::list< string >::iterator it = gates.begin(); it != gates.end(); it++ ) {
            string gname = *it;

            double price;
            SMPPGateManager::SMPPGatesMap gm = SMPPGateManager::Instance()->getGates();
            if ( !msg.operators.empty() )
                price = gm[ gname ].getTariff().costs( msg.mcc, msg.operators.begin()->second.mnc );
            else
                price = gm[ gname ].getTariff().costs( msg.mcc );

            double average_price = price;
            double average_time = 2;

            gatesPoints[ gname ] = std::make_pair( average_price, average_time );

        }

        for ( std::list< string >::iterator jt = gates.begin(); jt != gates.end(); jt++ ) {
            std::map< string, std::pair< double, double > >::iterator it, k;
            double vmin = gatesPoints.find( *jt )->second.first * gatesPoints.find( *jt )->second.second;
            k = gatesPoints.find( *jt ) == gatesPoints.end() ? gatesPoints.begin(): gatesPoints.find( *jt );
            for ( it = gatesPoints.begin(); it != gatesPoints.end(); it++ ) {
                double v = it->second.first * it->second.second;
                if ( v < vmin ) {
                    vmin = v;
                    k = it;
                }
            }
            gs.push_back( k->first );
            gatesPoints.erase( k );
        }

        int r_num;
        {
            boost::recursive_mutex::scoped_lock lck(lock);
            boost::uniform_int<> dist( 0, 100 );
            boost::variate_generator<boost::mt19937&, boost::uniform_int<> > res(gen, dist);

            r_num = res();
        }

        if ( ( r_num < 3 ) && ( gs.size() > 2 ) )
            std::swap( gs[1], gs[2] );

        if ( ( r_num < 13 ) && ( gs.size() > 1 ) )
            std::swap( gs[0], gs[1] );

        std::list< string > glist = std::list< string >( gs.begin(), gs.end() );

        if ( glist.empty() )
            throw (NoMoreGates());

//        send( req, msgid, *glist.begin() );

      mqueue.insert( req, msgid, glist );
    }


    bool SMPPGateManager::canAccept( string gName, SMSRequest::PTR req, SMSMessage::ID msgid ) {
        std::map< std::string, boost::any > args;
        SMSMessage::PTR msg = SMSMessageManager::get_mutable_instance().loadMessage( msgid );
        MessageClassifier::CountryInfo info = msg->getMsgClass();
        if ( gmap[ gName ].gateRule().empty() )
            return true;

        args[ "TO" ] = msg->getPhone();
        args[ "COUNTRY" ] = info.cName;
        args[ "COUNTRYCODE" ] = info.cCode;
        if ( !info.operators.empty() ) {
            args[ "OPERATORCODE" ] = info.operators.begin()->second.getCode();
        }
        args[ "FROM" ] = req->from;

        SMPPGateFilterParser parser;
        SMPPGateFilterParser::ResT res = parser.parseStr( gmap[ gName ].gateRule() );
        if ( res.ok )
            return res.filter.check( args );
        return false;
    }


    void SMPPGateManager::timeToSend() {
        std::ostringstream out;

        out << mqueue.size() << " messages in sending queue; ";

        std::list< msgqueue::MessageInfo > send_queue;

        SMPPGatesMap gateMap;
        {
            boost::recursive_mutex::scoped_lock lck(lock);
            gateMap = gmap;
        }
        StatManager::gNamePropMap p = StatManager::Instance()->get1MinuteStatsSMPPGate();
        for ( SMPPGatesMap::iterator itg = gateMap.begin(); itg != gateMap.end(); itg++ ) {
            bool busy = false;
            if ( ( p[ itg->first ].requests >= p[ itg->first ].acks + ( p[ itg->first ].responses - p[ itg->first ].deliveres ) + p[ itg->first ].acks / 10 + 1) // 10% bonus
                || (p[ itg->first ].acks >= 6*p[ itg->first ].responses + 5 ) ) {
                boost::recursive_mutex::scoped_lock lck(lock);
                gmap[ itg->first ].setBusy( true );
                busy = true;
            } else {
                boost::recursive_mutex::scoped_lock lck(lock);
                gmap[ itg->first ].setBusy( false );
            }

            if ( ( itg->second.enabled() ) &&
                 ( !itg->second.suspended() ) &&
                 ( !busy ) ) {
                std::list< msgqueue::MessageInfo > queue =
                mqueue.select(
                        msgqueue::FilterBY< msgqueue::IDX_GATEWAY, string >( itg->first ) <<
                        msgqueue::OrderBY< msgqueue::IDX_MESSAGE_PRIORITY >() <<
                        msgqueue::OrderBY< msgqueue::IDX_PARTNER_PRIORITY >() <<
                        msgqueue::LimitBY( 100 ) );

                send_queue.insert( send_queue.end(), queue.begin(), queue.end() );

                out << "[" << queue.size() << "=>" << itg->first << "] ";
            }
        }

        std::list< msgqueue::MessageInfo > queue =
        mqueue.select(
                msgqueue::FilterBYExpression( boost::bind( &SMPPGateManager::isExpiredACK, this, _1 ) )
                );
        out << queue.size() << " messages are ACK expired;";
        for ( std::list< msgqueue::MessageInfo >::iterator it = queue.begin(); it != queue.end(); it++ ) {
            SMSRequest::ID reqid = it->msgid.req;
            SMSRequest::PTR req;
            try {
                if ( RequestTracker::Instance()->req_cache.exists(reqid)) {
                    req = RequestTracker::Instance()->req_cache.get( reqid );
                } else {
                    req = RequestTracker::Instance()->loadRequestFromDb( reqid );
                    RequestTracker::Instance()->req_cache.push( req, req->getID() );
                }
            } catch (...) {
                continue;
            }

            if ( it->gateways.empty() )
                continue;

            string gateway_old = it->gateways.front(); it->gateways.pop_front();
            it->gateways.push_back( gateway_old );
            mqueue.insert( req, it->msgid, it->gateways );

        }

        queue =
        mqueue.select(
                msgqueue::FilterBYExpression( boost::bind( &SMPPGateManager::isExpired, this, _1 ) )
                );
        out << queue.size() << " messages are expired;";
        for ( std::list< msgqueue::MessageInfo >::iterator it = queue.begin(); it != queue.end(); it++ ) {
            SMSRequest::ID reqid = it->msgid.req;
            SMSRequest::PTR req;
            try {
                if ( RequestTracker::Instance()->req_cache.exists(reqid)) {
                    req = RequestTracker::Instance()->req_cache.get( reqid );
                } else {
                    req = RequestTracker::Instance()->loadRequestFromDb( reqid );
                    RequestTracker::Instance()->req_cache.push( req, req->getID() );
                }
            } catch (...) {
                continue;
            }
            RequestTracker* trck = RequestTracker::Instance();
            trck->parseMarkUndeliveredEvent( req, it->msgid );

        }

        for ( std::list< msgqueue::MessageInfo >::iterator it = send_queue.begin(); it != send_queue.end(); it++ ) {
            int ma_p = 0;
            string idp = it->req->pid;
            RequestTracker::Instance()->
                op_queue.push(
                        RequestTracker::SMSOperation::create<RequestTracker::OP_SubmitMessage>(
                                std::make_pair( it->req, std::make_pair( it->msgid, it->gateway ) ), idp, ma_p, RequestTracker::OP_SubmitMessageP ) );
        }

        Logger::get_mutable_instance().smsloginfo( out.str() );
    }

    bool SMPPGateManager::isExpiredACK( const msgqueue::MessageInfo& mi) {
        boost::xtime xt;
        boost::xtime_get(&xt, boost::TIME_UTC);

        return ( xt.sec - mi.added > 20 );
    }

    bool SMPPGateManager::isExpired( const msgqueue::MessageInfo& mi) {
        boost::xtime xt;
        boost::xtime_get(&xt, boost::TIME_UTC);

        return ( xt.sec - mi.added > 24*60*60 );
    }


    SMPPGateManager::SMPPGatesMap SMPPGateManager::getGates() {
        boost::recursive_mutex::scoped_lock lck(lock);
        return gmap;
    }

}
