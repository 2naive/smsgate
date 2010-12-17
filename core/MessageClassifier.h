#ifndef MESSAGECLASSIFIER_H
#define MESSAGECLASSIFIER_H

#include "ConfigManager.h"
#include <set>

namespace sms {
    struct OpInfo;

    class MessageClassifier
    {
    public:

        typedef std::multimap< std::string, OpInfo > DictT;
        typedef std::map< std::string, std::pair< std::string, std::string > > ReplaceT;
        typedef std::map< std::string, std::set< std::string > > CountryOperatorT;
        MessageClassifier();
        static MessageClassifier* Instance() {
            if (!pInstance_)
                pInstance_ = new MessageClassifier;
            return pInstance_;
        }

        OpInfo getMsgClass( std::string phone );
        std::string applyReplace( std::string phone );
        CountryOperatorT getCOMap();

    private:
        DictT dict;
        ReplaceT replaces;
        static MessageClassifier* pInstance_;

        void loadOpcodes();
        void loadReplacesMap();
    };

    struct OpInfo {               
        int countrycode;
        std::string country;
        std::string opcode;
        std::string opname;
        std::string opregion;

        typedef std::map< std::string, std::string > CostMapT;
        CostMapT costs;

        OpInfo( int ccode,
                std::string clcode,
                std::string opcode,
                std::string opname,
                std::string opregion);

        OpInfo();
    };

}

#endif // MESSAGECLASSIFIER_H
