#ifndef TARIFFEDITOR_H
#define TARIFFEDITOR_H

#include <Wt/WContainerWidget>
#include <Wt/WStandardItemModel>
#include <Wt/WTreeView>

#include <vector>
#include <string>

class TariffEditor: public Wt::WContainerWidget {
public:
    TariffEditor( WContainerWidget* parent = 0 );
private:
    Wt::WStandardItemModel* model_;
    Wt::WTreeView* treeView_;

    std::vector< int > columns_width;
    int elements_per_page;


    Wt::WStandardItemModel* buildModel();
    Wt::WTreeView* buildTreeView( Wt::WStandardItemModel* );
    void resizeTreeView( Wt::WTreeView* );
};

#endif // TARIFFEDITOR_H
