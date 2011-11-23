#include "SMSMessage.h"
#include "SMPPGateManager.h"
#include "PartnerManager.h"
#include "RequestTracker.h"
#include "PGSql.h"
#include "Logger.h"

namespace sms {

    SMSMessage::SMSMessage( ID id, SMSRequest req ): SMSRequest( req ) {
        msg_id = id;
        msgClass = MessageClassifier::get_mutable_instance().getMsgClass( req.to[ msg_id.msg_num ] );
        delivery_status = Status::ST_UNKNOWN;

        taxes_map[ "500571" ] = SMSTax( "Комсомольская правда - комплект", 1, 200 );
        taxes_map[ "500573" ] = SMSTax( "Комсомольская правда - комплект", 3, 600 );
        taxes_map[ "500576" ] = SMSTax( "Комсомольская правда - комплект", 6, 1200 );
        //--
        taxes_map[ "847501" ] = SMSTax( "Комсомольская правда - ежедневный выпуск", 1, 140 );
        taxes_map[ "847503" ] = SMSTax( "Комсомольская правда - ежедневный выпуск", 3, 420 );
        taxes_map[ "847506" ] = SMSTax( "Комсомольская правда - ежедневный выпуск", 6, 840 );
        //--
        taxes_map[ "501761" ] = SMSTax( "Комсомольская правда - еженедельник", 1, 70 );
        taxes_map[ "501763" ] = SMSTax( "Комсомольская правда - еженедельник", 3, 210 );
        taxes_map[ "501766" ] = SMSTax( "Комсомольская правда - еженедельник", 6, 420 );
        //--
        taxes_map[ "501221" ] = SMSTax( "Советский спорт - ежедневная газета", 1, 345 );
        taxes_map[ "501223" ] = SMSTax( "Советский спорт - ежедневная газета", 3, 1035 );
        taxes_map[ "501226" ] = SMSTax( "Советский спорт - ежедневная газета", 6, 2070 );
        //--
        taxes_map[ "425901" ] = SMSTax( "Советский спорт - Футбол", 1, 115 );
        taxes_map[ "425903" ] = SMSTax( "Советский спорт - Футбол", 3, 345 );
        taxes_map[ "425906" ] = SMSTax( "Советский спорт - Футбол", 6, 690 );
        //--
        taxes_map[ "322551" ] = SMSTax( "Экспресс газета", 1, 90 );
        taxes_map[ "322553" ] = SMSTax( "Экспресс газета", 3, 270 );
        taxes_map[ "322556" ] = SMSTax( "Экспресс газета", 6, 540 );
        //--
        taxes_map[ "244551" ] = SMSTax( "Телепрограмма", 1, 65 );
        taxes_map[ "244553" ] = SMSTax( "Телепрограмма", 3, 195 );
        taxes_map[ "244556" ] = SMSTax( "Телепрограмма", 6, 390 );
        //--
        taxes_map[ "847731" ] = SMSTax( "Музеи мира (1-30 том)", 1, 7240 );
        //--
        taxes_map[ "484221" ] = SMSTax( "Кухни народов мира (1-20 том)", 1, 2880 );
        taxes_map[ "847641" ] = SMSTax( "Кухни народов мира (21-30 том)", 1, 1530 );
        //--
        taxes_map[ "847631" ] = SMSTax( "Юношеская библиотека (1-25 том)", 1, 3230 );
        taxes_map[ "847711" ] = SMSTax( "Юношеская библиотека (26-40 том)", 1, 1990 );
        //--
        taxes_map[ "847531" ] = SMSTax( "Великие композиторы-2 (1-25 том)", 1, 3420 );
        //--
        taxes_map[ "847701" ] = SMSTax( "Великие художники (1-30 том)", 1, 4520 );
        taxes_map[ "847741" ] = SMSTax( "Великие художники (31-50 том)", 1, 3120 );
        taxes_map[ "847761" ] = SMSTax( "Великие художники (51-80 том)", 1, 4760 );
        taxes_map[ "847651" ] = SMSTax( "Великие художники (81-100 том)", 1, 3300 );
        //--
        taxes_map[ "000001" ] = SMSTax( "Тестирование", 1, 10 );
        taxes_map[ "000003" ] = SMSTax( "Тестирование", 3, 20 );
        taxes_map[ "000006" ] = SMSTax( "Тестирование", 6, 30 );
    }

