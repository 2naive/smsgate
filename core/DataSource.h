#ifndef DATASOURCE_H
#define DATASOURCE_H

#include "PGSql.h"

template < class RowType >
class WDataSource {
public:
    typedef std::vector< RowType > RowList;
    WDataSource();
    virtual int getTotalLines() = 0;

    RowList getLineRange( int lnl, int lnr );
    void releaseCache();
private:
    virtual void execute( int lnl, int lnr, RowList& data ) = 0;

    RowList cache;
    int __cache_from;
    int __lines;
};

template < class RowType >
WDataSource< RowType >::WDataSource() {
        __lines = 0;
        __cache_from = 0;
}

template < class RowType >
typename WDataSource< RowType >::RowList
WDataSource< RowType >::getLineRange( int lnl, int lnr ) {
    if ( ( lnl >= __cache_from ) && ( lnr <= __cache_from + __lines ) ) {
        return RowList( &cache[ lnl - __cache_from ], &cache[ lnr - __cache_from ] );
    }

    execute( lnl, lnr, cache );
    return cache;
}

template < class RowType >
void WDataSource< RowType >::releaseCache() {
    cache.resize( 0 );
}


#endif // DATASOURCE_H
