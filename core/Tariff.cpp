#include "Tariff.h"
#include "Timer.h"

#include <iostream>
#include <sstream>

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
        ia >> BOOST_SERIALIZATION_NVP( tariff );

    } catch ( std::exception& err ) {
        throw std::runtime_error( string( "Cannot deserialize tariff[" ) + name + string( "]: " ) + err.what() );
    }
}

std::string Tariff::serialize() {
    std::ostringstream ofs;
    try {
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP( tariff );
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

    if ( hasOption( "price", cname ) == false ) {
        return INVALID_VALUE;
    }

    try {
        return boost::lexical_cast< double >( getOption( "price", cname ) );
    } catch ( ... ) {
        return INVALID_VALUE;
    }

}

double Tariff::costs( std::string cname, std::string opcode ) {

    if ( hasOption( "price", cname, opcode ) == false ) {
        return INVALID_VALUE;
    }

    try {
        return boost::lexical_cast< double >( getOption( "price", cname, opcode ) );
    } catch ( ... ) {
        return INVALID_VALUE;
    }

}

boost::logic::tribool Tariff::hasOption( std::string name ) {
    if ( tariff.options.find( name ) != tariff.options.end() )
        return true;

    return false;
}

boost::logic::tribool Tariff::hasOption( std::string name, std::string country ) {
    if ( tariff.countries.find( country ) != tariff.countries.end() )
        if ( tariff.countries[ country ].options.find( name ) != tariff.countries[ country ].options.end() )
            return true;

    boost::logic::tribool r = hasOption( name );
    if ( r == true )
        return boost::logic::indeterminate_keyword_t();

    return r;
}

boost::logic::tribool Tariff::hasOption( std::string name, std::string country, std::string oper ) {
    if ( tariff.countries.find( country ) != tariff.countries.end() )
        if ( tariff.countries[ country ].operators.find( oper ) != tariff.countries[ country ].operators.end() )
            if (
                    tariff.countries[ country ].operators[ oper ].options.find( name ) !=
                    tariff.countries[ country ].operators[ oper ].options.end()
                )
                return true;

    boost::logic::tribool r = hasOption( name, country );
    if ( r == true )
        return boost::logic::indeterminate_keyword_t();

    return r;
}

std::string Tariff::getOption( std::string name ) {
    if ( hasOption( name ) == true )
        return tariff.options[ name ];

    return "";
}

std::string Tariff::getOption( std::string name, std::string country ) {
    if ( hasOption( name, country ) == true )
        return tariff.countries[ country ].options[ name ];

    if ( hasOption( name ) == true )
        return tariff.options[ name ];

    return "";
}

std::string Tariff::getOption( std::string name, std::string country, std::string oper ) {
    if ( hasOption( name, country, oper ) == true )
        return tariff.countries[ country ].operators[oper].options[ name ];

    if ( hasOption( name, country ) == true )
        return tariff.countries[ country ].options[ name ];

    if ( hasOption( name ) == true )
        return tariff.options[ name ];

    return "";
}

void Tariff::setOption( std::string name, std::string value ) {
    tariff.options[ name ] = value;
}

void Tariff::setOption( std::string name, std::string country, std::string value ) {
    tariff.countries[country].options[ name ] = value;
}

void Tariff::setOption( std::string name, std::string country, std::string oper, std::string value ) {
    tariff.countries[country].operators[oper].options[ name ] = value;
}

void Tariff::removeOption( std::string name ) {
    if ( hasOption( name ) == true )
        tariff.options.erase( name );
}

void Tariff::removeOption( std::string name, std::string country ) {
    if ( hasOption( name, country ) == true )
        tariff.countries[ country ].options.erase( name );
}

void Tariff::removeOption( std::string name, std::string country, std::string oper ) {
    if ( hasOption( name, country, oper ) == true )
        tariff.countries[ country ].operators[ oper ].options.erase( name );
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
        TransactionPTR tr = db.openTransaction( conn, "TariffManager::saveTariff" );

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

void TariffManager::removeTariff( std::string name ) {
    std::ostringstream out;
    try {
        std::ostringstream r;

        PGSql::ConnectionHolder cHold( db );
        ConnectionPTR conn = cHold.get();
        TransactionPTR tr = db.openTransaction( conn, "TariffManager::saveTariff" );

        r       << "DELETE from tariffs where name='" << tr->esc( name ) << "';";

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