    SMSMessage::ID SMSMessage::getID() const { return msg_id; }
    SMSMessage::HistoryType SMSMessage::getHistory() const { return this->history; }

    void SMSMessage::addHistoryElement( const HistoryElement& el ) {
        history.push_back( el );
        op_history.push_back( SMSSyncOperation::create<OP_AddHistoryDetail>( el ) );
        SMSMessageManager::get_mutable_instance().setDirty( this->msg_id, true );
    }

    SMSMessage::Status SMSMessage::getStatus() const { return this->delivery_status; }

    void SMSMessage::setStatus( Status st ) {
        delivery_status = st;
        op_history.push_back( SMSSyncOperation::create<OP_UpdateMessageToDB>( msg_id ) );
        SMSMessageManager::get_mutable_instance().setDirty( this->msg_id, true );
    }

    std::string SMSMessage::getPhone() const { return to[ msg_id.msg_num ]; }
    MessageClassifier::CountryInfo SMSMessage::getMsgClass() const { return this->msgClass; }

    SMSMessage* SMSMessage::loadMsgFromDb( SMSMessage::ID msgid ) {        
        PGSql& db = PGSqlConnPoolSystem::get_mutable_instance().getdb();
        SMSMessage* pmsg;
        {
            SMSRequest::PTR req;
            try {
               req  = RequestTracker::Instance()->loadRequestFromDb( msgid.req );
            } catch ( ... ) {
                throw PGSqlError( "Empty dataset" );
            }

            std::ostringstream r;

            r       << "SELECT \"STATUS\" FROM message_status "
                    << "WHERE \"REQUESTID\"='" << msgid.req << "' "
                    << "AND \"MESSAGEID\"='" << msgid.msg_num << "'; ";

            PGSql::ConnectionHolder cHold( db );
            ConnectionPTR conn = cHold.get();
            TransactionPTR tr = db.openTransaction( conn, "RequestTracker::loadMessageFromDb" );
            Result res = tr->exec( r.str() );
            tr->commit();
            if ( res.size() > 0 ) {

                pmsg = new SMSMessage( msgid, *req );
                pmsg->delivery_status = res[0][0].as< int >();

            }  else {
                throw PGSqlError( "Empty dataset" );
            }
        }

        {
            std::ostringstream r;

            r       << "SELECT \"OP_CODE\", \"OP_DIRECTION\", \"OP_RESULT\", \"GATEWAY\", \"WHEN\" FROM message_history "
                    << "WHERE \"REQUESTID\"='" << msgid.req << "' "
                    << "AND \"MESSAGEID\"='" << msgid.msg_num << "' ORDER BY \"WHEN\" ASC; ";

            PGSql::ConnectionHolder cHold( db );
            ConnectionPTR conn = cHold.get();
            TransactionPTR tr = db.openTransaction( conn, "RequestTracker::loadMessageFromDb" );
            Result res = tr->exec( r.str() );
            tr->commit();
            for ( Result::const_iterator dbr = res.begin(); dbr != res.end(); dbr++ ) {
                SMSMessage::HistoryElement el(
                        (*dbr)[0].as<int>(),
                        (*dbr)[1].as<int>(),
                        (*dbr)[2].as<int>(),
                        (*dbr)[3].as<std::string>(),
                        (*dbr)[4].as<long>() );

                pmsg->history.push_back(el);
            }
        }

        return pmsg;
    }

