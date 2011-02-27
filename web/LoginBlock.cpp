#include "LoginBlock.h"
#include "PartnerManager.h"

#include <Wt/WHBoxLayout>
#include <Wt/WGridLayout>

#include <Wt/WImage>

using namespace Wt;
using namespace std;

LoginBlock::LoginBlock( WContainerWidget* parent ):
    WContainerWidget( parent ),
    authorized( false ),
    isAdmin( false )
{

    WString failedMsg = WString::fromUTF8( "Login failed" );
    WString unameMsg = WString::fromUTF8( "login" );
    WString passMsg = WString::fromUTF8( "password" );

    failedMsgLabel = new WLabel( failedMsg );
    loginBox = new WHintLineEdit( unameMsg );
    loginBox->setMinimumSize( WLength( 100, WLength::Pixel ), WLength::Auto );
    passBox = new WHintLinePassEdit( passMsg );
    passBox->setMinimumSize( WLength( 100, WLength::Pixel ), WLength::Auto );
    passBox->enterPressed().connect(SLOT(this, LoginBlock::onLoginEvent));

    failedMsgLabel->setStyleClass( "failed" );
    failedMsgLabel->setHidden( true );

    WImage *okBtn = new WImage( "resources/ok.png" );
    okBtn->setStyleClass( "link" );
    okBtn->setMaximumSize( WLength::Auto, WLength( 20, WLength::Pixel ) );
    okBtn->clicked().connect(SLOT(this, LoginBlock::onLoginEvent));

    WGridLayout *wLoginGrp = new WGridLayout();
    wLoginGrp->addWidget( loginBox, 0, 0, 0, 0 );
    wLoginGrp->addWidget( passBox, 0, 1, 0, 0 );
    wLoginGrp->addWidget( okBtn, 0, 2, 0, 0 );
    wLoginGrp->addWidget( failedMsgLabel, 1, 0, 0, 2 );

//    WContainerWidget* loginPassBlock = new WContainerWidget();
//    loginPassBlock->setLayout( wLoginGrp );

    WImage* logo = new WImage( "resources/Greendsms.png" );
    logo->setMaximumSize( WLength::Auto, WLength( 60, WLength::Pixel ));

    WImage* underDev = new WImage( "resources/devel.png" );

    WGridLayout *pageGroup = new WGridLayout();
    pageGroup->addWidget( logo, 0, 0, 0, 0 );
    pageGroup->addLayout( wLoginGrp, 0, 2, 0, 0, AlignBottom );
    pageGroup->addWidget( underDev, 1, 1, 0, 0 );

    setMargin( WLength( 5, WLength::Pixel ) );
    setLayout( pageGroup, AlignMiddle | AlignCenter );
}

void LoginBlock::onLoginEvent() {
    authorized = false;
    isAdmin = false;

    string pId;

    try {
        PartnerInfo p = PartnerManager::get_mutable_instance().findByName( loginBox->text().toUTF8() );
        authorized = ( p.pPass == passBox->text().toUTF8() );
        pId = p.pId;
        isAdmin = p.isAdmin();
    } catch ( ... ) {
    }

    if ( !authorized ) {
        failedMsgLabel->setHidden( false );
        passBox->setText( "" );
        passBox->setFocus( );
    } else {
        onLogin_.emit( pId, isAdmin );
    }
}
