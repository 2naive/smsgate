#ifndef PARTNEREDITOR_H
#define PARTNEREDITOR_H

#include <Wt/WContainerWidget>
#include <Wt/WTable>
#include <Wt/WLabel>
#include <Wt/WInPlaceEdit>
#include <Wt/WLineEdit>
#include <Wt/WCheckBox>

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

    Wt::WLabel* pName;
    Wt::WInPlaceEdit* pNameEditor;
};

class PartnerEditor: public Wt::WContainerWidget {
public:
    PartnerEditor( Wt::WContainerWidget* parent = 0 );

private:

};

#endif // PARTNEREDITOR_H
