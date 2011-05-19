#ifndef PARTNERMANAGER_H
#define PARTNERMANAGER_H

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>

#include <boost/serialization/singleton.hpp>
#include <sstream>
#include "PGSql.h"
#include "ConfigManager.h"
#include "Logger.h"
#include "Error.h"
#include "Tariff.h"

using std::string;
using namespace boost::multi_index;

class PartnerNotFoundError: public sms::FailureError{};
typedef boost::error_info<struct tag_pName,const char*> throw_pName;
typedef boost::error_info<struct tag_pId,const char*> throw_pId;

struct PartnerInfo {
    string pName;
    string pPass;
    string pId;
    string pCName;
    string pManager;
    string phone;
    string text;
    Tariff tariff;
    double pBalance;
    double pCredit;
    int pLimit;
    bool pPostPay;
    bool pIsTrial;
    int pPriority;
    int tzone;

    PartnerInfo(string pName = "",
                string pPass = "",
                string pId = "",
                string pCName = "",
                string pManager = "",
                string phone = "",
                string text = "",
                string tariff = "",
                double pBalance = 50.0,
                double pCredit = 0.0,
                int pLimit = 60,
                bool pPostPay = 0,
                bool pIsTrial = 0,
                int pPriority = 0,
                int tzone = 4 ) {
        this->pName = pName;
        this->pPass = pPass;
        this->pId = pId;
        this->pCName = pCName;
        this->pManager = pManager;
        this->phone = phone;
        this->text = text;
        try {
            this->tariff = TariffManager::get_mutable_instance().loadTariff( tariff );
        } catch ( std::exception& err ) {
            Logger::get_mutable_instance().smslogerr( err.what() );
        }

        this->pBalance = pBalance;
        this->pCredit = pCredit;
        this->pLimit = pLimit;
        this->pPostPay = pPostPay;
        this->pIsTrial = pIsTrial;
        this->pPriority = pPriority;
        this->tzone = tzone;
    }

    bool isAdmin();
};


class PartnerManager: public boost::serialization::singleton< PartnerManager > {
public:

    PartnerManager();
    PartnerInfo findByName( string pName ) throw ( PartnerNotFoundError );
    PartnerInfo findById( string id ) throw ( PartnerNotFoundError );
    std::list< PartnerInfo > getAll();

    void loadFromDb();

private:
    typedef boost::multi_index::multi_index_container<
                PartnerInfo,
                indexed_by<
                    sequenced<>,
                    ordered_unique< member<PartnerInfo, string, &PartnerInfo::pName> >,
                    ordered_unique< member<PartnerInfo, string, &PartnerInfo::pId> >
                >
            > pBox;
    pBox pbox;
    boost::recursive_mutex pmlock;
    int tid;

};

#endif // PARTNERMANAGER_H
