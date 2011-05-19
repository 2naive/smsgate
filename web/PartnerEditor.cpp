#include "PartnerEditor.h"
#include "PartnerManager.h"

#include <Wt/WHBoxLayout>
#include <Wt/WStandardItem>
#include <vector>

using namespace Wt;

PartnerOptions::PartnerOptions( std::string _pid, Wt::WContainerWidget *parent ): WContainerWidget( parent ) {
    pid = _pid;
    PartnerInfo pi;

    try {
        pi = PartnerManager::get_mutable_instance().findById( pid );
    } catch ( ... ) {}
    WString uv = WString::fromUTF8( "Значение не задано" );

    int cl = 0;

    WTable* tbl = new WTable();
    tbl->setStyleClass( "restable" );
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
    lyt->addWidget( tbl, AlignCenter | AlignMiddle );

    setLayout( lyt, AlignCenter | AlignMiddle );
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
