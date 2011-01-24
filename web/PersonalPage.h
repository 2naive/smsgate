#ifndef PERSONALPAGE_H
#define PERSONALPAGE_H

#include <boost/function.hpp>

#include <Wt/WApplication>
#include <Wt/WLabel>
#include <Wt/WContainerWidget>
#include <Wt/WLineEdit>
#include <Wt/WVBoxLayout>
#include <Wt/WHBoxLayout>
#include <Wt/WPushButton>
#include <Wt/WOverlayLoadingIndicator>
#include <Wt/WDefaultLoadingIndicator>
#include <Wt/WTable>
#include <Wt/WDatePicker>
#include <Wt/WComboBox>
#include "WHintLineEdit.h"
#include "WHintLinePassEdit.h"
#include "WScrollTable.h"

#include "PartnerManager.h"
#include "RequestTracker.h"
#include "PGSql.h"
#include "AdminPage.h"
#include "DataSource.h"

using namespace Wt;

class PersonalPage;

class WStatPageHeader: public WDataSource< vector< boost::shared_ptr< WWidget* > > > {
public:
    typedef vector< boost::shared_ptr< WWidget > > Row;
    typedef WDataSource< vector< boost::shared_ptr< WWidget > > >::RowList RowList;

    WStatPageHeader( PersonalPage* _ppage );

    virtual int getTotalLines();
    virtual void execute( int lnl, int lnr, RowList &data );
private:
    PersonalPage* ppage;
};

class WStatPageFooter: public WDataSource< vector< boost::shared_ptr< WWidget* > > > {
public:
    typedef vector< boost::shared_ptr< WWidget > > Row;
    typedef WDataSource< vector< boost::shared_ptr< WWidget > > >::RowList RowList;

    WStatPageFooter( PersonalPage* _ppage );

    virtual int getTotalLines();
    virtual void execute( int lnl, int lnr, RowList &data );
private:
    PersonalPage* ppage;
};

class PersonalPage : public WApplication {
public:
    PersonalPage( const WEnvironment& env );

private:
    typedef std::vector< std::pair< sms::SMSMessage::ID, std::string > > MsgidList;
    typedef std::vector< std::vector<std::string> > ReqResp;

    string pId;
    bool authorized;
    WTable* tbl;
    WPushButton* reportbtn;
    WLabel* reportstatus;

    WLineEdit* pid;
    WLineEdit* phone;
    WDatePicker* date_from, *date_to;
    WLineEdit* text;
    WComboBox* status;

    WContainerWidget* buildLoginPage( const WEnvironment& env );
    void buildPersonalPage( );
    void onLogin();
    void onReportBtnClicked(
            WLineEdit* pid,
            WLineEdit* phone,
            WDatePicker* date_from,
            WDatePicker* date_to,
            WLineEdit* text,
            WComboBox* status,
            WPushButton* reportbtn,
            WLabel* report_status );

    MsgidList genMsgIds( const std::string& _idp, const std::string& phone, const std::string& _ldate, const std::string& _rdate, const std::string& _text, int page );
    ReqResp genReq( const MsgidList& list, int status );
    void onReportGenerate();

    static PGSql& db;
    friend class WStatPageHeader;

};

WApplication *createPersonalPage(const WEnvironment& env);

#endif // PERSONALPAGE_H
