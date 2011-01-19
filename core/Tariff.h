#ifndef TARIFF_H
#define TARIFF_H

#include <string>
#include <list>
#include <vector>
#include <map>
#include <boost/serialization/singleton.hpp>
#include <boost/serialization/nvp.hpp>

#include "MessageClassifier.h"

class Tariff {
public:
    typedef std::string ID;
    typedef std::pair< ID, int > TariffDescriptor;
    enum Type {
        ROOT,
        MULTIMPLEXION,
        ADDICTION,
        MINIMAL,
        MAXIMAL
    };

    static Tariff buildFromFile( const std::string& filename );
    void saveToFile( const std::string& filename );

    static Tariff buildEmpty( ID name );
    static Tariff buildInherit( ID name, const std::string& filename );
    static Tariff buildInheritMultiplex( ID name, const std::string& filename, float mult );
    static Tariff buildInheritAdd( ID name, const std::string& filename, float add );
    static Tariff buildInheritMin( ID name, const std::string& filename1, const std::string& filename2 );
    static Tariff buildInheritMax( ID name, const std::string& filename1, const std::string& filename2 );

    static Tariff buildClone( ID name, const Tariff& base );

    void addFilterCountry( std::string cname, float price );
    void addFilterCountryOperator( std::string cname, std::string opcode, float price );
    float costs( sms::OpInfo& op ) const;
    float costs( std::string cname, std::string opcode ) const;

    template<class Archive>
        void serialize(Archive & ar, const unsigned int) {
            ar & BOOST_SERIALIZATION_NVP(TariffType);
            ar & BOOST_SERIALIZATION_NVP(name);
            ar & BOOST_SERIALIZATION_NVP(bases);
            ar & BOOST_SERIALIZATION_NVP(arguments);
            ar & BOOST_SERIALIZATION_NVP(COFilterList);
            ar & BOOST_SERIALIZATION_NVP(CFilterList);
        }

private:
    int TariffType;
    ID name;
    std::list< std::pair< ID, std::string > > bases;
    std::vector< std::string > arguments;
    std::list< std::pair< std::pair< std::string, std::string >, float > > COFilterList;
    std::list< std::pair< std::string, float > > CFilterList;

    std::map< ID, Tariff > tlist;

    Tariff();
    Tariff( ID name, int tarifftype );
    bool searchForCountryPrice( std::string cname, float& res ) const;
    bool searchForCountryOperatorPrice( std::string cname, std::string opcode, float& res ) const;
    void rebuildBases();
    const Tariff& tariffByID( ID id ) const;
};

#endif // TARIFF_H