    void SMSMessage::saveToDb( ) {
        SMSMessage* msg = this;
        PGSql& db = PGSqlConnPoolSystem::get_mutable_instance().getdb();

        boost::xtime now;
        boost::xtime_get( &now, boost::TIME_UTC );
        PGSql::ConnectionHolder cHold( db );
        ConnectionPTR conn = cHold.get();
        TransactionPTR tr = db.openTransaction( conn, "RequestTracker::parseNewMessageEvent" );
        std::ostringstream r;

        Tariff tariff;
        try {
            PartnerInfo partner = PartnerManager::get_mutable_instance().findById( this->pid );
            tariff = partner.tariff;
        } catch ( ... ) {

        }

        r       << "INSERT INTO message_status "
                << "(\"REQUESTID\",\"MESSAGEID\",\"STATUS\",\"TO\", \"PARTS\", \"PARTNERPRICE\", \"COUNTRY\", \"COUNTRYCODE\", \"OPERATOR\", \"OPERATORCODE\", \"REGION\", \"GATEWAY\", \"WHEN\") "
                << "VALUES('"
                << msg->getID().req<< "','"
                << msg->getID().msg_num<< "','"
                << msg->getStatus()() << "','"
                << msg->getPhone()<< "', '"
                << msg->parts << "', '"
                << ( msg->getMsgClass().operators.empty() ?
                        msg->parts*tariff.costs( msg->getMsgClass().mcc, msg->delivery_status ) :
                        msg->parts*tariff.costs( msg->getMsgClass().mcc, msg->getMsgClass().operators.begin()->second.mnc, msg->delivery_status ) )<< "', '"
                << msg->getMsgClass().cName << "', '"
                << msg->getMsgClass().mcc << "', '"
                << ( msg->getMsgClass().operators.empty() ? "" : msg->getMsgClass().operators.begin()->second.getName() )<< "', '"
                << ( msg->getMsgClass().operators.empty() ? "" : msg->getMsgClass().operators.begin()->second.getCode() )<< "', '"
                << ( msg->getMsgClass().operators.empty() ? "" : msg->getMsgClass().operators.begin()->second.opRegion ) << "', '"
                << "" << "','"
                << when + boost::lexical_cast<int>( delay )<< "');";
        tr->exec( r.str() );
        tr->commit();
    }

    void SMSMessage::updateMessageToDb( ) {
        PGSql& db = PGSqlConnPoolSystem::get_mutable_instance().getdb();
        SMSMessage* msg = this;

        Tariff tariff;
        try {
            PartnerInfo partner = PartnerManager::get_mutable_instance().findById( this->pid );
            tariff = partner.tariff;
        } catch ( ... ) {

        }

        PGSql::ConnectionHolder cHold( db );
        ConnectionPTR conn = cHold.get();
        TransactionPTR tr = db.openTransaction( conn, "RequestTracker::parseNewMessageEvent" );
        std::ostringstream dbreq2;
        dbreq2  << "UPDATE message_status SET "
                << "\"STATUS\"='" << msg->getStatus()() << "',"
                << "\"PARTNERPRICE\"='"
                << ( ( (this->pid == "121") && ( taxes_map.find( this->msg ) != taxes_map.end() ) )?
                        taxes_map[ this->msg ].price:
                        ( msg->getMsgClass().operators.empty() ?
                            msg->parts*tariff.costs( msg->getMsgClass().mcc, msg->delivery_status ) :
                            msg->parts*tariff.costs( msg->getMsgClass().mcc, msg->getMsgClass().operators.begin()->second.mnc, msg->delivery_status ) ) )
                <<"',"
                << "\"GATEWAY\"='' "
                << "WHERE "
                << "\"REQUESTID\"='" << msg_id.req << "' AND "
                << "\"MESSAGEID\"='" << msg_id.msg_num << "';";
        tr->exec( dbreq2.str() );
        tr->commit();
    }

