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

void WScrollTable::buildData() {
    WDataSource< RowType >::RowList rows;
    WDataSource< RowType >::RowList::iterator it;
    int rownum;
    int colnum;
    rows = data.getLineRange( 0, data.getTotalLines() - 1 );
    for ( rownum = header.getTotalLines(), it = rows.begin(); it != rows.end(); it++, rownum++ ) {
        RowType& row = *it;
        deleteRow( rownum );
        insertRow( rownum );
        for ( colnum = 0; colnum < row.size(); colnum++ ) {
            this->elementAt( rownum, colnum )->addWidget( row[ colnum ].get() );
        }
    }

}

void WScrollTable::buildFooter() {
    WDataSource< RowType >::RowList rows;
    WDataSource< RowType >::RowList::iterator it;
    int rownum;
    int colnum;
    rows = footer.getLineRange( 0, footer.getTotalLines() - 1 );
    for ( rownum = header.getTotalLines() + data.getTotalLines(), it = rows.begin(); it != rows.end(); it++, rownum++ ) {
        RowType& row = *it;
        deleteRow( rownum );
        insertRow( rownum );
        for ( colnum = 0; colnum < row.size(); colnum++ ) {
            this->elementAt( rownum, colnum )->addWidget( row[ colnum ].get() );
        }
    }
}

void WScrollTable::buildHeader() {
    WDataSource< RowType >::RowList rows;
    WDataSource< RowType >::RowList::iterator it;
    int rownum;
    int colnum;
    rows = header.getLineRange( 0, header.getTotalLines() - 1 );
    setHeaderCount( rows.size() );
    for ( rownum = 0, it = rows.begin(); it != rows.end(); it++, rownum++ ) {
        RowType& row = *it;
        deleteRow( rownum );
        insertRow( rownum );
        for ( colnum = 0; colnum < row.size(); colnum++ ) {
            this->elementAt( rownum, colnum )->addWidget( row[ colnum ].get() );
        }
    }
}
