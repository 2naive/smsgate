#ifndef TARIFFEDITOR_H
#define TARIFFEDITOR_H

#include "MessageClassifier.h"
#include "Tariff.h"

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
#include <Wt/WLineEdit>
#include <Wt/WSpinBox>
#include <Wt/WDoubleValidator>
#include <Wt/WIntValidator>
#include <Wt/WApplication>
#include <Wt/WFileUpload>
#include <Wt/WProgressBar>
#include <Wt/WTreeView>
#include <Wt/WFileResource>
#include <Wt/WTextArea>

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

    struct import_controls {
        Wt::WLabel* netcode_helper;
        Wt::WLabel* price_helper;
        Wt::WLabel* fieldsep_hepler;
        Wt::WLabel* textsep_helper;
        Wt::WSpinBox* netcode;
        Wt::WSpinBox* price;
        Wt::WLineEdit* fieldsep;
        Wt::WLineEdit* textsep;
        Wt::WFileUpload* upload;
        Wt::WPushButton* uploadBtn;
        Wt::WTable* spacer;
        Wt::WTable* preview;
        Wt::WDialog* importDlg;
        WContainerWidget* root;
        WContainerWidget* fake;
        Wt::WPushButton* cancelBtn;
        Tariff* tariff;
    } importCtx;

    std::vector< int > columns_width;
    int elements_per_page;


    void buildModel( Wt::WStandardItemModel*, Tariff* tariff = NULL);
    Wt::WTreeView* buildTreeView( Wt::WStandardItemModel* );
    void resizeTreeView( Wt::WTreeView* );
    void onPriceEdit( Wt::WModelIndex, Wt::WMouseEvent );

    void changeItemText( Wt::WModelIndex, Wt::WSpinBox* );
    void changeItemTextRecursive( Wt::WModelIndex, int column, Wt::WSpinBox* );

    void exportToCsv();
    void recursivePrintCsv( std::ostream& out, sms::MessageClassifier::CountryOperatorMapT&, Wt::WStandardItem* item );

    void importFromCsv();
    void importFileTooLargeError();
    void importFileDone();
    void importUploadRequest();
    void importParseCsv();
    void importCsvFinish();

    std::string sdouble2string( std::string v, std::string def_val = "" );
    double sdouble2double( std::string v, double defval );
    std::string double2string( double v );
};

#endif // TARIFFEDITOR_H
