#ifndef TARIFFEDITOR_H
#define TARIFFEDITOR_H

#include "MessageClassifier.h"

#include <Wt/WContainerWidget>
#include <Wt/WStandardItemModel>
#include <Wt/WTreeView>
#include <Wt/WSpinBox>
#include <Wt/WFileResource>

#include <vector>
#include <string>
#include <ostream>

class TariffEditor: public Wt::WContainerWidget {
public:
    TariffEditor( WContainerWidget* parent = 0 );
private:
    Wt::WStandardItemModel* model_;
    Wt::WTreeView* treeView_;
    Wt::WPushButton* exportBtn;
    Wt::WPushButton* importBtn;

    std::vector< int > columns_width;
    int elements_per_page;


    Wt::WStandardItemModel* buildModel();
    Wt::WTreeView* buildTreeView( Wt::WStandardItemModel* );
    void resizeTreeView( Wt::WTreeView* );
    void onPriceEdit( Wt::WModelIndex, Wt::WMouseEvent );

    void changeItemText( Wt::WModelIndex, Wt::WSpinBox* );
    void changeItemTextRecursive( Wt::WModelIndex, int column, Wt::WSpinBox* );

    void exportToCsv();
    void recursivePrintCsv( std::ostream& out, sms::MessageClassifier::CountryOperatorMapT&, Wt::WStandardItem* item );

    void importFromCsv();

    std::string sdouble2string( std::string v, std::string def_val = "" );
    double sdouble2double( std::string v, double defval );
    std::string double2string( double v );
};

#endif // TARIFFEDITOR_H
