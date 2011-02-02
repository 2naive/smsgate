#include "LoginBlock.h"
#include "PartnerManager.h"

#include <Wt/WVBoxLayout>

using namespace Wt;
using namespace std;

LoginBlock::LoginBlock( WContainerWidget* parent ):
    WContainerWidget( parent ),
    authorized( false ),
    isAdmin( false )
{

    WString failedMsg = WString::fromUTF8( "Логин или пароль неверны" );
    WString greetMsg = WString::fromUTF8( "Представьтесь, пожалуйста" );
    WString unameMsg = WString::fromUTF8( "Ваш логин" );
    WString passMsg = WString::fromUTF8( "Ваш пароль" );
    WString loginMsg = WString::fromUTF8( "Вход" );

    failedMsgLabel = new WLabel( failedMsg );
    greetMsgLabel = new WLabel( greetMsg );
    loginBox = new WHintLineEdit( unameMsg );
    passBox = new WHintLinePassEdit( passMsg );
    loginBtn = new WPushButton( loginMsg );

    failedMsgLabel->setStyleClass( "failed" );
    failedMsgLabel->setHidden( true );

    loginBox->enterPressed().connect(SLOT(passBox, WHintLinePassEdit::setFocus));

    passBox->enterPressed().connect(SLOT(this, LoginBlock::onLoginEvent));

    loginBtn->setMaximumSize( WLength( 50, WLength::Percentage ), WLength::Auto );
    loginBtn->clicked().connect(SLOT(this, LoginBlock::onLoginEvent));

    WVBoxLayout *wLoginGrp = new WVBoxLayout();
    wLoginGrp->addWidget( greetMsgLabel );
    wLoginGrp->addWidget( loginBox );
    wLoginGrp->addWidget( passBox );
    wLoginGrp->addWidget( failedMsgLabel );
    wLoginGrp->addWidget( loginBtn );

    setLayout( wLoginGrp, AlignMiddle | AlignCenter );
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
        return;
    }

    if ( !authorized ) {
        failedMsgLabel->setHidden( false );
        passBox->setText( "" );
        passBox->setFocus( );
    } else {
        onLogin_.emit( pId, isAdmin );
    }
}
