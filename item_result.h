/*
======================================================================

RSS Power Tool Source Code
Copyright (C) 2013 Gregory Naughton

This file is part of RSS Power Tool

RSS Power Tool is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RSS Power Tool is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RSS Power Tool  If not, see <http://www.gnu.org/licenses/>.

======================================================================
*/

#ifndef __ITEM_RESULT_H__
#define __ITEM_RESULT_H__

#define DEFAULT_QUERY_LIMIT 200

#include "dba_sqlite.h"
#include "misc.h"

// overloads row mechanism, so that if the indices is out of the size bound,
//  a new query is made inline, 
class ItemResult : public result_t
{
protected:
    
    struct res_t {
        unsigned int index;
        DBResult * res;
        res_t() : index(0), res(0) { }
        void set( unsigned int i, DBResult * r ) { index = i; res = r; }
        ~res_t() { res = 0; }
    };

    // no init
    list_t<res_t> results;
    basicString_t clause;   // client where clause, optional

    // init
    unsigned int totalRows;
    DBSqlite * DB;
    unsigned int limitSz;   // each result has up to this many rows

    basicString_t field;    // sqldate,id
    basicString_t sort;     // asc|desc

    int distinct;
    int use_priority;


    DBResult * inlineQuery( unsigned int );
    

public:

    ItemResult() : totalRows(0), DB(0), limitSz(DEFAULT_QUERY_LIMIT), field("sqldate"), sort("desc"), distinct(0), use_priority(0)
    { }

    ItemResult( DBSqlite * db ) : totalRows(0), DB(db), limitSz(DEFAULT_QUERY_LIMIT), field( "sqldate"), sort("desc"), distinct(0), use_priority(0)
    { }

    ItemResult( DBSqlite * db, unsigned int _lim ) : totalRows(0), DB(db), limitSz(_lim), field( "sqldate"), sort("desc"), distinct(0), use_priority(0)
    { }

    virtual ~ItemResult() {
        // erase these to make sure they're left alone
        DB = 0; 
    }

    void setDBHandle( DBSqlite * db ) { DB = db; }


    // indexing out of range triggers further queries until index is met, 
    // the result is added to results,
    //  exclusion_clause is updated, row is returned
    DBRow & operator[]( unsigned int );

    // total items as if this where a full query
    unsigned int numRows(); 

    // 
    void setClause( const char * str ) { clause = str; }
    basicString_t& getClause() { return clause; }

    void setDistinct(int d=1) { distinct = d; }

    void usePriority(int d=1) { use_priority = d; }
};




#endif /* __ITEM_RESULT_H__ */