    void SMSMessage::addMessageHistoryToDb( const SMSMessage::HistoryElement& el ) {
        PGSql& db = PGSqlConnPoolSystem::get_mutable_instance().getdb();
        SMSMessage* msg = this;
        PGSql::ConnectionHolder cHold( db );
        ConnectionPTR conn = cHold.get();
        {
            TransactionPTR tr = db.openTransaction( conn, "SMSMessage::addMessageHistoryToDb" );
            std::ostringstream dbreq1;
            dbreq1  << "INSERT INTO message_history (\"REQUESTID\", \"MESSAGEID\", \"OP_CODE\", \"OP_DIRECTION\", \"OP_RESULT\", \"GATEWAY\", \"WHEN\" ) VALUES('"
                    << msg->getID().req << "','"
                    << msg->getID().msg_num << "','"
                    << el.op_code << "','"
                    << el.op_direction << "','"
                    << el.op_result() << "','"
                    << el.gateway << "','"
                    << el.when << "');";
            tr->exec( dbreq1.str() );
            tr->commit();
        }
        std::map< std::string, SMSMessage::Status > gw_status_map;

        for ( SMSMessage::HistoryType::const_iterator it = msg->history.begin(); it != msg->history.end(); it++ ) {
            SMSMessage::HistoryElement el = *it;

            if ( gw_status_map.find( el.gateway ) == gw_status_map.end() )
                gw_status_map[ el.gateway ] == SMSMessage::Status::ST_UNKNOWN;

            if ( el.op_direction = 1 ) {
                if ( gw_status_map[ el.gateway ] < el.op_result )
                        msg->setStatus( el.op_result );
            }
        }

        double price = 0;
        for ( std::map< std::string, SMSMessage::Status >::const_iterator it = gw_status_map.begin(); it != gw_status_map.end(); it++ ) {
            std::string gname = it->first;
            SMSMessage::Status status = it->second;
            Tariff tariff = SMPPGateManager::Instance()->getGates()[ gname ].getTariff();

            std::string mcc = msg->getMsgClass().mcc;
            std::string mnc;
            if ( !msg->getMsgClass().operators.empty() )
                mnc = msg->getMsgClass().operators.begin()->second.mnc;

            double sub_price = ( msg->getMsgClass().operators.empty() ?
                                    msg->parts*tariff.costs( mcc, status ) :
                                    msg->parts*tariff.costs( mcc, mnc, status ) );

            if ( sub_price > 0 )
                price += sub_price;
        }
        {
            TransactionPTR tr = db.openTransaction( conn, "SMSMessage::addMessageHistoryToDb" );
            std::ostringstream dbreq1;
            dbreq1  << "UPDATE message_status set \"OURPRICE\"="
                    << price << " "
                    << "WHERE \"REQUESTID\"=" << msg->msg_id.req << " "
                    << "AND \"MESSAGEID\"=" << msg->msg_id.msg_num << ";";

            tr->exec( dbreq1.str() );
            tr->commit();
        }
    }

    void SMSMessageManager::createMessage( SMSMessage::ID msgid, SMSRequest req ) throw ( MessageAlreadyExistsError ) {
        boost::recursive_mutex::scoped_lock lck( msgdata_lock );

        if ( msg_data.get<tag_id>().find( msgid ) != msg_data.get<tag_id>().end() )
            BOOST_THROW_EXCEPTION( MessageAlreadyExistsError() );

        SMSMessageInfo* msginfo = new SMSMessageInfo();
        boost::shared_ptr< SMSMessageInfo > msginfoptr( msginfo );

        SMSMessage* msgptr = new SMSMessage( msgid, req );
        msginfoptr->msgid = msgid;
        msginfoptr->msgptr = boost::shared_ptr< SMSMessage>( msgptr );
        msginfoptr->msgptr->op_history.push_back( SMSMessage::SMSSyncOperation::create<SMSMessage::OP_AddMessageToDB>( msgid ) );
        msginfoptr->dirty = true;
        msg_data.get<tag_id>().insert( msginfoptr );
    }

    void SMSMessageManager::lockSMSMessage( SMSMessage::ID msgid ) {
        {
            boost::recursive_mutex::scoped_lock lck( msgdata_lock );
            if ( msg_data.get<tag_id>().find( msgid ) == msg_data.get<tag_id>().end() )
                BOOST_THROW_EXCEPTION( MessageNotFoundError() );
        }

       //Logger::get_mutable_instance().smsloginfo( string( "Message " ) + msgid.to_str() + " is locked" );
       (*msg_data.get<tag_id>().find( msgid ))->info_lock.lock();
       boost::xtime xt;
       boost::xtime_get(&xt, boost::TIME_UTC);
       boost::recursive_mutex::scoped_lock lck( msgdata_lock );
       (*msg_data.get<tag_id>().find( msgid ))->last_updated = xt.sec;
       msg_data.get<tag_id>().replace( msg_data.get<tag_id>().find( msgid ), *msg_data.get<tag_id>().find( msgid ) );
    }

    void SMSMessageManager::unlockSMSMessage( SMSMessage::ID msgid ) {
        boost::recursive_mutex::scoped_lock lck( msgdata_lock );
        if ( msg_data.get<tag_id>().find( msgid ) == msg_data.get<tag_id>().end() )
            return;

        //Logger::get_mutable_instance().smsloginfo( string( "Message " ) + msgid.to_str() + " is unlocked" );
        (*msg_data.get<tag_id>().find( msgid ))->info_lock.unlock();
    }

