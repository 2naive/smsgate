#include "Tariff.h"

#include <iostream>
#include <fstream>

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>

Tariff::Tariff( const ID _name, int tt ): TariffType( tt ), name( _name ) {
}

Tariff::Tariff() {
}

Tariff Tariff::buildEmpty( ID name ) {
    Tariff t;
    t.name = name;
    t.TariffType = Tariff::ROOT;

    return t;
}

Tariff Tariff::buildInherit( Tariff::ID name, const std::string& filename ) {
    Tariff t;
    t.name = name;
    t.TariffType = Tariff::MULTIMPLEXION;

    Tariff base = Tariff::buildFromFile( filename );

    t.bases.push_back( std::make_pair( base.name, filename ) );
    t.tlist.insert( std::make_pair( base.name, base ) );
    t.arguments.push_back( base.name );
    t.arguments.push_back( filename );
    t.arguments.push_back( "1.0" );

    return t;
}

Tariff Tariff::buildInheritMultiplex( Tariff::ID name, const std::string& filename, float mult ) {
    Tariff t;
    t.name = name;
    t.TariffType = Tariff::MULTIMPLEXION;

    Tariff base = Tariff::buildFromFile( filename );

    t.bases.push_back( std::make_pair( base.name, filename ) );
    t.tlist.insert( std::make_pair( base.name, base ) );
    t.arguments.push_back( base.name );
    t.arguments.push_back( filename );
    t.arguments.push_back( boost::lexical_cast< std::string > ( mult ) );

    return t;
}

Tariff Tariff::buildInheritAdd( Tariff::ID name, const std::string& filename, float mult ) {
    Tariff t;
    t.name = name;
    t.TariffType = Tariff::ADDICTION;

    Tariff base = Tariff::buildFromFile( filename );

    t.bases.push_back( std::make_pair( base.name, filename ) );
    t.tlist.insert( std::make_pair( base.name, base ) );
    t.arguments.push_back( base.name );
    t.arguments.push_back( filename );
    t.arguments.push_back( boost::lexical_cast< std::string > ( mult ) );

    return t;
}

Tariff Tariff::buildInheritMin( ID name, const std::string& filename1, const std::string& filename2 ) {
    Tariff t;
    t.name = name;
    t.TariffType = Tariff::MINIMAL;

    Tariff base1 = Tariff::buildFromFile( filename1 );
    Tariff base2 = Tariff::buildFromFile( filename2 );

    t.bases.push_back( std::make_pair( base1.name, filename1 ) );
    t.bases.push_back( std::make_pair( base2.name, filename2 ) );

    t.tlist.insert( std::make_pair( base1.name, base1 ) );
    t.tlist.insert( std::make_pair( base2.name, base2 ) );

    t.arguments.push_back( base1.name );
    t.arguments.push_back( filename1 );
    t.arguments.push_back( base2.name );
    t.arguments.push_back( filename2 );

    return t;
}

Tariff Tariff::buildInheritMax( ID name, const std::string& filename1, const std::string& filename2 ) {
    Tariff t;
    t.name = name;
    t.TariffType = Tariff::MAXIMAL;

    Tariff base1 = Tariff::buildFromFile( filename1 );
    Tariff base2 = Tariff::buildFromFile( filename2 );

    t.bases.push_back( std::make_pair( base1.name, filename1 ) );
    t.bases.push_back( std::make_pair( base2.name, filename2 ) );

    t.tlist.insert( std::make_pair( base1.name, base1 ) );
    t.tlist.insert( std::make_pair( base2.name, base2 ) );

    t.arguments.push_back( base1.name );
    t.arguments.push_back( filename1 );
    t.arguments.push_back( base2.name );
    t.arguments.push_back( filename2 );

    return t;
}

void Tariff::addFilterCountry( std::string cname, float price ) {
    CFilterList.push_back( std::make_pair( cname, price ) );
}

