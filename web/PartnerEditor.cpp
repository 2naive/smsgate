#include "PartnerEditor.h"
#include "PartnerManager.h"
#include "Tariff.h"

#include <Wt/WHBoxLayout>
#include <Wt/WVBoxLayout>
#include <Wt/WPushButton>
#include <Wt/WText>
#include <Wt/WStandardItem>
#include <Wt/WRegExpValidator>
#include <Wt/WIntValidator>
#include <Wt/WSuggestionPopup>
#include <vector>
#include <iostream>

using namespace Wt;

PartnerOptions::PartnerOptions( std::string _pid, Wt::WContainerWidget *parent ): WContainerWidget( parent ) {
    pid = _pid;
    PartnerInfo pi;

    try {
        pi = PartnerManager::get_mutable_instance().findById( pid );
    } catch ( ... ) {}
    WString uv = WString::fromUTF8( "Значение не задано" );

    int cl = 0;

    WPushButton* saveBtn;
    if ( pid.empty() )
        saveBtn = new WPushButton( WString::fromUTF8( "Создать" ) );
    else
        saveBtn = new WPushButton( WString::fromUTF8( "Сохранить" ) );

    tbl = new WTable();
    tbl->setStyleClass( "restable" );
    tbl->setHeaderCount( 1 );

    WCustomInPlaceEdit* pCNameEdit = new WCustomInPlaceEdit( WString::fromUTF8( pi.pCName ), uv );
    tbl->elementAt( cl, 0 )->addWidget( pCNameEdit );
    tbl->elementAt( cl, 1 )->setContentAlignment( AlignRight | AlignMiddle );
    tbl->elementAt( cl++, 1 )->addWidget( saveBtn );

    WCustomInPlaceEdit* pLastNameEdit = new WCustomInPlaceEdit( WString::fromUTF8( pi.phone ), uv );
    tbl->elementAt( cl, 0 )->addWidget( new WLabel( WString::fromUTF8( "Фамилия" ) ) );
    tbl->elementAt( cl++, 1 )->addWidget( pLastNameEdit );

    WCustomInPlaceEdit* pFirstNameEdit = new WCustomInPlaceEdit( WString::fromUTF8( pi.phone ), uv );
    tbl->elementAt( cl, 0 )->addWidget( new WLabel( WString::fromUTF8( "Имя" ) ) );
    tbl->elementAt( cl++, 1 )->addWidget( pFirstNameEdit );

    WCustomInPlaceEdit* pMiddleNameEdit = new WCustomInPlaceEdit( WString::fromUTF8( pi.phone ), uv );
    tbl->elementAt( cl, 0 )->addWidget( new WLabel( WString::fromUTF8( "Отчество" ) ) );
    tbl->elementAt( cl++, 1 )->addWidget( pMiddleNameEdit );

    WCustomInPlaceEdit* pEmailEdit = new WCustomInPlaceEdit( WString::fromUTF8( pi.phone ), uv );
    std::string email_match = "^[A-Z0-9._%-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}$";
    WRegExpValidator* pEmailValidator = new WRegExpValidator( email_match );
    pEmailValidator->setFlags( MatchCaseInsensitive );
    pEmailValidator->setMandatory( true );
    pEmailEdit->lineEdit()->setValidator( pEmailValidator );
    tbl->elementAt( cl, 0 )->addWidget( new WLabel( WString::fromUTF8( "Электронная почта" ) ) );
    tbl->elementAt( cl++, 1 )->addWidget( pEmailEdit );

    WCustomInPlaceEdit* pOwnerPhoneEdit = new WCustomInPlaceEdit( WString::fromUTF8( pi.phone ), uv );
    std::string phone_match = "^[1-9]{1}[0-9]{7,14}$";
    WRegExpValidator* pPhoneValidator = new WRegExpValidator( phone_match );
    pPhoneValidator->setFlags( MatchCaseInsensitive );
    pPhoneValidator->setMandatory( true );
    pOwnerPhoneEdit->lineEdit()->setValidator( pPhoneValidator );
    tbl->elementAt( cl, 0 )->addWidget( new WLabel( WString::fromUTF8( "Контрольный телефон" ) ) );
    tbl->elementAt( cl++, 1 )->addWidget( pOwnerPhoneEdit );

    WCustomInPlaceEdit* pPhoneEdit = new WCustomInPlaceEdit( WString::fromUTF8( pi.phone ), uv );
    pPhoneEdit->lineEdit()->setValidator( pPhoneValidator );
    tbl->elementAt( cl, 0 )->addWidget( new WLabel( WString::fromUTF8( "Контактный телефон" ) ) );
    tbl->elementAt( cl++, 1 )->addWidget( pPhoneEdit );

    WCustomInPlaceEdit* pTimeZoneEdit = new WCustomInPlaceEdit( WString::fromUTF8( boost::lexical_cast< std::string >( pi.tzone ) ), uv );
    WIntValidator* pTimeZoneValidator = new WIntValidator( -12, 12 );
    pTimeZoneEdit->lineEdit()->setValidator( pTimeZoneValidator );
    tbl->elementAt( cl, 0 )->addWidget( new WLabel( WString::fromUTF8( "Часовой пояс ( UTC )" ) ) );
    tbl->elementAt( cl++, 1 )->addWidget( pTimeZoneEdit );

    pExpand = new WText( WString::fromUTF8( "⇑Скрыть личную инфомацию⇑" ) );
    pExpand->setStyleClass( "link" );
    pExpand->clicked().connect( this, &PartnerOptions::onPersonalShowHide );
    tbl->elementAt( cl, 0 )->setColumnSpan( 2 );
    tbl->elementAt( cl, 0 )->addWidget( pExpand );
    tbl->elementAt( cl, 0 )->setContentAlignment( AlignCenter | AlignMiddle );
    cl++;

    {
        Wt::WSuggestionPopup::Options suggestOptions
        = { "<b>",         // highlightBeginTag
            "</b>",        // highlightEndTag
            ',',           // listSeparator      (for multiple addresses)
            " \\n",        // whitespace
            " ",           // wordSeparators     (within an address)
            ""             // appendReplacedText (prepare next email address)
           };

        WCustomInPlaceEdit* pManagerEdit = new WCustomInPlaceEdit( WString::fromUTF8( pi.pManager ), uv );
        WSuggestionPopup* pManagerSuggest = new WSuggestionPopup( suggestOptions, this );
        std::list< PartnerInfo > lst = PartnerManager::get_mutable_instance().getAll();
        pManagerSuggest->forEdit( pManagerEdit->lineEdit(), WSuggestionPopup::Editing | WSuggestionPopup::DropDownIcon );
        pManagerSuggest->activated().connect( boost::bind( &PartnerOptions::onSuggestionActivated, this, pManagerSuggest, _1, pManagerEdit ) );
        std::set< std::string > lst_unique;
        for ( std::list< PartnerInfo >::iterator it = lst.begin(); it != lst.end(); it++ ) {
            lst_unique.insert( it->pManager );
        }
        for ( std::set< std::string >::iterator it = lst_unique.begin(); it != lst_unique.end(); it++ ) {
            pManagerSuggest->addSuggestion( WString::fromUTF8( *it ), WString::fromUTF8( *it ) );
        }
        tbl->elementAt( cl, 0 )->addWidget( new WLabel( WString::fromUTF8( "Менеджер" ) ) );
        tbl->elementAt( cl, 1 )->addWidget( pManagerSuggest );
        tbl->elementAt( cl++, 1 )->addWidget( pManagerEdit );
    }

    {
        Wt::WSuggestionPopup::Options suggestOptions
        = { "<b>",         // highlightBeginTag
            "</b>",        // highlightEndTag
            ',',           // listSeparator      (for multiple addresses)
            " \\n",        // whitespace
            " ",           // wordSeparators     (within an address)
            ""             // appendReplacedText (prepare next email address)
           };

        WCustomInPlaceEdit* pTariffEdit = new WCustomInPlaceEdit( WString::fromUTF8( pi.tariff.getName() ), uv );
        TariffManager::TariffListT tariffs = TariffManager::get_mutable_instance().tariffs_list();
        WSuggestionPopup* pTariffSuggest = new WSuggestionPopup( suggestOptions, this );
        pTariffSuggest->forEdit( pTariffEdit->lineEdit(), WSuggestionPopup::Editing | WSuggestionPopup::DropDownIcon );
        pTariffSuggest->activated().connect( boost::bind( &PartnerOptions::onSuggestionActivated, this, pTariffSuggest, _1, pTariffEdit ) );
        std::set< std::string > tariffs_unique( tariffs.begin(), tariffs.end() );
        for ( std::set< std::string >::iterator it = tariffs_unique.begin(); it != tariffs_unique.end(); it++ ) {
            pTariffSuggest->addSuggestion( WString::fromUTF8( *it ), WString::fromUTF8( *it ) );
        }
        tbl->elementAt( cl, 0 )->addWidget( new WLabel( WString::fromUTF8( "Тариф" ) ) );
        tbl->elementAt( cl, 1 )->addWidget( pTariffSuggest );
        tbl->elementAt( cl++, 1 )->addWidget( pTariffEdit );
    }

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

    if ( pid.empty() ) {
        isPersonalInfoVisible = true;
    } else
        isPersonalInfoVisible = false;
    onPersonalShowHide();

    WVBoxLayout* lyt = new WVBoxLayout();
    lyt->addWidget( tbl, AlignCenter | AlignMiddle );

    setLayout( lyt, AlignCenter | AlignMiddle );
}


