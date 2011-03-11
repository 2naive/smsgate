#ifndef TARIFFEDITOR_H
#define TARIFFEDITOR_H

#include "MessageClassifier.h"
#include "Tariff.h"

#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>
#include <Wt/WBorderLayout>
#include <Wt/WVBoxLayout>
#include <Wt/WHBoxLayout>
#include <Wt/WGridLayout>
#include <Wt/WContainerWidget>
#include <Wt/WComboBox>
#include <Wt/WCheckBox>
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

#include <Wt/WGroupBox>

#include <vector>
#include <string>
#include <ostream>

template < class Option >
class TariffOptionMultiEditor: public Wt::WContainerWidget {
public:
    enum POSITION_TYPE {
        POS_ROOT,
        POS_COUNTRY,
        POS_OPERATOR
    };

    TariffOptionMultiEditor( Tariff* _tariff, WContainerWidget* parent = 0 ): WContainerWidget( parent ), tariff( _tariff ) {
        Option option;

        optionState = new Wt::WCheckBox( Wt::WString::fromUTF8( Option::getName() ) );
        optionState->setTristate();
        optionState->changed().connect( boost::bind( &TariffOptionMultiEditor< Option >::optionChanged, this ) );

        opt_box_grp = new Wt::WContainerWidget();
        Wt::WVBoxLayout* opt_box_layout = new Wt::WVBoxLayout();

        typename Option::DescriptionList description = option.getDescriptions();
        for ( typename Option::DescriptionList::iterator it = description.begin(); it != description.end(); it++ ) {
            Wt::WCheckBox* opt_box = new Wt::WCheckBox( Wt::WString::fromUTF8( *it ) );
            opt_box->changed().connect( boost::bind( &TariffOptionMultiEditor< Option >::valueChanged, this, *it ) );

            opt_box_layout->addWidget( opt_box );
            checkboxes.insert( std::make_pair( *it, opt_box ) );
        }

        opt_box_grp->setLayout( opt_box_layout );
        opt_box_grp->hide();

        Wt::WVBoxLayout *root_layout = new Wt::WVBoxLayout();
        root_layout->addWidget( optionState );
        root_layout->addWidget( opt_box_grp );

        setLayout( root_layout );
    }

    void setCurPosRoot( ) {
        last_post_type = POS_ROOT;
        last_mcc = "";
        last_mnc = "";

        repaintOption();
    }

    void setCurPosCountry( std::string mcc ) {
        last_post_type = POS_COUNTRY;
        last_mcc = mcc;
        last_mnc = "";

        repaintOption();
    }

    void setCurPosOperator( std::string mcc, std::string mnc ) {
        last_post_type = POS_OPERATOR;
        last_mcc = mcc;
        last_mnc = mnc;

        repaintOption();
    }

    void repaintOption() {
        boost::logic::tribool opt_exists;
        Option option;

        switch ( last_post_type ) {
        case POS_ROOT:
            opt_exists = tariff->hasOption< Option >();
            option = tariff->getOption< Option >();
            break;
        case POS_COUNTRY:
            opt_exists = tariff->hasOption< Option >( last_mcc );
            option = tariff->getOption< Option >( last_mcc );
            break;
        case POS_OPERATOR:
            opt_exists = tariff->hasOption< Option >( last_mcc, last_mnc );
            option = tariff->getOption< Option >( last_mcc, last_mnc );
            break;
        }

        if ( opt_exists ) {
            optionState->setCheckState( Wt::Checked );
            opt_box_grp->show();
            opt_box_grp->setDisabled( false );

            for ( std::map< std::string, Wt::WCheckBox* >::iterator it = checkboxes.begin(); it != checkboxes.end(); it++) {
                it->second->setCheckState( Wt::Unchecked );
            }

            typename Option::ValuesListT values = option.getValues();
            for ( typename Option::ValuesListT::iterator it = values.begin(); it != values.end(); it++) {
                checkboxes[*it]->setCheckState( Wt::Checked );
            }
        }
        else if ( !opt_exists ) {
            optionState->setCheckState( Wt::Unchecked );
            opt_box_grp->hide();
            opt_box_grp->setDisabled( true );
        }
        else {
            optionState->setCheckState( Wt::PartiallyChecked );
            opt_box_grp->show();
            opt_box_grp->setDisabled( true );

            for ( std::map< std::string, Wt::WCheckBox* >::iterator it = checkboxes.begin(); it != checkboxes.end(); it++) {
                it->second->setCheckState( Wt::Unchecked );
            }

            typename Option::ValuesListT values = option.getValues();
            for ( typename Option::ValuesListT::iterator it = values.begin(); it != values.end(); it++) {
                checkboxes[*it]->setCheckState( Wt::Checked );
            }
        }
    }

