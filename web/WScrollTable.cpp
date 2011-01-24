#include "WScrollTable.h"

WScrollTable::WScrollTable( Storage& _header, Storage& _footer, Storage& _data, Wt::WContainerWidget *parent ):
    Wt::WTable( parent ),
    header( _header ),
    footer( _footer ),
    data( _data )
{
    buildHeader();
    buildFooter();
    buildData();
}