void PartnerOptions::onSuggestionActivated( WSuggestionPopup* sugg, int index, WCustomInPlaceEdit* widget ) {
    WString data = boost::any_cast< WString >( sugg->model()->data( index, 1 ) );
    widget->setText( data );
}

void PartnerOptions::onPersonalShowHide() {
    if ( isPersonalInfoVisible ) {
        pExpand->setText( WString::fromUTF8( "⇑Скрыть личную инфомацию⇑" ) );
        for ( int i = 1; i < 8; i++ ) {
            tbl->rowAt( i )->show();
        }
    } else {
        pExpand->setText( WString::fromUTF8( "⇓Показать личную инфомацию⇓" ) );
        for ( int i = 1; i < 8; i++ ) {
            tbl->rowAt( i )->hide();
        }
    }
    isPersonalInfoVisible = !isPersonalInfoVisible;
}

PartnerEditor::PartnerEditor( Wt::WContainerWidget* parent ):WContainerWidget( parent ) {

    root = new WBorderLayout();
    opts = new WHBoxLayout();

    columns_width.push_back(150);
    columns_width.push_back(70);
    elements_per_page = 25;

    model_ = new WStandardItemModel();
    buildModel( model_ );
    treeView_ = buildTreeView( model_ );
    resizeTreeView( treeView_ );

    opts->addWidget( new PartnerOptions( "" ) );

    root->addWidget( treeView_, WBorderLayout::West );
    root->add( opts, WBorderLayout::Center );
    setLayout( root );
}