    void valueChanged( std::string name ) {
        Wt::WApplication::instance()->processEvents();
        Wt::WCheckBox* chk_box = checkboxes[ name ];
        Option option;
        switch ( last_post_type ) {
        case POS_ROOT:
            option = tariff->getOption< Option >();

            if ( chk_box->isChecked() ) {
                typename Option::ValuesListT values;
                values = option.getValues();
                values.insert( name );
                option.setValues( values );
            }
            else {
                typename Option::ValuesListT values;
                values = option.getValues();
                values.erase( name );
                option.setValues( values );
            }

            tariff->setOption<Option>( option );
            break;
        case POS_COUNTRY:
            option = tariff->getOption< Option >( last_mcc );

            if ( chk_box->isChecked() ) {
                typename Option::ValuesListT values;
                values = option.getValues();
                values.insert( name );
                option.setValues( values );
            }
            else {
                typename Option::ValuesListT values;
                values = option.getValues();
                values.erase( name );
                option.setValues( values );
            }

            tariff->setOption<Option>( option, last_mcc );
            break;
        case POS_OPERATOR:
            option = tariff->getOption< Option >( last_mcc, last_mnc );

            if ( chk_box->isChecked() ) {
                typename Option::ValuesListT values;
                values = option.getValues();
                values.insert( name );
                option.setValues( values );
            }
            else {
                typename Option::ValuesListT values;
                values = option.getValues();
                values.erase( name );
                option.setValues( values );
            }

            tariff->setOption<Option>( option, last_mcc, last_mnc );
            break;
        }
        repaintOption();
    }

    void optionChanged() {
        if ( optionState->checkState() == Wt::PartiallyChecked )
            optionState->setChecked();
        Wt::WApplication::instance()->processEvents();

        if ( optionState->checkState() == Wt::Unchecked ) {
            boost::logic::tribool opt_exists;

            switch ( last_post_type ) {
            case POS_ROOT:
                opt_exists = tariff->hasOption< Option >();
                break;
            case POS_COUNTRY:
                opt_exists = tariff->hasOption< Option >( last_mcc );
                break;
            case POS_OPERATOR:
                opt_exists = tariff->hasOption< Option >( last_mcc, last_mnc );
                break;
            }

            if ( opt_exists ) {
                switch ( last_post_type ) {
                case POS_ROOT:
                    tariff->removeOption< Option >();
                    break;
                case POS_COUNTRY:
                    tariff->removeOption< Option >( last_mcc );
                    break;
                case POS_OPERATOR:
                    tariff->removeOption< Option >( last_mcc, last_mnc );
                    break;
                }
            }
        }

        if ( optionState->checkState() == Wt::Checked ) {
            boost::logic::tribool opt_exists;

            switch ( last_post_type ) {
            case POS_ROOT:
                opt_exists = tariff->hasOption< Option >();
                break;
            case POS_COUNTRY:
                opt_exists = tariff->hasOption< Option >( last_mcc );
                break;
            case POS_OPERATOR:
                opt_exists = tariff->hasOption< Option >( last_mcc, last_mnc );
                break;
            }

            if ( !opt_exists ) {
                switch ( last_post_type ) {
                case POS_ROOT:
                    tariff->setOption< Option >( Option() );
                    break;
                case POS_COUNTRY:
                    tariff->setOption< Option >( Option(), last_mcc );
                    break;
                case POS_OPERATOR:
                    tariff->setOption< Option >( Option(), last_mcc, last_mnc );
                    break;
                }
            }

        }

        repaintOption();
    }

    void eraseAll() {
        for ( std::map< std::string, Wt::WCheckBox* >::iterator it = checkboxes.begin(); it != checkboxes.end(); it++ ) {
            this->removeWidget( it->second );
        }

        checkboxes.clear();
    }

private:
    Tariff* tariff;
    std::map< std::string, Wt::WCheckBox* > checkboxes;
    Wt::WCheckBox* optionState;
    Wt::WContainerWidget* opt_box_grp;

    POSITION_TYPE last_post_type;
    std::string last_mcc;
    std::string last_mnc;
};


class TariffEditor: public Wt::WContainerWidget {
public:
    TariffEditor( WContainerWidget* parent = 0 );
private:
    Wt::WStandardItemModel* model_;
    Wt::WTreeView* treeView_;
    Wt::WPushButton* exportBtn;
    Wt::WPushButton* importBtn;
    Wt::WComboBox* tlistBox;
    Wt::WLineEdit* nameBox;
    TariffOptionMultiEditor< Tariff::TariffOptionPaidStatuses >* paidStatuses;

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
    } importCtx;

    std::vector< int > columns_width;
    int elements_per_page;
    Tariff tariff;

    void buildModel( Wt::WStandardItemModel*, Tariff& tariff );
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

    void onTariffLoad();
    void onTariffRemove();
    void onTariffSave();
    void onTariffClear();
    void onTariffUpdate();
    void tlistRebuild();

    void onChangeRoot();

    std::string sdouble2string( std::string v, std::string def_val = "" );
    double sdouble2double( std::string v, double defval );
    std::string double2string( double v );
};

#endif // TARIFFEDITOR_H