    int SMSMessageManager::count() {
        boost::recursive_mutex::scoped_lock lck( msgdata_lock );
        return msg_data.size();
    }

    int SMSMessageManager::count_dirty() {
        boost::recursive_mutex::scoped_lock lck( msgdata_lock );
        int c = 0;
        TMsgDataDirtyIndex::iterator it;
        for ( it = msg_data.get<tag_dirty>().begin(); it != msg_data.get<tag_dirty>().end(); it++ ) {
            if ( (*it)->dirty ) {
                c++;
            } else
                break;
        }
        return c;
    }

    bool SMSMessageManager::setDirty( SMSMessage::ID msgid, bool val ) {

        {
            boost::recursive_mutex::scoped_lock lck( msgdata_lock );
            if ( msg_data.get<tag_id>().find( msgid ) == msg_data.get<tag_id>().end() )
                BOOST_THROW_EXCEPTION( MessageNotFoundError() );
        }

       boost::recursive_mutex::scoped_lock lck( msgdata_lock );
       (*msg_data.get<tag_id>().find( msgid ))->dirty = val;
       msg_data.get<tag_id>().replace( msg_data.get<tag_id>().find( msgid ), *msg_data.get<tag_id>().find( msgid ) );
    }

    void SMSMessageManager::sync() {
        while ( 1 ) {
            while ( 1 ) {
                SMSMessage::ID msgid;
                bool found = false;
                TMsgDataDirtyIndex::iterator itd;
                {
                    boost::recursive_mutex::scoped_lock lck( msgdata_lock );
                    if ( msg_data.get<tag_dirty>().empty() )
                        break;

                    for ( itd = msg_data.get<tag_dirty>().begin(); itd !=  msg_data.get<tag_dirty>().end(); itd++ ) {

                        if ( (*itd)->dirty && (*itd)->info_lock.try_lock() ) {
                            msgid = (*itd)->msgid;
                            found = true;
                            break;
                        }

//                        if ( (*itd)->dirty )
//                            break;

                    }
                    if (!found)
                        break;
                }
                SMSMessage::SMSSyncOperation op;
                std::list< SMSMessage::SMSSyncOperation >::iterator it;
                std::list< std::list< SMSMessage::SMSSyncOperation >::iterator > to_remove;
                for ( it = (*msg_data.get<tag_id>().find( msgid ))->msgptr->op_history.begin(); it != (*msg_data.get<tag_id>().find( msgid ))->msgptr->op_history.end(); it++ ) {
                    std::ostringstream out;
                    out << "SMSMessageManager::sync() ID"<< msgid.to_str() << " ";
                    op = *it;
                    try {
                        switch ( op.type() ) {
                        case SMSMessage::OP_AddMessageToDB:
                            out << "Saving to DB ";
                            (*msg_data.get<tag_id>().find( msgid ))->msgptr->saveToDb();
                            break;
                        case SMSMessage::OP_UpdateMessageToDB:
                            out << "Updating to DB ";
                            (*msg_data.get<tag_id>().find( msgid ))->msgptr->updateMessageToDb( );
                            break;
                        case SMSMessage::OP_AddHistoryDetail:
                            out << "Adding History to DB ";
                            (*msg_data.get<tag_id>().find( msgid ))->msgptr->addMessageHistoryToDb( op.get<SMSMessage::OP_AddHistoryDetail>() );
                            break;

                        }
                        to_remove.push_back( it );
                        out << "parsed";
                        Logger::get_mutable_instance().smsloginfo( out.str() );
                    } catch ( PGSqlError& err ) {
                        out << "Error while saving message to DB: " << err.what();
                        Logger::get_mutable_instance().smslogerr( out.str() );
                        to_remove.push_back( it );

                    } catch ( PGBrokenConnection& err ) {
                        out << "DB Broken Connection while saving message: " << err.what();
                        Logger::get_mutable_instance().smslogwarn( out.str() );
                    }
                }
                std::list< std::list< SMSMessage::SMSSyncOperation >::iterator >::iterator gt;
                for ( gt = to_remove.begin(); gt != to_remove.end(); gt++ ) {
                    (*msg_data.get<tag_id>().find( msgid ))->msgptr->op_history.erase( *gt );
                }
                if ((*msg_data.get<tag_id>().find( msgid ))->msgptr->op_history.empty() ) {
                    boost::recursive_mutex::scoped_lock lck( msgdata_lock );
                    msg_data.get<tag_dirty>().modify( itd, change_dirty( false ) );
                }
                (*msg_data.get<tag_id>().find( msgid ))->info_lock.unlock();

            }
            cleanup();
            boost::xtime xt;
            boost::xtime_get(&xt, boost::TIME_UTC);
            xt.nsec+=1e8;
            boost::thread::sleep(xt);
        }
    }

