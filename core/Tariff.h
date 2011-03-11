#ifndef TARIFF_H
#define TARIFF_H

#include <string>
#include <list>
#include <vector>
#include <map>
#include <boost/serialization/singleton.hpp>
#include <boost/serialization/nvp.hpp>

#include <boost/logic/tribool.hpp>

#include "MessageClassifier.h"
#include "PGSql.h"

class TariffOption {
public:
    enum ValuesPolicy {
        VALUE_MULTIPLE,
        VALUE_SINGLE
    };

    TariffOption() {
        valuesList = valuesSupported();
        optionName = getOptionName();
        optionDescr = getOptionDescription();
    }

    std::string name() { return optionName; }
    std::string decsription() { return optionDescr; }

    virtual ValuesPolicy valuesPolicy() = 0;

protected:
    typedef std::map< std::string, std::string > ValuesListT;
    virtual ValuesListT valuesSupported() = 0;
    virtual std::string getOptionName( ) = 0;
    virtual std::string getOptionDescription( ) = 0;

private:
    ValuesListT valuesList;
    std::string optionName;
    std::string optionDescr;
};

class SingleTariffOption: public TariffOption {
public:
    TariffOption::ValuesPolicy valuesPolicy() { return TariffOption::VALUE_SINGLE; }
    virtual std::string getDefaultValue() = 0;

    SingleTariffOption() { value = getDefaultValue(); }
    SingleTariffOption( std::string _value ) {
        if ( valuesList.find( _value ) != valuesList.end() )
            value = _value;
        else
            value = getDefaultValue();
    }

    template<class Archive>
        void serialize(Archive & ar, const unsigned int) {
            ar & BOOST_SERIALIZATION_NVP( value );
        }
private:
    std::string value;
};

class MultipleTariffOption: public TariffOption {
public:
    TariffOption::ValuesPolicy valuesPolicy() { return TariffOption::VALUE_MULTIPLE; }
    virtual std::string getDefaultValues() = 0;

    MultipleTariffOption() { values = getDefaultValues(); }
    MultipleTariffOption( std::set< std::string > _values ) {
        for ( std::set< std::string >::iterator it = _values.begin(); it != _values.end(); it ++) {
            if ( valuesList.find( *it ) != valuesList.end() )
                values.insert( *it );
        }

        if ( values.empty() )
            values = getDefaultValues();
    }

    template<class Archive>
        void serialize(Archive & ar, const unsigned int) {
            ar & BOOST_SERIALIZATION_NVP( values );
        }
private:
    std::set< std::string > values;
};

class StatusPaidTariffOption: public MultipleTariffOption {
public:
    std::string getOptionName()
};

class Tariff {
public:
    static const double INVALID_VALUE = -1.0;
    enum OptionLevel {
        OPT_GLOBAL,
        OPT_COUNTRY,
        OPT_OPERATOR
    };

    Tariff( );
    Tariff( std::string name );
    Tariff( std::string name, std::string source );

    std::string serialize();

    void addFilterCountry( std::string cname, double price );
    void addFilterCountryOperator( std::string cname, std::string opcode, double price );

    double costs( std::string op );
    double costs( std::string cname, std::string opcode );

    void setName( std::string n ) { tariff.name = n; }
    std::string getName( ) { return tariff.name; };

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

    void addFilterCountry( std::string cname, float price );
    void addFilterCountryOperator( std::string cname, std::string opcode, float price );
    float costs( sms::OpInfo& op ) const;
    float costs( std::string cname, std::string opcode = "" ) const;

    template<class Archive>
        void serialize(Archive & ar, const unsigned int) {
            ar & BOOST_SERIALIZATION_NVP( tariff );
        }

private:
    TariffInfo tariff;

    boost::logic::tribool hasOption( std::string name );
    boost::logic::tribool hasOption( std::string name, std::string country );
    boost::logic::tribool hasOption( std::string name, std::string country, std::string oper );

    std::string getOption( std::string name );
    std::string getOption( std::string name, std::string country );
    std::string getOption( std::string name, std::string country, std::string oper );

    void setOption( std::string name, std::string value );
    void setOption( std::string name, std::string country, std::string value );
    void setOption( std::string name, std::string country, std::string oper, std::string value );

    void removeOption( std::string name );
    void removeOption( std::string name, std::string country );
    void removeOption( std::string name, std::string country, std::string oper );
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
    void removeTariff( std::string name );
    TariffListT tariffs_list();

private:
    TariffListT tlist;
    TariffMapT tmap;
    int updateTimerID;

    PGSql& db;
};

#endif // TARIFF_H
