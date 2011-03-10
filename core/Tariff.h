#ifndef TARIFF_H
#define TARIFF_H

#include <string>
#include <list>
#include <vector>
#include <map>
#include <boost/serialization/singleton.hpp>
#include <boost/serialization/nvp.hpp>

#include "MessageClassifier.h"
#include "PGSql.h"

class Tariff {
public:
    static const double INVALID_VALUE = -1.0;

    Tariff( );
    Tariff( std::string name );
    Tariff( std::string name, std::string source );

    std::string serialize();

    void addFilterCountry( std::string cname, double price );
    void addFilterCountryOperator( std::string cname, std::string opcode, double price );

    double costs( std::string op );
    double costs( std::string cname, std::string opcode );

    struct TariffOperatorInfo {
        std::map< std::string, std::string > options;

        template<class Archive>
            void serialize(Archive & ar, const unsigned int) {
                ar & BOOST_SERIALIZATION_NVP(options);
            }
    };

    struct TariffCountryInfo {
        std::map< std::string, std::string > options;
        std::map< std::string, TariffOperatorInfo > operators;

        template<class Archive>
            void serialize(Archive & ar, const unsigned int) {
                ar & BOOST_SERIALIZATION_NVP(options);
                ar & BOOST_SERIALIZATION_NVP(operators);
            }
    };

    struct TariffInfo {
        std::string name;
        std::map< std::string, std::string > options;
        std::map< std::string, TariffCountryInfo > countries;

        template<class Archive>
            void serialize(Archive & ar, const unsigned int) {
                ar & BOOST_SERIALIZATION_NVP(name);
                ar & BOOST_SERIALIZATION_NVP(options);
                ar & BOOST_SERIALIZATION_NVP(countries);
            }
    };

    template<class Archive>
        void serialize(Archive & ar, const unsigned int) {
            ar & BOOST_SERIALIZATION_NVP( tariff );
        }

private:
    TariffInfo tariff;
};

class TariffManager: public boost::serialization::singleton< TariffManager > {
public:
    typedef std::map< std::string, Tariff > TariffMapT;
    typedef std::list< std::string > TariffListT;

    TariffManager();
    ~TariffManager();

    void updateTariffList();
    Tariff loadTariff( std::string name );
    void saveTariff( std::string name, Tariff t );
    TariffListT tariffs_list();

private:
    TariffListT tlist;
    TariffMapT tmap;
    int updateTimerID;

    PGSql& db;
};

#endif // TARIFF_H
