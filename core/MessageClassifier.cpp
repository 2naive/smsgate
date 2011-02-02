#include "MessageClassifier.h"
#include "utils.h"
#include "Logger.h"

#include <iostream>
#include <fstream>
#include <cstdlib>

namespace sms {


    OpInfo::OpInfo() {}
    OpInfo::OpInfo(  int countrycode,
                     std::string country,
                     std::string opcode,
                     std::string opname,
                     std::string opregion) {
        this->countrycode = countrycode;
        this->country = country;
        this->opcode = opcode;
        this->opname = opname;
        this->opregion = opregion;
    }

    MessageClassifier::MessageClassifier() {
        loadOpcodes();
        loadReplacesMap();

        loadCountryOperatorMap();
    }

    void MessageClassifier::loadOpcodes() {
        ConfigManager& cfg = *ConfigManager::Instance();
        std::ostringstream out;

        out << "Loading opcodes: ";

        std::string opdb = cfg.getProperty<std::string>("system.opcodes");
        std::ifstream in(opdb.c_str());

        while ( in.good() ) {
            char l[256];
            in.getline(l, 256);
            std::vector< std::string > to_vec;
            utils::Tokenize( l, to_vec, "`" );

            if ( to_vec.size() < 6 )
                continue;

            OpInfo inf;
            inf.countrycode = atoi( to_vec[1].c_str() );
            inf.country = to_vec[2];
            inf.opcode = to_vec[3];
            inf.opname = to_vec[4];
            inf.opregion = to_vec[5];

            dict.insert( std::make_pair( to_vec[0], inf ) );

        }

        out << dict.size() << " opcodes loaded;";
        Logger::get_mutable_instance().smsloginfo( out.str() );

    }

    void MessageClassifier::loadReplacesMap() {
        ConfigManager& cfg = *ConfigManager::Instance();
        std::ostringstream out;

        out << "Loading replaces: ";

        std::string opdb = cfg.getProperty<std::string>("system.replacesmap");
        std::ifstream in(opdb.c_str());

        while ( in.good() ) {
            char l[1024];
            in.getline(l, 1024);
            std::vector< std::string > to_vec;
            utils::Tokenize( l, to_vec, "`" );

            if ( to_vec.size() != 3 )
                continue;

            replaces[ to_vec[0] ] = std::make_pair( to_vec[1], to_vec[2] );

        }

        out << replaces.size() << " replaces loaded;";
        Logger::get_mutable_instance().smsloginfo( out.str() );

    }

    void MessageClassifier::loadCountryOperatorMap() {
        comap.clear();
        {
            std::ostringstream r;

            r       << "select countries.mcc, mnc, preffix, code, countries.name, company, mccmnc.name "
                    << "from countries, mccmnc where mccmnc.mcc = countries.mcc order by mcc, mnc;";

            PGSql& db = PGSqlConnPoolSystem::get_mutable_instance().getdb();

            PGSql::ConnectionHolder cHold( db );
            ConnectionPTR conn = cHold.get();
            TransactionPTR tr = db.openTransaction( conn, "MessageClassifier::loadCountryOperatorMap()" );
            Result res = tr->exec( r.str() );
            tr->commit();
            for ( Result::const_iterator dbr = res.begin(); dbr != res.end(); dbr++ ) {
                CountryOperatorInfo coinfo;
                coinfo.mcc = (*dbr)[0].as< int >();
                coinfo.mnc = (*dbr)[1].as< int >();
                coinfo.cPreffix = (*dbr)[2].as< std::string >();
                coinfo.cCode= (*dbr)[3].as< std::string >();
                coinfo.cName = (*dbr)[4].as< std::string >();
                coinfo.opCompany = (*dbr)[5].as< std::string >();
                coinfo.opName = (*dbr)[6].as< std::string >();

                comap[ coinfo.mcc ][ coinfo.mnc ] = coinfo;
            }
        }
    }

    std::string MessageClassifier::applyReplace( std::string phone ) {
        for ( int i = phone.length(); i > 0 ; i-- ) {
            std::string s = phone.substr(0, i );
            if ( replaces.find( s ) != replaces.end() ) {
                phone.replace( 0, replaces[ s ].first.size(), replaces[ s ].second );
                return phone;
            }
        }
        return phone;
    }

    OpInfo MessageClassifier::getMsgClass( std::string phone ) {
        for ( int i = phone.length(); i > 0 ; i-- ) {
            std::string s = phone.substr(0, i );
            if ( dict.find( s ) != dict.end() ) {
                if ( dict.count( s ) == 1) {
                    OpInfo inf = dict.equal_range(s).first->second;
                    return inf;
                }

                DictT::iterator it;
                OpInfo inf = dict.equal_range( s ).first->second;
                for ( it = dict.equal_range( s ).first; it != dict.equal_range( s ).second; it++ ) {
                    if ( it->second.country != inf.country ) {
                        inf.country = "unknown";
                        inf.countrycode = 0;
                        inf.opname = "unknown";
                        inf.opcode = "0";
                        inf.opregion = "unknown";
                    }
                    if ( it->second.opname != inf.opname ) {
                        inf.opname = "unknown";
                    }
                    if ( it->second.opregion != inf.opregion ) {
                        inf.opregion = "unknown";
                    }
                }

                return inf;
            }
        }
        OpInfo unknown( 0, "0", "unknown", "unknown", "unknown" );
        return unknown;
    }

    MessageClassifier::CountryOperatorT MessageClassifier::getCOMap() {
        CountryOperatorT res;
        for ( DictT::iterator it = dict.begin(); it != dict.end(); it++ ) {
            OpInfo inf = it->second;
            res[ inf.country ].insert( inf.opname );
        }
        return res;
    }

    MessageClassifier::CountryOperatorMapT MessageClassifier::getCOMap_v2() {
        return comap;
    }

}
