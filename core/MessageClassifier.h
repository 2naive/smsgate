#ifndef MESSAGECLASSIFIER_H
#define MESSAGECLASSIFIER_H

#include <boost/serialization/singleton.hpp>
#include <boost/tuple/tuple.hpp>

#include "ConfigManager.h"
#include "PGSql.h"

#include <set>
#include <map>

namespace sms {
    struct OpInfo;

    class MessageClassifier: public boost::serialization::singleton< MessageClassifier > {
    public:
        struct OperatorInfo {
            std::string mcc;
            std::string mnc;
            std::string opCompany;
            std::string opName;
            std::string opRegion;

            std::string getName() {
                if ( opName.empty() )
                    return opCompany;

                if ( opCompany.empty() )
                    return opName;

                return opName + std::string("(") + opCompany + std::string(")");
            }
        };

        struct CountryInfo {
            std::string mcc;
            std::string cCode;
            std::string cName;
            std::string cPreffix;

            typedef std::map< std::string, OperatorInfo > OperatorMapT;
            OperatorMapT operators;
        };

        typedef std::multimap< std::string, OpInfo > DictT;
        typedef std::map< std::string, std::pair< std::string, std::string > > ReplaceT;
        typedef std::map< std::string, std::set< std::string > > CountryOperatorT;
        typedef std::multimap< std::string, boost::tuples::tuple< std::string, std::string, std::string > > PreffixMapT;

        typedef std::map< std::string, CountryInfo > CountryOperatorMapT;

        MessageClassifier();

        OpInfo getMsgClass( std::string phone );
        CountryInfo getMsgClass_v2( std::string phone );
        std::string applyReplace( std::string phone );
        CountryOperatorT getCOMap();

        CountryOperatorMapT getCOMap_v2();

    private:
        DictT dict;
        ReplaceT replaces;
        CountryOperatorMapT comap;
        PreffixMapT preffmap;

        void loadOpcodes();
        void loadReplacesMap();
        void loadCountryOperatorMap();
        void loadRoutingMap();
    };

    struct OpInfo {               
        int countrycode;
        std::string country;
        std::string opcode;
        std::string opname;
        std::string opregion;

        typedef std::map< std::string, std::string > CostMapT;

        OpInfo( int ccode,
                std::string clcode,
                std::string opcode,
                std::string opname,
                std::string opregion);

        OpInfo();
    };

}

#endif // MESSAGECLASSIFIER_H
