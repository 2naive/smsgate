#include "TariffEditor.h"

#include <Wt/WLabel>

using namespace Wt;

TariffEditor::TariffEditor( WContainerWidget* parent ): WContainerWidget( parent ) {
    addWidget( new WLabel( "TariffEditor" ) );
}
