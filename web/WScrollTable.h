#ifndef WSCROLLTABLE_H
#define WSCROLLTABLE_H

#include <boost/shared_ptr.hpp>

#include <Wt/WTable>
#include "DataSource.h"

class WScrollTable: public Wt::WTable {
public:
    typedef std::vector< Wt::WWidget* > RowType ;
    typedef WDataSource< RowType > Storage;
    WScrollTable( Storage& header, Storage& footer, Storage& data, Wt::WContainerWidget *parent=0 );

    void rebuildData();
private:
    Storage& header;
    Storage& footer;
    Storage& data;

    int headerlen;
    int footerlen;
    int datalen;

    void buildHeader();
    void buildFooter();
    void buildData();

};

#endif // WSCROLLTABLE_H
