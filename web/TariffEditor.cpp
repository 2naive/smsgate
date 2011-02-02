#include "TariffEditor.h"
#include "MessageClassifier.h"

#include <Wt/WTreeView>
#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>

using namespace Wt;

TariffEditor::TariffEditor( WContainerWidget* parent ): WContainerWidget( parent ) {
    //addWidget( new WLabel( "TariffEditor" ) );
    sms::MessageClassifier::CountryOperatorMapT comap = sms::MessageClassifier::get_mutable_instance().getCOMap_v2();

    WStandardItemModel* data = new WStandardItemModel( 0, 1 );

    data->setHeaderData(0, Horizontal, WString::fromUTF8("Страна/Оператор"));

    WStandardItem *root = data->invisibleRootItem();
    for( sms::MessageClassifier::CountryOperatorMapT::iterator it = comap.begin(); it != comap.end(); it++ ) {
        WStandardItem *country = new WStandardItem();
        country->setText( WString::fromUTF8( it->second.begin()->second.cName ) );

        for ( sms::MessageClassifier::OperatorT::iterator gt = it->second.begin(); gt != it->second.end(); gt++ ) {
            sms::CountryOperatorInfo info = gt->second;
            WStandardItem* op = new WStandardItem();
            op->setText( gt->second.opName );
            country->appendRow( op );
        }
        data->appendRow( country );
    }

    WTreeView* tw = new WTreeView();
    tw->setMaximumSize( WLength( 20, WLength::Percentage ), WLength::Auto );
    tw->setModel( data );
    tw->setSelectionMode( Wt::SingleSelection );

    tw->setRootIndex( data->index(0, 0) );

//    tw->setExpanded(data->index(0, 0), true);
//    tw->setExpanded(data->index(0, 0, data->index(0, 0)), true);


    addWidget( tw );
}
