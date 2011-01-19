#ifndef PERSONALPAGE_H
#define PERSONALPAGE_H

#include <Wt/WApplication>
#include <Wt/WLabel>
#include <Wt/WContainerWidget>
#include <Wt/WLineEdit>
#include <Wt/WVBoxLayout>
#include <Wt/WPushButton>
#include <Wt/WOverlayLoadingIndicator>
#include <Wt/WDefaultLoadingIndicator>
#include <Wt/WTable>
#include <Wt/WDatePicker>
#include <Wt/WComboBox>

#include "PartnerManager.h"
#include "RequestTracker.h"
#include "PGSql.h"
#include "AdminPage.h"

using namespace Wt;

class WHintLineEdit: public WLineEdit {
public:
    WHintLineEdit( WContainerWidget *parent=0 ): WLineEdit( parent ) {}
    WHintLineEdit( const WString &content, WContainerWidget *parent=0 ): WLineEdit( content ) {
        hint = content;

        focussed().connect( SLOT( this, WHintLineEdit::onFocus ) );
        blurred().connect( SLOT( this, WHintLineEdit::onFocusLost ) );
    }
private:
    WString hint;

    void onFocus() {
        if ( text() == hint ) {
            setText( "" );
        }
    }

    void onFocusLost() {
        if ( text() == "" ) {
            setText( hint );
        }
    }
};

class WHintLinePassEdit: public WLineEdit {
public:
    WHintLinePassEdit( WContainerWidget *parent=0 ): WLineEdit( parent ) {}
    WHintLinePassEdit( const WString &content, WContainerWidget *parent=0 ): WLineEdit( content ) {
        hint = content;

        focussed().connect( SLOT( this, WHintLinePassEdit::onFocus ) );
        blurred().connect( SLOT( this, WHintLinePassEdit::onFocusLost ) );
    }
private:
    WString hint;

    void onFocus() {
        if ( text() == hint ) {
            setText( "" );
            setEchoMode( WLineEdit::Password );
        }
    }

    void onFocusLost() {
        if ( text() == "" ) {
            setEchoMode( WLineEdit::Normal );
            setText( hint );
        }
    }
};

class WScrollTable: public Wt::WTable {
public:
    WScrollTable( WContainerWidget *parent=0 ): WTable( parent ) {}
    ~WScrollTable();
};

template < class RowType >
class WDataSource {
public:
    typedef std::vector< RowType > RowList;
    int getTotalLines();

    RowList& getLineRange( int lnl, int lnr );
    void releaseCache();
private:
    virtual void cacheFill( int lnl, int lnr ) = 0;

    RowList cache;
    int __lines;
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
    MsgidList genMsgIds( const std::string& _idp, const std::string& phone, const std::string& _ldate, const std::string& _rdate, const std::string& _text, int page );
    ReqResp genReq( const MsgidList& list, int status );
    void onReportGenerate();

    static PGSql& db;

};

WApplication *createPersonalPage(const WEnvironment& env);

#endif // PERSONALPAGE_H