WTreeView* PartnerEditor::buildTreeView( Wt::WStandardItemModel * model ) {

    WTreeView* tw = new WTreeView();
    tw->setModel( model );
    tw->setSelectionMode( Wt::ExtendedSelection );
    tw->setAlternatingRowColors( true );
    tw->sortByColumn( 0, AscendingOrder);

    tw->clicked().connect( this, &PartnerEditor::onChangeRoot );

    return tw;
}

void PartnerEditor::resizeTreeView( WTreeView* tw) {
    int columns_total_width = 0;
    int scroll_width = 10 + columns_width.size()*10;
    for ( int i = 0; i < columns_width.size(); columns_total_width += columns_width[i++] ) {}


    tw->resize(
                WLength( scroll_width + columns_total_width, WLength::Pixel ),
                WLength( elements_per_page * tw->rowHeight().toPixels() + tw->headerHeight().toPixels(), WLength::Pixel )
                );

    for ( int i = 0; i < columns_width.size(); i++, columns_total_width++ ) {
        tw->setColumnWidth( i, WLength( columns_width[i], WLength::Pixel) );
    }
}

void PartnerEditor::buildModel( WStandardItemModel* data ) {
    data->clear();
    data->insertColumns(0, columns_width.size());

    data->setHeaderData(0, Horizontal, WString::fromUTF8("Партнер"));
    data->setHeaderData(1, Horizontal, WString::fromUTF8("PID"));

    std::list< PartnerInfo > lst = PartnerManager::get_mutable_instance().getAll();
    for ( std::list< PartnerInfo >::iterator it = lst.begin(); it != lst.end(); it++ ) {
        WStandardItem *pName,*pId;
        pName = new WStandardItem( WString::fromUTF8( it->pCName ) );
        pId = new WStandardItem( WString::fromUTF8( it->pId ) );

        std::vector< WStandardItem* > row;
        row.push_back( pName );
        row.push_back( pId );

        data->appendRow( row );
    }
}

void PartnerEditor::onChangeRoot() {
    Wt::WModelIndexSet selected = treeView_->selectedIndexes();


    while ( opts->count() ) {
        opts->removeItem( opts->itemAt( 0 ) );
    }

    if ( selected.empty() ) {
        opts->addWidget( new PartnerOptions( "" ) );
    }

    for ( Wt::WModelIndexSet::iterator it = selected.begin(); it != selected.end(); it++ ) {
        Wt::WModelIndex index = *it;

        WStandardItem* iroot = model_->itemFromIndex( index.parent() );
        WStandardItem* item = iroot->child( index.row(), 1 );

        std::string pId = item->text().toUTF8();

        opts->addWidget( new PartnerOptions( pId ) );
    }

}
