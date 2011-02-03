#include "TariffEditor.h"
#include "MessageClassifier.h"

#include <string>
#include <algorithm>

#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>
#include <Wt/WBorderLayout>
#include <Wt/WVBoxLayout>
#include <Wt/WContainerWidget>
#include <Wt/WLabel>

using namespace Wt;
using namespace std;

TariffEditor::TariffEditor( WContainerWidget* parent ): WContainerWidget( parent ) {
    columns_width.push_back(300);
    columns_width.push_back(100);
    columns_width.push_back(70);
    elements_per_page = 20;

    model_ = buildModel();
    treeView_ = buildTreeView( model_ );

//    WContainerWidget* uploadBlock = new WContainerWidget();
//    WVBoxLayout* uploadBlockLayout = new WVBoxLayout();

    WBorderLayout* root = new WBorderLayout();
    root->addWidget( treeView_, WBorderLayout::West );

    setLayout( root );
    resizeTreeView( treeView_ );
}

WStandardItemModel* TariffEditor::buildModel() {
    sms::MessageClassifier::CountryOperatorMapT comap = sms::MessageClassifier::get_mutable_instance().getCOMap_v2();

    WStandardItemModel* data = new WStandardItemModel( 0, columns_width.size() );
    data->setHeaderData(0, Horizontal, WString::fromUTF8("Страна/Оператор"));
    data->setHeaderData(1, Horizontal, WString::fromUTF8("MCC/MNC"));
    data->setHeaderData(2, Horizontal, WString::fromUTF8("Цена"));

    for( sms::MessageClassifier::CountryOperatorMapT::iterator it = comap.begin(); it != comap.end(); it++ ) {
        sms::MessageClassifier::CountryInfo cinfo = it->second;
        std::vector< WStandardItem* > row;

        WStandardItem *country = new WStandardItem( string( "resources/flags/" ) + cinfo.cCode + ".png", WString::fromUTF8( cinfo.cName ) );
        row.push_back( country );

        WStandardItem* mcc = new WStandardItem();
        mcc->setText( WString::fromUTF8( boost::lexical_cast< string >( cinfo.mcc ) ) );
        row.push_back( mcc );

        WStandardItem* price = new WStandardItem( WString::fromUTF8( "0.00" ) );

        row.push_back( price );

        for ( sms::MessageClassifier::CountryInfo::OperatorMapT::iterator gt = cinfo.operators.begin(); gt != cinfo.operators.end(); gt++ ) {
            sms::MessageClassifier::OperatorInfo info = gt->second;
            std::vector< WStandardItem* > subrow;

            WStandardItem* op = new WStandardItem();
            op->setText( WString::fromUTF8( info.opName == "" ? info.opCompany : info.opName ) );
            subrow.push_back( op );

            WStandardItem* code = new WStandardItem();
            code->setText( WString::fromUTF8( boost::lexical_cast< string >( info.mcc ) + string(":") + boost::lexical_cast< string >( info.mnc ) ) );
            subrow.push_back( code );

            WStandardItem* subprice = new WStandardItem( WString::fromUTF8( "0.00" ) );
            subrow.push_back( subprice );

            country->appendRow( subrow );
        }

        data->appendRow( row );
    }



    return data;
}

WTreeView* TariffEditor::buildTreeView( Wt::WStandardItemModel * model ) {

    WTreeView* tw = new WTreeView();
    tw->setModel( model );
    tw->setSelectionMode( Wt::SingleSelection );
    tw->setAlternatingRowColors( true );
    tw->sortByColumn( 0, AscendingOrder );
//    tw->

    return tw;
}

void TariffEditor::resizeTreeView( WTreeView* tw) {
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