void Tariff::addFilterCountryOperator( std::string cname, std::string opcode, float price ) {
    COFilterList.push_back( std::make_pair( std::make_pair( cname, opcode ), price ) );
}

Tariff Tariff::buildFromFile( const std::string& filename ) {
    std::ifstream ifs( filename.c_str() );
    if ( !ifs.good() )
        throw std::runtime_error( string( "Cannot open tariff file: " ) + filename );;
    try {
        Tariff t;
        boost::archive::xml_iarchive ia( ifs );
        ia >> BOOST_SERIALIZATION_NVP( t );
        t.rebuildBases();

        return t;
    } catch ( std::exception& err ) {
        throw std::runtime_error( string( "Cannot deserialize tariff[" ) + filename + string( "]: " ) + err.what() );
    }
}

void Tariff::saveToFile( const std::string& filename ) {
    std::ofstream ofs( filename.c_str() );
    try {
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP( this );
    } catch (...) {
        throw std::runtime_error( "Cannot serialize tariff" );;
    }
}


float Tariff::costs( sms::OpInfo& op ) {
    return costs( op.country, op.opcode );
}

float Tariff::costs( std::string cname, std::string opcode ) {
    float res;
    if ( searchForCountryOperatorPrice( cname, opcode, res ) )
        return res;

    if ( searchForCountryPrice( cname, res ) )
        return res;

    if ( arguments.size() < 3 ) {
        throw std::runtime_error( "Invalid number of tariff arguments" );
    }

    switch ( TariffType ) {

    case Tariff::ROOT:
        throw std::runtime_error( "Entry not found in tariff" );
        break;

    case Tariff::MULTIMPLEXION:
        return tariffByID( arguments[ 0 ] ).costs( cname, opcode )*boost::lexical_cast< float >( arguments[ 2 ] );
        break;

    case Tariff::ADDICTION:
        return tariffByID( arguments[ 0 ] ).costs( cname, opcode ) + boost::lexical_cast< float >( arguments[ 2 ] );
        break;

    case Tariff::MINIMAL:
        return std::min( tariffByID( arguments[ 0 ] ).costs( cname, opcode ), tariffByID( arguments[ 2 ] ).costs( cname, opcode ) );
        break;

    case Tariff::MAXIMAL:
        return std::max( tariffByID( arguments[ 0 ] ).costs( cname, opcode ), tariffByID( arguments[ 2 ] ).costs( cname, opcode ) );
        break;

    }
}

bool Tariff::searchForCountryPrice( std::string cname, float& res ) {
    std::list< std::pair< std::string, float > >::iterator it;
    for ( it = CFilterList.begin(); it != CFilterList.end(); it++ ) {
        std::string _cname = it->first;
        float _res = it->second;

        if ( _cname == cname ) {
            res = _res;
            return true;
        }
    }
    return false;
}

bool Tariff::searchForCountryOperatorPrice( std::string cname, std::string opcode, float& res ) {
    std::list< std::pair< std::pair< std::string, std::string >, float > >::iterator it;
    for ( it = COFilterList.begin(); it != COFilterList.end(); it++ ) {
        std::string _cname = it->first.first;
        std::string _opcode = it->first.second;
        float _res = it->second;

        if ( ( _cname == cname ) && ( _opcode == opcode ) ) {
            res = _res;
            return true;
        }
    }
    return false;
}

Tariff& Tariff::tariffByID( ID id ) {
    if ( tlist.find( id ) == tlist.end() )
        throw std::runtime_error( "Entry not found in tariff" );

    return tlist.find( id )->second;
}

void Tariff::rebuildBases() {
    std::list< std::pair< ID, std::string > >::iterator it;
    for ( it = bases.begin(); it != bases.end(); it++ ) {
        ID id = it->first;
        std::string id_fname = it->second;

        tlist.insert( std::make_pair(id, buildFromFile( id_fname ) ) );
    }
}
