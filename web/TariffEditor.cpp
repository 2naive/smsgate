#include "TariffEditor.h"
#include "MessageClassifier.h"
#include "utils.h"

#include <string>
#include <algorithm>
#include <boost/bind.hpp>
#include <iostream>
#include <fstream>



using namespace Wt;
using namespace std;

TariffEditor::TariffEditor( WContainerWidget* parent ): WContainerWidget( parent ) {
    columns_width.push_back(300);
    columns_width.push_back(100);
    columns_width.push_back(70);
    elements_per_page = 20;

    model_ = new WStandardItemModel();
    buildModel( model_ );
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

void TariffEditor::buildModel( WStandardItemModel* data, Tariff* tariff ) {
    sms::MessageClassifier::CountryOperatorMapT comap = sms::MessageClassifier::get_mutable_instance().getCOMap_v2();

    data->clear();
    data->insertColumns(0, 3);

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

        string price_text = "Не задано";
        if ( tariff )
        try {
            price_text = double2string( tariff->costs( boost::lexical_cast< string >( mcc ) ) );
        } catch ( ... ) {}

        WStandardItem* price = new WStandardItem( WString::fromUTF8( price_text ) );;

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

            string price_text = "Не задано";
            if ( tariff )
            try {
                price_text = double2string( tariff->costs( boost::lexical_cast< string >( info.mcc ), boost::lexical_cast< string >( info.mnc ) ) );
            } catch ( ... ) {
                continue;
            }

            WStandardItem* subprice = new WStandardItem( WString::fromUTF8( price_text ) );

            subrow.push_back( subprice );

            country->appendRow( subrow );
        }

        data->appendRow( row );
    }
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
    importCtx.importDlg = &import;

    importCtx.fake = new WContainerWidget( importCtx.importDlg->contents() );
    importCtx.fake->hide();

    importCtx.upload = new WFileUpload( );
    importCtx.upload->uploaded().connect( boost::bind( &TariffEditor::importFileDone, this ) );
    importCtx.upload->fileTooLarge().connect( boost::bind( &TariffEditor::importFileTooLargeError, this ) );

    importCtx.uploadBtn = new WPushButton( WString::fromUTF8( "Загрузить" ) );
    importCtx.uploadBtn->clicked().connect( boost::bind( &TariffEditor::importUploadRequest, this ) );

    importCtx.cancelBtn = new WPushButton( WString::fromUTF8( "Отмена" ) );
    importCtx.cancelBtn->clicked().connect( &import, &WDialog::reject );

    importCtx.root = new WContainerWidget();

    importCtx.spacer = new WTable();

    importCtx.spacer->elementAt( 0, 0 )->addWidget( importCtx.upload );
    importCtx.spacer->elementAt( 0, 1 )->addWidget( importCtx.uploadBtn );

    importCtx.spacer->elementAt( 1, 0 )->addWidget( importCtx.cancelBtn );

    importCtx.root->addWidget( importCtx.spacer );

    import.contents()->addWidget( importCtx.root );

    import.exec();
}

void TariffEditor::importFileTooLargeError() {
    importCtx.importDlg->setCaption( WString::fromUTF8( "Испорт из CSV: ошибка ( слишком большой файл )" ) );
}

void TariffEditor::importFileDone() {

    if ( importCtx.upload->empty() ) {
        importCtx.importDlg->setCaption( WString::fromUTF8( "Испорт из CSV: ошибка ( выберите файл )" ) );
        return;
    }

    importCtx.fake->addWidget( importCtx.upload );
    importCtx.fake->addWidget( importCtx.cancelBtn );

    importCtx.spacer->deleteRow( 0 );
    importCtx.spacer->deleteRow( 0 );

    importCtx.netcode_helper = new WLabel( WString::fromUTF8( "Номер столбца с MCC/MNC" ) );
    importCtx.price_helper = new WLabel( WString::fromUTF8( "Номер столбца с ценой" ) );
    importCtx.fieldsep_hepler = new WLabel( WString::fromUTF8( "Разделитель полей" ) );
    importCtx.textsep_helper = new WLabel( WString::fromUTF8( "Ограничитель строк" ) );

    importCtx.netcode = new WSpinBox();
    importCtx.netcode->setValue( 1 );
    importCtx.netcode->setMinimum( 1 );
    importCtx.netcode->setSingleStep( 1 );
    importCtx.netcode->setValidator( new WIntValidator( 1, 1000 ) );

    importCtx.price = new WSpinBox();
    importCtx.price->setValue( 1 );
    importCtx.price->setMinimum( 1 );
    importCtx.price->setSingleStep( 1 );
    importCtx.price->setValidator( new WIntValidator( 1, 1000 ) );

    importCtx.fieldsep = new WLineEdit( ";" );
    importCtx.fieldsep->setMaxLength( 1 );

    importCtx.textsep = new WLineEdit( "\"" );
    importCtx.textsep->setMaxLength( 1 );

    WPushButton* nextBtn = new WPushButton( WString::fromUTF8( "Далее" ) );
    nextBtn->clicked().connect( this, &TariffEditor::importParseCsv );

    importCtx.spacer->elementAt( 0, 0 )->addWidget( importCtx.netcode_helper );
    importCtx.spacer->elementAt( 0, 1 )->addWidget( importCtx.netcode );

    importCtx.spacer->elementAt( 1, 0 )->addWidget( importCtx.price_helper );
    importCtx.spacer->elementAt( 1, 1 )->addWidget( importCtx.price );

    importCtx.spacer->elementAt( 2, 0 )->addWidget( importCtx.fieldsep_hepler );
    importCtx.spacer->elementAt( 2, 1 )->addWidget( importCtx.fieldsep );

    importCtx.spacer->elementAt( 3, 0 )->addWidget( importCtx.textsep_helper );
    importCtx.spacer->elementAt( 3, 1 )->addWidget( importCtx.textsep );

    importCtx.spacer->elementAt( 4, 0 )->addWidget( importCtx.cancelBtn );
    importCtx.spacer->elementAt( 4, 1 )->addWidget( nextBtn );
}

