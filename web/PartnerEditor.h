#ifndef PARTNEREDITOR_H
#define PARTNEREDITOR_H

#include <Wt/WContainerWidget>
#include <Wt/WTable>
#include <Wt/WLabel>
#include <Wt/WInPlaceEdit>
#include <Wt/WLineEdit>
#include <Wt/WCheckBox>
#include <Wt/WTreeView>
#include <Wt/WStandardItemModel>
#include <Wt/WBorderLayout>
#include <Wt/WHBoxLayout>


class WCustomInPlaceEdit: public Wt::WInPlaceEdit {
public:
    WCustomInPlaceEdit( Wt::WString ss, Wt::WString es, Wt::WContainerWidget* parent = 0 ): Wt::WInPlaceEdit( ss, parent ) {
        setEmptyText( es );
        setButtonsEnabled( false );
//        setMaximumSize( Wt::WLength( 80, Wt::WLength::Percentage ), Wt::WLength::Auto );
        if ( ss.toUTF8().empty() ) {
            setText( Wt::WString::Empty );
        }
    }
};

class PartnerOptions: public Wt::WContainerWidget {
public:
    PartnerOptions( std::string pid, Wt::WContainerWidget* parent = 0 );

private:
    std::string pid;
    bool isPersonalInfoVisible;

    Wt::WTable* tbl;
    Wt::WLabel* pName;
    Wt::WInPlaceEdit* pNameEditor;
    Wt::WText* pExpand;

    void onPersonalShowHide();
    void onSuggestionActivated( Wt::WSuggestionPopup* sugg, int index, WCustomInPlaceEdit* widget );
};

class PartnerEditor: public Wt::WContainerWidget {
public:
    PartnerEditor( Wt::WContainerWidget* parent = 0 );

private:
    Wt::WStandardItemModel* model_;
    Wt::WTreeView* treeView_;
    Wt::WBorderLayout* root;
    Wt::WHBoxLayout* opts;

    std::vector< int > columns_width;
    int elements_per_page;

    Wt::WTreeView* buildTreeView( Wt::WStandardItemModel * model );
    void resizeTreeView( Wt::WTreeView* tw);
    void buildModel( Wt::WStandardItemModel* data );
    void onChangeRoot();
};

#endif // PARTNEREDITOR_H
