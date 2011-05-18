#include "PartnerEditor.h"
#include "PartnerManager.h"

#include <Wt/WHBoxLayout>

using namespace Wt;

PartnerOptions::PartnerOptions( std::string _pid, Wt::WContainerWidget *parent ): WContainerWidget( parent ) {
    pid = _pid;

    PartnerInfo pi = PartnerManager::get_mutable_instance().findById( pid );
    WString uv = WString::fromUTF8( "Значение не задано" );

    int cl = 0;

    WTable* tbl = new WTable();
    tbl-> setStyleClass( "restable" );
    tbl->setHeaderCount( 1 );

    WCustomInPlaceEdit* pCNameEdit = new WCustomInPlaceEdit( WString::fromUTF8( pi.pCName ), uv );
    tbl->elementAt( cl, 0 )->addWidget( pCNameEdit );
    tbl->elementAt( cl++, 0 )->setColumnSpan( 2 );


    WCustomInPlaceEdit* pPhoneEdit = new WCustomInPlaceEdit( WString::fromUTF8( pi.phone ), uv );
    tbl->elementAt( cl, 0 )->addWidget( new WLabel( WString::fromUTF8( "Контактный телефон" ) ) );
    tbl->elementAt( cl++, 1 )->addWidget( pPhoneEdit );

    tbl->elementAt( cl, 0 )->addWidget( new WLabel( WString::fromUTF8( "Часовой пояс ( UTC )" ) ) );
    tbl->elementAt( cl++, 1 )->addWidget( new WCustomInPlaceEdit( WString::fromUTF8( boost::lexical_cast< std::string >( pi.tzone ) ), uv ) );

    tbl->elementAt( cl, 0 )->addWidget( new WLabel( WString::fromUTF8( "Менеджер" ) ) );
    tbl->elementAt( cl++, 1 )->addWidget( new WCustomInPlaceEdit( WString::fromUTF8( pi.pManager ), uv ) );

    tbl->elementAt( cl, 0 )->addWidget( new WLabel( WString::fromUTF8( "Тариф" ) ) );
    tbl->elementAt( cl++, 1 )->addWidget( new WCustomInPlaceEdit( WString::fromUTF8( pi.tariff.getName() ), uv ) );

    tbl->elementAt( cl, 0 )->addWidget( new WLabel( WString::fromUTF8( "Тестовый аккаунт" ) ) );
    tbl->elementAt( cl++, 1 )->addWidget( new WCustomInPlaceEdit( WString::fromUTF8( boost::lexical_cast< std::string >( pi.pIsTrial ) ), uv ) );

    tbl->elementAt( cl, 0 )->addWidget( new WLabel( WString::fromUTF8( "Приоритет" ) ) );
    tbl->elementAt( cl++, 1 )->addWidget( new WCustomInPlaceEdit( WString::fromUTF8( boost::lexical_cast< std::string >( pi.pPriority ) ), uv ) );

    tbl->elementAt( cl, 0 )->addWidget( new WLabel( WString::fromUTF8( "Постоплата" ) ) );
    tbl->elementAt( cl++, 1 )->addWidget( new WCustomInPlaceEdit( WString::fromUTF8( boost::lexical_cast< std::string >( pi.pPostPay ) ), uv ) );

    tbl->elementAt( cl, 0 )->addWidget( new WLabel( WString::fromUTF8( "Баланс" ) ) );
    tbl->elementAt( cl++, 1 )->addWidget( new WCustomInPlaceEdit( WString::fromUTF8( boost::lexical_cast< std::string >( pi.pBalance ) ), uv ) );

    tbl->elementAt( cl, 0 )->addWidget( new WLabel( WString::fromUTF8( "Кредитный лимит" ) ) );
    tbl->elementAt( cl++, 1 )->addWidget( new WCustomInPlaceEdit( WString::fromUTF8( boost::lexical_cast< std::string >( pi.pCredit ) ), uv ) );

    tbl->elementAt( cl, 0 )->addWidget( new WLabel( WString::fromUTF8( "Пропускная способность" ) ) );
    tbl->elementAt( cl++, 1 )->addWidget( new WCustomInPlaceEdit( WString::fromUTF8( boost::lexical_cast< std::string >( pi.pLimit ) ), uv ) );

    WHBoxLayout* lyt = new WHBoxLayout();
    lyt->addWidget( tbl, AlignCenter | AlignMiddle | AlignJustify );

    setLayout( lyt, AlignCenter );
}

PartnerEditor::PartnerEditor( Wt::WContainerWidget* parent ):WContainerWidget( parent ) {

    std::list< PartnerInfo > lst = PartnerManager::get_mutable_instance().getAll();

    for ( std::list< PartnerInfo >::iterator it = lst.begin(); it != lst.end(); it++ ) {
        std::string pid = it->pId;
        addWidget( new PartnerOptions( pid ) );
    }
}