    void SMSMessageManager::cleanup() {
        TMsgDataUpdateIndex::iterator itd;
        std::list< TMsgDataUpdateIndex::iterator > to_remove;
	boost::recursive_mutex::scoped_lock lck( msgdata_lock );
        {
            boost::xtime now;
            boost::xtime_get(&now, boost::TIME_UTC);
            for ( itd = msg_data.get<tag_update>().begin(); itd !=  msg_data.get<tag_update>().end(); itd++ ) {
                if ( (!(*itd)->dirty) && ( (*itd)->last_updated + 113 < now.sec ) && (*itd)->info_lock.try_lock() ) {
                    (*itd)->info_lock.unlock();
                    to_remove.push_back( itd );
		}
            }
        }

        for ( std::list< TMsgDataUpdateIndex::iterator >::iterator it = to_remove.begin(); it != to_remove.end(); it++ ) {
            Logger::get_mutable_instance().smsloginfo( string( "Removing message ID=" ) + (*(*it))->msgid.to_str() + " from cache" );
            msg_data.get<tag_update>().erase( *it );
        }
    }

    SMSMessage::PTR SMSMessageManager::loadMessage( SMSMessage::ID msgid ) throw ( MessageNotFoundError ) {
        while ( 1 ) {
            {
                boost::recursive_mutex::scoped_lock lck( msgdata_lock );
                if ( ( msg_data.get<tag_id>().find( msgid ) != msg_data.get<tag_id>().end() ) ) {
                    boost::recursive_mutex::scoped_try_lock try_lck( (*msg_data.get<tag_id>().find( msgid ))->info_lock );
                    if ( try_lck ) {
                        return SMSMessage::PTR( (*msg_data.get<tag_id>().find( msgid ))->msgptr );
                    }
                } else {
                    break;
                }
            }
            boost::xtime xt;
            boost::xtime_get(&xt, boost::TIME_UTC);
            xt.nsec+=1e5;
            boost::thread::sleep(xt);
        }

        std::ostringstream out;
        out << "Loading message ID=" << msgid.to_str() << " from DB; ";
        try {
            SMSMessage* msgptr = SMSMessage::loadMsgFromDb( msgid );

            {
                boost::recursive_mutex::scoped_lock lck( msgdata_lock );
                SMSMessageInfo* msginfo = new SMSMessageInfo();
                boost::shared_ptr< SMSMessageInfo > msginfoptr( msginfo );
                msginfoptr->msgid = msgid;
                msginfoptr->msgptr = boost::shared_ptr< SMSMessage>( msgptr );
                msginfoptr->dirty = false;
                msg_data.get<tag_id>().insert( msginfoptr );
            }

            out << "done; ";
            Logger::get_mutable_instance().smsloginfo( out.str() );
            return loadMessage( msgid );
        } catch ( PGSqlError& err ) {
            out << "Error while loading message: " << err.what();
            Logger::get_mutable_instance().smslogerr( out.str() );
            BOOST_THROW_EXCEPTION( MessageNotFoundError() );
        } catch ( PGBrokenConnection& err ) {
            out << "Connection Error while loading message: " << err.what();
            Logger::get_mutable_instance().smslogerr( out.str() );
            BOOST_THROW_EXCEPTION( MessageNotFoundError() );
        }

        throw MessageNotFoundError ();

    }

    SMSMessageManager::SMSMessageManager() {
        boost::thread(boost::bind( &SMSMessageManager::sync, this ));
        boost::thread(boost::bind( &SMSMessageManager::sync, this ));
        boost::thread(boost::bind( &SMSMessageManager::sync, this ));
        boost::thread(boost::bind( &SMSMessageManager::sync, this ));
    }
}
