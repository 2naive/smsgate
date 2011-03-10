#include "Tariff.h"
#include "Timer.h"

#include <iostream>
#include <sstream>

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>

Tariff::Tariff( ) {}

Tariff::Tariff( std::string name ) {
    tariff.name = name;
}

Tariff::Tariff( std::string name, std::string source ) {
    tariff.name = name;

    std::istringstream ifs( source );
    try {
        boost::archive::xml_iarchive ia( ifs );
        ia >> BOOST_SERIALIZATION_NVP( *this );

    } catch ( std::exception& err ) {
        throw std::runtime_error( string( "Cannot deserialize tariff[" ) + name + string( "]: " ) + err.what() );
    }
}

std::string Tariff::serialize() {
    std::ostringstream ofs;
    try {
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP( *this );
    } catch (...) {
        throw std::runtime_error( "Cannot serialize tariff" );;
    }
    return ofs.str();
}

void Tariff::addFilterCountry( std::string cname, double price ) {
    tariff.countries[ cname ].options[ "price" ] = boost::lexical_cast< std::string >( price );
}

void Tariff::addFilterCountryOperator( std::string cname, std::string opcode, double price ) {
    tariff.countries[ cname ].operators[ opcode ].options[ "price" ] = boost::lexical_cast< std::string >( price );
}

double Tariff::costs( std::string cname ) {
    if ( tariff.countries.find( cname ) == tariff.countries.end() )
        return INVALID_VALUE;

    if ( tariff.countries[ cname ].options.find( "price" ) == tariff.countries[ cname ].options.end() )
        return INVALID_VALUE;

    try {
        return boost::lexical_cast< double >( tariff.countries[ cname ].options[ "price" ] );
    } catch ( ... ) {
        return INVALID_VALUE;
    }

}

double Tariff::costs( std::string cname, std::string opcode ) {

    if ( tariff.countries.find( cname ) == tariff.countries.end() )
        return INVALID_VALUE;

    if ( tariff.countries[ cname ].operators.find( opcode ) == tariff.countries[ cname ].operators.end() )
        return costs( cname );

    if ( tariff.countries[ cname ].operators[ opcode ].options.find( "price" ) == tariff.countries[ cname ].operators[ opcode ].options.end() )
        return costs( cname );

    try {
        return boost::lexical_cast< double >( tariff.countries[ cname ].operators[ opcode ].options[ "price" ] );
    } catch ( ... ) {
        return costs( cname );
    }

}

TariffManager::TariffManager(): db( PGSqlConnPoolSystem::get_mutable_instance().getdb() ) {
    updateTimerID = Timer::Instance()->addPeriodicEvent( boost::bind( &TariffManager::updateTariffList, this ), 60 );
    updateTariffList();
}

TariffManager::~TariffManager() {
    Timer::Instance()->cancelEvent( updateTimerID );
}

Tariff TariffManager::loadTariff(std::string name) {
    return tmap[ name ];
}

void TariffManager::updateTariffList() {
    TariffListT _tlist;
    TariffMapT _tmap;
    std::ostringstream out;
    try {
        std::ostringstream r;

        PGSql::ConnectionHolder cHold( db );
        ConnectionPTR conn = cHold.get();
        TransactionPTR tr = db.openTransaction( conn, "TariffManager::updateTariffList" );

        r       << "SELECT name, description from tariffs;";

        Result res = tr->exec( r.str() );
        tr->commit();
        for ( Result::const_iterator dbr = res.begin(); dbr != res.end(); dbr++ ) {
            _tlist.push_back( (*dbr)[0].as<std::string>() );
            _tmap.insert( std::make_pair( (*dbr)[0].as<std::string>(), Tariff( (*dbr)[0].as<std::string>(), (*dbr)[1].as<std::string>() ) ) );
        }

        tlist = _tlist;
        tmap = _tmap;

    } catch ( PGSqlError& err ) {
        out << "Error while loading tariff: " << err.what();
        Logger::get_mutable_instance().smslogerr( out.str() );
    } catch ( PGBrokenConnection& err ) {
        out << "Connection Error while loading tariff: " << err.what();
        Logger::get_mutable_instance().smslogerr( out.str() );
    }
}

void TariffManager::saveTariff( std::string name, Tariff t ) {
    std::ostringstream out;
    try {
        std::ostringstream r;

        PGSql::ConnectionHolder cHold( db );
        ConnectionPTR conn = cHold.get();
        TransactionPTR tr = db.openTransaction( conn, "TariffManager::updateTariffList" );

        if ( tmap.find( name ) == tmap.end() ) {
            r       << "INSERT into tariffs values ("
                    << "'" << tr->esc( name ) << "', "
                    << "'" << tr->esc( t.serialize() ) << "');";
        } else {
            r       << "UPDATE tariffs "
                    << "set description='" << tr->esc( t.serialize() ) << "' "
                    << "WHERE name='" << tr->esc( name ) << "';";
        }

        Result res = tr->exec( r.str() );
        tr->commit();

    } catch ( PGSqlError& err ) {
        out << "Error while saving tariff: " << err.what();
        Logger::get_mutable_instance().smslogerr( out.str() );
    } catch ( PGBrokenConnection& err ) {
        out << "Connection Error while saving tariff: " << err.what();
        Logger::get_mutable_instance().smslogerr( out.str() );
    }

    updateTariffList();
}

TariffManager::TariffListT TariffManager::tariffs_list() {
    return tlist;
}
