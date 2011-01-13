#include "PartnerManager.h"
#include "PGSql.h"
#include "Logger.h"
#include "Timer.h"

using namespace std;

PartnerManager::PartnerManager() {

    loadFromDb();

    tid = sms::Timer::Instance()->addPeriodicEvent( boost::bind( &PartnerManager::loadFromDb, this ), 20 );
}

PartnerInfo PartnerManager::findByName( string pName ) throw ( PartnerNotFoundError ){
    boost::recursive_mutex::scoped_lock lck( pmlock );
    if ( pbox.get<1>().find( pName ) != pbox.get<1>().end() ) {
        return *pbox.get<1>().find( pName );
    }
    BOOST_THROW_EXCEPTION(  PartnerNotFoundError()
                            << throw_pName( pName.c_str() )
                            << sms::throw_descr( "PartnerManager::findByName [ Partner Not Found ]" ) );

}

PartnerInfo PartnerManager::findById( string id ) throw ( PartnerNotFoundError ){
    boost::recursive_mutex::scoped_lock lck( pmlock );
    if ( pbox.get<2>().find( id ) != pbox.get<2>().end() ) {
        return *pbox.get<2>().find( id );
    }
    BOOST_THROW_EXCEPTION(  PartnerNotFoundError()
                            << throw_pId( id.c_str() )
                            << sms::throw_descr( "PartnerManager::findById [ Partner Not Found ]" ) );

}

void PartnerManager::loadFromDb() {
    PGSql& db = PGSqlConnPoolSystem::get_mutable_instance().getdb();

    std::ostringstream out;
    out << "Loading partners data ";
    pBox pb;
    try {

        PGSql::ConnectionHolder cHold( db );
        ConnectionPTR conn = cHold.get();
        TransactionPTR tr = db.openTransaction( conn, "PartnerManager::loadFromDb" );
        std::ostringstream dbreq1;
        dbreq1  << " SELECT pid, uname, pass, cname, manager, balance, credit, plimit, postplay, trial, priority, phone, text, tariff FROM partners;";

        Result res = tr->exec( dbreq1.str() );
        tr->commit();

        for ( Result::const_iterator dbr = res.begin(); dbr != res.end(); dbr++ ) {
            //TODO onlone reloading info
            pb.get<1>().insert( PartnerInfo(
                                            (*dbr)[1].as<string>(),
                                            (*dbr)[2].as<string>(),
                                            (*dbr)[0].as<string>(),
                                            (*dbr)[3].as<string>(),
                                            (*dbr)[11].as<string>(),
                                            (*dbr)[12].as<string>(),
                                            (*dbr)[13].as<string>(),
                                            (*dbr)[4].as<string>(),
                                            (*dbr)[5].as<double>(),
                                            (*dbr)[6].as<double>(),
                                            (*dbr)[7].as<int>(),
                                            (*dbr)[8].as<bool>(),
                                            (*dbr)[9].as<bool>(),
                                            (*dbr)[10].as<int>()
                                   ) );
        }

    } catch ( PGSqlError & err ) {
        out << "error; " << err.what();
        Logger::get_mutable_instance().smslogerr( out.str() );
    } catch ( PGBrokenConnection& err ) {
        out << "error; " << err.what();
        Logger::get_mutable_instance().smslogwarn( out.str() );
    }

    boost::recursive_mutex::scoped_lock lck( pmlock );
    pbox = pb;

    out << "parsed;";
    Logger::get_mutable_instance().smsloginfo( out.str() );
}
