#ifndef WSCROLLTABLE_H
#define WSCROLLTABLE_H

#include <boost/shared_ptr.hpp>

#include <Wt/WTable>
#include "DataSource.h"

class WScrollTable: public Wt::WTable {
public:
    typedef std::vector< boost::shared_ptr< Wt::WWidget > > RowType ;
    typedef WDataSource< RowType > Storage;
    WScrollTable( Storage& header, Storage& footer, Storage& data, Wt::WContainerWidget *parent=0 );

private:
    Storage& header;
    Storage& footer;
    Storage& data;

    void buildHeader();
    void buildFooter();
    void buildData();
};

#endif // WSCROLLTABLE_H
