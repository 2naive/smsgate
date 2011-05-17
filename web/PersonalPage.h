#ifndef PERSONALPAGE_H
#define PERSONALPAGE_H

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <Wt/WApplication>

#include "StatisticsBlock.h"
#include "TariffEditor.h"
#include "LoginBlock.h"
#include "PartnerManager.h"

using namespace Wt;


class PersonalPage : public WApplication {
public:
    PersonalPage( const WEnvironment& env );

private:

    string pId;
    bool authorized;
    bool isAdmin;

    LoginBlock* wLoginBox;
    StatisticsBlock* wStatBlock;
    TariffEditor* wTariffEditor;

    void buildPersonalPage( );

    WContainerWidget* buildStatisticsBlock( );
    WContainerWidget* buildTariffEditor( );

    void onLogin( string pId, bool isAdmin );
};

WApplication *createPersonalPage(const WEnvironment& env);

#endif // PERSONALPAGE_H
