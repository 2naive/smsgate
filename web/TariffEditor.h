#ifndef TARIFFEDITOR_H
#define TARIFFEDITOR_H

#include <Wt/WContainerWidget>
#include <Wt/WStandardItemModel>
#include <Wt/WTreeView>
#include <Wt/WSpinBox>
#include <Wt/WFileResource>

#include <vector>
#include <string>

class TariffEditor: public Wt::WContainerWidget {
public:
    TariffEditor( WContainerWidget* parent = 0 );
private:
    Wt::WStandardItemModel* model_;
    Wt::WTreeView* treeView_;
    Wt::WLabel* csv_link;

    std::vector< int > columns_width;
    int elements_per_page;


    Wt::WStandardItemModel* buildModel();
    Wt::WTreeView* buildTreeView( Wt::WStandardItemModel* );
    void resizeTreeView( Wt::WTreeView* );
    void onPriceEdit( Wt::WModelIndex, Wt::WMouseEvent );

    void changeItemText( Wt::WModelIndex, Wt::WSpinBox* );
    void changeItemTextRecursive( Wt::WModelIndex, int column, Wt::WSpinBox* );

    void exportToCsv();
};

#endif // TARIFFEDITOR_H
