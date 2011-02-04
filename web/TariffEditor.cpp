#include "TariffEditor.h"
#include "MessageClassifier.h"
#include "utils.h"

#include <string>
#include <algorithm>
#include <boost/bind.hpp>
#include <iostream>
#include <fstream>

#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>
#include <Wt/WBorderLayout>
#include <Wt/WVBoxLayout>
#include <Wt/WGridLayout>
#include <Wt/WContainerWidget>
#include <Wt/WLabel>
#include <Wt/WDialog>
#include <Wt/WTable>
#include <Wt/WPushButton>
#include <Wt/WSpinBox>
#include <Wt/WDoubleValidator>
#include <Wt/WIntValidator>
#include <Wt/WApplication>
#include <Wt/WFileUpload>
#include <Wt/WProgressBar>

using namespace Wt;
using namespace std;

TariffEditor::TariffEditor( WContainerWidget* parent ): WContainerWidget( parent ) {
    columns_width.push_back(300);
    columns_width.push_back(100);
    columns_width.push_back(70);
    elements_per_page = 20;

    model_ = buildModel();
    treeView_ = buildTreeView( model_ );

    exportBtn = new WPushButton( WString::fromUTF8( "Export to Excel" ) );
    exportBtn->clicked().connect( this, &TariffEditor::exportToCsv );

    importBtn = new WPushButton( WString::fromUTF8( "Import from CSV" ) );
    importBtn->clicked().connect( this, &TariffEditor::importFromCsv);

    WBorderLayout* root = new WBorderLayout();
    root->addWidget( treeView_, WBorderLayout::West );
    root->addWidget( exportBtn, WBorderLayout::South );
    root->addWidget( importBtn, WBorderLayout::North );

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

        WStandardItem* price = new WStandardItem( WString::fromUTF8( "Не задано" ) );

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

            WStandardItem* subprice = new WStandardItem( WString::fromUTF8( "Не задано" ) );
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
    tw->setSelectionMode( Wt::ExtendedSelection );
    tw->setAlternatingRowColors( true );
    tw->sortByColumn( 0, AscendingOrder );

    tw->clicked().connect( this, &TariffEditor::onPriceEdit );

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

void TariffEditor::onPriceEdit( Wt::WModelIndex index, Wt::WMouseEvent event ) {

    if ( index.column() != 2 ) {
        return;
    }

    WDialog summary;
    summary.setWindowTitle( WString::fromUTF8("Редактор цены") );
    summary.setTitleBarEnabled( true );
    summary.setPositionScheme( Wt::Absolute );
    summary.setOffsets( WLength( event.document().x, WLength::Pixel ), Wt::Left );
    summary.setOffsets( WLength( event.document().y, WLength::Pixel ), Wt::Top );

    WStandardItem* root = model_->itemFromIndex( index.parent() );
    WStandardItem* item = root->child( index.row(), index.column() );
    WModelIndex capital = model_->indexFromItem( root->child( index.row(), 0 ) );

    WString old_price = item->text();

    WLabel* oldp = new WLabel( old_price );
    WSpinBox* newp = new WSpinBox( );
    newp->setMinimum( 0 );
    newp->setMaxLength(5);
    newp->setMaximumSize( WLength( 1.5, WLength::Centimeter ), WLength::Auto );
    newp->setSingleStep( 0.01 );
    newp->setText( "0.00" );
    newp->setValidator( new WDoubleValidator( 0, 100 ) );
    newp->enterPressed().connect( &summary, &WDialog::accept );
    newp->enterPressed().connect( boost::bind( &TariffEditor::changeItemText, this, index, newp ) );

    WTable report( summary.contents() );
    report.setStyleClass("restable");
    report.elementAt(0, 0)->addWidget( new WLabel( WString::fromUTF8("Старая цена") ) );
    report.elementAt(1, 0)->addWidget( new WLabel( WString::fromUTF8("Новая цена") ) );

    report.elementAt(0, 1)->addWidget( oldp );
    report.elementAt(1, 1)->addWidget( newp );

    if ( root == model_->invisibleRootItem() ) {
        WPushButton* okAll = new WPushButton( WString::fromUTF8("Изменить все"), summary.contents() );
        okAll->clicked().connect(&summary, &WDialog::accept);
        okAll->clicked().connect( boost::bind( &TariffEditor::changeItemTextRecursive, this, capital, 2, newp ) );
    }

    WPushButton* okBtn = new WPushButton( WString::fromUTF8("Изменить"), summary.contents() );
    okBtn->clicked().connect(&summary, &WDialog::accept);
    okBtn->clicked().connect( boost::bind( &TariffEditor::changeItemText, this, index, newp ) );

    WPushButton* cancelBtn = new WPushButton( WString::fromUTF8("Отмена"), summary.contents() );
    cancelBtn->clicked().connect(&summary, &WDialog::reject);

    newp->setFocus();
    summary.exec();
}

void TariffEditor::changeItemText( Wt::WModelIndex index, WSpinBox* text ) {
    WStandardItem* root = model_->itemFromIndex( index.parent() );
    WStandardItem* item = root->takeChild( index.row(), index.column() );

    item->setText( text->text() );
    root->setChild( index.row(), index.column(), item );

}

void TariffEditor::changeItemTextRecursive( Wt::WModelIndex index, int column, WSpinBox* text ) {
    WStandardItem* root = model_->itemFromIndex( index.parent() );
    WStandardItem* item = root->child( index.row(), index.column() );

    changeItemText( model_->indexFromItem( root->child( index.row(), column ) ), text );

    for ( int i = 0; i < item->rowCount(); i++ ) {
        changeItemText( index.child( i, column ), text );
    }
}

void TariffEditor::exportToCsv() {
    Wt::WFileResource* csv = new WFileResource( this );

    csv->setFileName( "/tmp/123.csv" );
    csv->setMimeType( "text/txt" );
    csv->suggestFileName( "report.csv" );

    sms::MessageClassifier::CountryOperatorMapT comap = sms::MessageClassifier::get_mutable_instance().getCOMap_v2();
    ofstream fout( "/tmp/123.csv" );

    WStandardItem* item = model_->invisibleRootItem();

    recursivePrintCsv( fout, comap, item );

    fout.close();

    WApplication::instance()->redirect( csv->generateUrl() );
}

void TariffEditor::recursivePrintCsv( std::ostream& out, sms::MessageClassifier::CountryOperatorMapT& map, Wt::WStandardItem* item ) {
    for ( int i = 0; i < item->rowCount(); i++ ) {
        string mccmnc = item->child( i, 1 )->text().toUTF8();
        vector< string > to_vec;
        double mcc;
        double mnc;
        string price;
        sms::utils::Tokenize( mccmnc, to_vec, ":" );
        bool isCountry = (to_vec.size() == 1);

        mcc = sdouble2double( to_vec[0], -1 );
        if ( mcc == -1 ) continue;

        price = sdouble2string( item->child( i, 2 )->text().toUTF8() );

        if ( isCountry ) {
            sms::MessageClassifier::CountryInfo ci = map[ mcc ];
            out << "\"" << ci.cName << "\"" << ";";     // Country name
            out << ";";                                 // Network name
            out << mcc << ";";                          // MCC
            out << ";";                                 // MNC
            out << price << ";";                        // Price
            out << endl;

            if ( !item->child( i, 0 )->hasChildren() )
                continue;

            recursivePrintCsv( out, map, item->child( i, 0 ) );
            continue;
        }

        // Is Network
        mnc = sdouble2double( to_vec[1], -1 );

        sms::MessageClassifier::CountryInfo ci = map[ mcc ];
        sms::MessageClassifier::OperatorInfo oi = map[ mcc ].operators[ mnc ];
        out << "\"" << ci.cName << "\"" << ";";     // Country name
        out << "\"" << oi.getName() << "\"" << ";"; // Network name
        out << mcc << ";";                          // MCC
        out << mnc << ";";                          // MNC
        out << price << ";";                        // Price
        out << endl;

    }
}

string TariffEditor::sdouble2string( string v, string defval ) {
    double val;
    try {
        val = boost::lexical_cast< double >( v );
    } catch ( ... ) {
        return defval;
    }
    return double2string( val );
}

double TariffEditor::sdouble2double( string v, double defval ) {
    double val;
    try {
        val = boost::lexical_cast< double >( v );
    } catch ( ... ) {
        val =  defval;
    }
    return val;
}

string TariffEditor::double2string( double v ) {
    char buf[100];
    sprintf( buf, "%0.4f", v );

    return buf;
}

void TariffEditor::importFromCsv() {
    WDialog import( WString::fromUTF8( "Испорт из CSV" ) );

    WLabel* netcode_helper = new WLabel( WString::fromUTF8( "Номер столбца с MCC/MNC" ) );
    WLabel* price_helper = new WLabel( WString::fromUTF8( "Номер столбца с ценой" ) );

    WSpinBox* netcode = new WSpinBox();
    netcode->setMinimum( 1 );
    netcode->setSingleStep( 1 );
    netcode->setValidator( new WIntValidator( 1, 1000 ) );

    WSpinBox* price = new WSpinBox();
    price->setMinimum( 1 );
    price->setSingleStep( 1 );
    price->setValidator( new WIntValidator() );

    WProgressBar* pbar = new WProgressBar();

    WFileUpload* upload = new WFileUpload();
    upload->uploaded().connect( &import, &WDialog::accept );
    upload->fileTooLarge().connect(boost::bind( &WDialog::reject, &import) );
    upload->setProgressBar( pbar );

    WPushButton* uploadBtn = new WPushButton( WString::fromUTF8( "Загрузить" ) );
    uploadBtn->clicked().connect( upload, &WFileUpload::upload );

    WContainerWidget* root = new WContainerWidget();

    WGridLayout* spacer = new WGridLayout();

    spacer->addWidget( netcode_helper, 0, 0, 0, 0, Wt::AlignCenter );
    spacer->addWidget( netcode, 0, 1, 0, 0, Wt::AlignCenter );

    spacer->addWidget( price_helper, 1, 0, 0, 0, Wt::AlignCenter );
    spacer->addWidget( price, 1, 1, 0, 0, Wt::AlignCenter );

    spacer->addWidget( upload, 2, 0, 0, 0, Wt::AlignCenter );
    spacer->addWidget( uploadBtn, 2, 1, 0, 0, Wt::AlignCenter );

    root->setLayout( spacer );
    import.contents()->addWidget( root );

    import.exec();
}
