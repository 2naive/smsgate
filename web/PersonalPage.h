#ifndef PERSONALPAGE_H
#define PERSONALPAGE_H

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <Wt/WApplication>
#include <Wt/WLabel>
#include <Wt/WContainerWidget>
#include <Wt/WLineEdit>
#include <Wt/WVBoxLayout>
#include <Wt/WHBoxLayout>
#include <Wt/WPushButton>
#include <Wt/WTable>
#include <Wt/WDatePicker>
#include <Wt/WComboBox>
#include <Wt/WSpinBox>
#include <Wt/WDialog>
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

class WStatPageHeader: public WDataSource< vector< WWidget* > > {
public:
    typedef vector< WWidget* > Row;
    typedef WDataSource< vector< WWidget* > >::RowList RowList;

    WStatPageHeader( PersonalPage* _ppage );

    virtual int getTotalLines();
private:
    virtual void execute( int lnl, int lnr, RowList &data );
    PersonalPage* ppage;
};

class WStatPageFooter: public WDataSource< vector< WWidget* > > {
public:
    typedef vector< WWidget* > Row;
    typedef WDataSource< vector< WWidget* > >::RowList RowList;

    WStatPageFooter( PersonalPage* _ppage ) { ppage = _ppage; }

    virtual int getTotalLines() { return 0; }
private:
    virtual void execute( int lnl, int lnr, RowList &data ) { }

    PersonalPage* ppage;
};

class WStatPageData: public WDataSource< vector< WWidget* > > {
public:
    typedef vector< WWidget* > Row;
    typedef WDataSource< vector< WWidget* > >::RowList RowList;

    WStatPageData( PersonalPage* _ppage );
    ~WStatPageData( );

    void prepareRequest( );
    void resetFilter( );
    void setPidFilter( string pid );
    void setPhoneFilter( string phone );
    void setDateFromFilter( long date_from );
    void setDateToFilter( long date_to );
    void setTextFilter( string text );
    void setCountryFilter( string country );
    void setStatusFilter( SMSMessage::Status status );

    void evaluateSummary( double &price, int &total, int &delivered, int &rejected, int &undelivered );

    virtual int getTotalLines();
private:
    virtual void execute( int lnl, int lnr, RowList &data );

    PersonalPage* ppage;
    bool initialized;
    bool pid_filter; string pid_value;
    bool phone_filter; string phone_value;
    bool date_from_filter; long date_from_value;
    bool date_to_filter; long date_to_value;
    bool text_filter; string text_value;
    bool country_filter; string country_value;
    bool status_filter; SMSMessage::Status status_value;

    string view_name;
    string res_name;
    int __total_lines;
};

class PersonalPage : public WApplication {
public:
    PersonalPage( const WEnvironment& env );

private:
    typedef std::vector< std::pair< sms::SMSMessage::ID, std::string > > MsgidList;
    typedef std::vector< std::vector<std::string> > ReqResp;

    string pId;
    bool authorized;
    bool isAdmin;

    WStatPageHeader header;
    WStatPageData data;
    WStatPageFooter footer;

    WScrollTable* statistics;

    WContainerWidget* buildLoginPage( const WEnvironment& env );
    void buildPersonalPage( );
    void onLogin();
    void onReportBtnClicked(
            WLineEdit* pid,
            WLineEdit* phone,
            WDatePicker* date_from,
            WDatePicker* date_to,
            std::pair<WLineEdit*, WLineEdit*>  text_country,
            std::pair<WComboBox*, WSpinBox*>  status_page,
            WPushButton* reportbtn,
            WLabel* report_status );
    void onPageUpdate( WSpinBox* page );
    void onPageInc( WSpinBox* page );
    void onPageDec( WSpinBox* page );

    void onSummaryShow(  );


    static PGSql& db;
    friend class WStatPageHeader;
    friend class WStatPageFooter;
    friend class WStatPageData;

};

WApplication *createPersonalPage(const WEnvironment& env);

#endif // PERSONALPAGE_H
