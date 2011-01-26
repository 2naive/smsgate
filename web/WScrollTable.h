#ifndef WSCROLLTABLE_H
#define WSCROLLTABLE_H

#include <boost/shared_ptr.hpp>

#include <Wt/WTable>
#include "DataSource.h"

class WScrollTable: public Wt::WTable {
public:
    typedef std::vector< Wt::WWidget* > RowType ;
    typedef WDataSource< RowType > Storage;
    WScrollTable( Storage& header, Storage& data, Storage& footer, Wt::WContainerWidget *parent=0 );

    void rebuildData( int offset = 0 );
    void setPageLimit( int limit );

    void buildHeader();
    void buildFooter();
    void buildData( int offset = 0 );
    void nextPage() { rebuildData( skipped + limit ); }
    void prevPage() { rebuildData( (skipped - limit) > 0 ? skipped - limit: 0 ); }
    void exactPage( int page ) { rebuildData( page*limit ); }
private:
    Storage& header;
    Storage& footer;
    Storage& data;

    int headerlen;
    int footerlen;
    int datalen;

    int limit;
    int skipped;
};

#endif // WSCROLLTABLE_H
