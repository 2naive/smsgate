#include "NumberInfoPage.h"
#include "MessageClassifier.h"
#include "StatManager.h"
#include "SMPPGateManager.h"

#include <boost/lexical_cast.hpp>

WApplication *createNumberInfoPage(const WEnvironment& env) {
    return new NumberInfoPage( env );
}

NumberInfoPage::NumberInfoPage( const WEnvironment& env ):WApplication( env ) {
    setTitle( "SMSGate countries-gates stats page" );

    this->setCssTheme("polished");

    phoneInput = new Wt::WLineEdit( WString::fromUTF8("Номер абонента") );
    phoneInput->setMinimumSize( WLength( 6, WLength::Centimeter ), WLength::Auto );
    phoneInput->focussed().connect( SLOT( this, NumberInfoPage::onPhoneEditFocus ) );
    phoneInput->blurred().connect( SLOT( this, NumberInfoPage::onPhoneEditFocusLost ) );
    phoneInput->keyWentDown().connect( SLOT( this, NumberInfoPage::onPhonePrintInfo ) );

    phoneBtn = new Wt::WPushButton( WString::fromUTF8("ОК") );
    phoneBtn->setMinimumSize( WLength( 3, WLength::Centimeter ), WLength::Auto );
    phoneBtn->setMaximumSize( WLength( 3, WLength::Centimeter ), WLength::Auto );
    phoneBtn->clicked().connect( SLOT( this, NumberInfoPage::onPhonePrintInfo ) );

    phoneResult = new Wt::WTextArea();
    phoneResult->setMinimumSize( Wt::WLength( 14, WLength::Centimeter ), Wt::WLength( 8, WLength::Centimeter ) );
    phoneResult->setReadOnly( true );


    WVBoxLayout* bv = new Wt::WVBoxLayout();
    WHBoxLayout* bh = new Wt::WHBoxLayout();

    WContainerWidget* fl = new Wt::WContainerWidget();
    WContainerWidget* sl = new Wt::WContainerWidget();

    bh->addWidget( phoneInput );
    bh->addWidget( phoneBtn );
    fl->setLayout( bh, AlignTop | AlignJustify );
    bv->addWidget( fl );
    bv->addWidget( phoneResult );
    sl->setLayout( bv, AlignTop  );

    root()->addWidget( sl );
}

void NumberInfoPage::onPhoneEditFocus() {
    if ( phoneInput->text() == WString::fromUTF8("Номер абонента") ) {
        phoneInput->setText( "" );
    }
}
void NumberInfoPage::onPhoneEditFocusLost() {
    if ( phoneInput->text() == "" ) {
        phoneInput->setText( WString::fromUTF8("Номер абонента") );
    }
}

void NumberInfoPage::onPhonePrintInfo() {
    if ( phoneInput->text() == "" ) return;
    if ( phoneInput->text() == WString::fromUTF8("Номер абонента") ) return;

    std::ostringstream out;
    out.precision(2);
    sms::OpInfo msg = sms::MessageClassifier::Instance()->getMsgClass( phoneInput->text().toUTF8() );

    out << "Код страны: " <<  msg.countrycode << std::endl;
    out << "Страна: " <<  msg.country << std::endl;
    out << "Оператор: " <<  msg.opname << std::endl;
    out << "Код оператора: " <<  msg.opcode << std::endl;
    out << "Регион: " << msg.opregion << std::endl << std::endl;

    SMPPGateManager::SMPPGatesMap gm = SMPPGateManager::Instance()->getGates();

    sms::StatManager::TCountryInfoTable tcd = sms::StatManager::Instance()->getCountryInfoLastUpdate();

    {
        out << "Информация по ценам:" << std::endl;
        for ( SMPPGateManager::SMPPGatesMap::iterator it = gm.begin(); it != gm.end(); it++ ) {
            sms::CountryInfo msgci;
            double c;
            try {
                c = it->second.getTariff().costs( msg.country, msg.opcode );
            } catch ( ... ) {
                c = -1;
            }
            bool found = false;
            for ( int i = 0; i < tcd.size(); i++ ) {
                for ( int j = 0; j < tcd[i].size(); j++ ) {
                    sms::CountryInfo ci = tcd[i][j];
                    if (
                            ( msg.country == ci.cname ) &&
                            ( msg.opname == ci.opname ) &&
                            ( it->first == ci.gname )
                            ) {
                        found = true;
                        msgci = tcd[i][j];
                    }

                }
            }
            double quality = double(msgci.deliveres*100) / ( msgci.requests == 0 ? 1: msgci.requests );
            if ( quality >= 100 ) { quality = 100; }
            try {
                if( c == -1 )
                    out     << std::setw( 8 ) << std::left << it->first << ": "
                            << std::setw( 9 ) << " ----- " << "Направление закрыто" << std::endl;
                else {
                    if ( found && msgci.requests > 0 ) {
                        double average_price = c / 100 / quality == 0? 0.0001: quality;
                        double average_time = 60 * (1 - quality == 100? 0.9999: quality/100 ) + 20;
                        out << std::setw( 8 ) << std::left <<  it->first << ": €"
                            << std::setw( 8 ) << std::left << c/100 << "Доставка " << quality << "% "
                            << "по средней цене €" << std::setw( 8 ) << std::left << average_price
                            << "за " << std::setw( 4 ) << std::left << average_time << "секунд" << std::endl;
                    } else {
                        out << std::setw( 8 ) << std::left <<  it->first << ": €"
                            << std::setw( 8 ) << std::left << c/100 << "Процент доставки неизвестен" << std::endl;
                    }
                }
            } catch (...) {}
        }
    }


    phoneResult->setText( WString::fromUTF8(out.str()) );
}

NumberInfoPage::~NumberInfoPage() {

}
