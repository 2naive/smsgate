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
    double pBalance;
    double pCredit;
    int pLimit;
    bool pPostPay;
    bool pIsTrial;
    int pPriority;

    PartnerInfo( string pName, string pPass, string pId, string pCName, string pManager, double pBalance, double pCredit, int pLimit, bool pPostPay, bool pIsTrial, int pPriority ) {
        this->pName = pName;
        this->pPass = pPass;
        this->pId = pId;
        this->pCName = pCName;
        this->pManager = pManager;
        this->pBalance = pBalance;
        this->pCredit = pCredit;
        this->pLimit = pLimit;
        this->pPostPay = pPostPay;
        this->pIsTrial = pIsTrial;
        this->pPriority = pPriority;
    }
};


class PartnerManager: public boost::serialization::singleton< PartnerManager > {
public:

    PartnerManager();
    PartnerInfo findByName( string pName ) throw ( PartnerNotFoundError );
    PartnerInfo findById( string id ) throw ( PartnerNotFoundError );

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
