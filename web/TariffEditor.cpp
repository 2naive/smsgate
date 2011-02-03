#include "TariffEditor.h"
#include "MessageClassifier.h"

#include <string>
#include <algorithm>


#include <Wt/WTreeView>
#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>
#include <Wt/WBoxLayout>

using namespace Wt;
using namespace std;

TariffEditor::TariffEditor( WContainerWidget* parent ): WContainerWidget( parent ) {
    sms::MessageClassifier::CountryOperatorMapT comap = sms::MessageClassifier::get_mutable_instance().getCOMap_v2();

    WStandardItemModel* data = new WStandardItemModel( 0, 1 );
    data->setHeaderData(0, Horizontal, WString::fromUTF8("Страна/Оператор"));

    for( sms::MessageClassifier::CountryOperatorMapT::iterator it = comap.begin(); it != comap.end(); it++ ) {
        sms::CountryOperatorInfo info = it->second.begin()->second;

        transform( info.cCode.begin(), info.cCode.end(), info.cCode.begin(), ::tolower );
        WStandardItem *country = new WStandardItem( string( "resources/flags/" ) + info.cCode + ".png", WString::fromUTF8( info.cName ) );

        for ( sms::MessageClassifier::OperatorT::iterator gt = it->second.begin(); gt != it->second.end(); gt++ ) {
            info = gt->second;
            WStandardItem* op = new WStandardItem();
            op->setText( WString::fromUTF8( info.opName == "" ? info.opCompany : info.opName ) );
            country->appendRow( op );
        }

        data->appendRow( country );
    }


    WBoxLayout* box = new WBoxLayout( WBoxLayout::TopToBottom );
    WTreeView* tw = new WTreeView();
    tw->setModel( data );
    tw->setMinimumSize( WLength::Auto, WLength( 10, WLength::Centimeter ) );
    tw->setSelectionMode( Wt::SingleSelection );
    tw->setAlternatingRowColors( true );
    tw->sortByColumn( 0, AscendingOrder );

    box->addWidget( tw );
    setLayout( box );
}