void TariffEditor::importUploadRequest() {
    if ( importCtx.upload->canUpload() ) {
        importCtx.upload->upload();
    } else {
        importCtx.importDlg->setCaption( WString::fromUTF8( "Импорт из CSV: ошибка ( невозможно отправить )" ) );
    }
}

void TariffEditor::importParseCsv() {
    ifstream in( importCtx.upload->spoolFileName().c_str() );

    importCtx.fake->addWidget( importCtx.cancelBtn );

    int col_mccmnc = importCtx.netcode->value();
    int col_price = importCtx.price->value();

    importCtx.spacer->deleteRow( 0 );
    importCtx.spacer->deleteRow( 0 );
    importCtx.spacer->deleteRow( 0 );
    importCtx.spacer->deleteRow( 0 );
    importCtx.spacer->deleteRow( 0 );

    char sep = importCtx.fieldsep->text().toUTF8().c_str()[0];
    char quotes = importCtx.textsep->text().toUTF8().c_str()[0];

    WTextArea* output = new WTextArea();
    output->setMinimumSize( WLength( 10, WLength::Centimeter ), WLength( 15, WLength::Centimeter ) );

    WPushButton* finishBtn = new WPushButton( WString::fromUTF8( "Готово" ) );
    finishBtn->clicked().connect( this, &TariffEditor::importCsvFinish );

    importCtx.spacer->elementAt( 0, 0 )->addWidget( output );
    importCtx.spacer->elementAt( 0, 0 )->setColumnSpan( 2 );
    importCtx.spacer->elementAt( 1, 0 )->addWidget( importCtx.cancelBtn );
    importCtx.spacer->elementAt( 1, 1 )->addWidget( finishBtn );

    importCtx.tariff = new Tariff( Tariff::buildEmpty( "TEST" ) );

    while ( !in.eof() && !in.fail() ) {
        int blocks_found = 0;
        bool parsing_text = false;
        vector< string > row;
        row.push_back("");
        string line;
        getline( in, line );

        for ( int i = 0; i < line.size(); i++ ) {
            char ch = line[i];
            if ( ( ch == sep ) && !parsing_text ) {
                blocks_found++;
                row.push_back("");
                continue;
            }
            if ( ch == quotes ) {
                parsing_text != parsing_text;
                continue;
            }
            row[ blocks_found ] += ch;
        }

        if ( row.size() < std::max( col_mccmnc, col_price ) ) {
            continue;
        }

        if ( row[ col_mccmnc ].size() < 3 ) {
            continue;
        }

        int mcc;
        int mnc;
        try {
            mcc = boost::lexical_cast< int >( row[ col_mccmnc ].substr( 0, 3 ) );
            mnc = sdouble2double( row[ col_mccmnc ].substr( 3, row[ col_mccmnc ].length()-3 ), -1 );
        } catch ( ... ) {
            continue;
        }

        double price;
        try {
            price = boost::lexical_cast< int >( row[ col_price ] );
        } catch ( ... ) {
            continue;
        }

        if ( mcc == -1 ) {
            importCtx.tariff->addFilterCountry( boost::lexical_cast< string >( mcc ), price );
        } else {
            importCtx.tariff->addFilterCountryOperator( boost::lexical_cast< string >( mcc ), boost::lexical_cast< string >( mnc ), price );
        }

        output->setText( output->text() + boost::lexical_cast< string >( mcc ) + string("\t") );
        if ( mnc != -1 ) {
            output->setText( output->text() + boost::lexical_cast< string >( mnc ) + string("\t") );
        } else {
            output->setText( output->text() + string("\t") );
        }
        output->setText( output->text() + double2string( price ) + string("\t") );
        output->setText( output->text() + string("\n") );
    }
}

void TariffEditor::importCsvFinish() {
    model_->clear();
    buildModel( model_, importCtx.tariff );

    importCtx.importDlg->accept();
}
