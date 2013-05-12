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

// dba_sqlite.h
#ifndef __DBA_SQLITE__
#define __DBA_SQLITE__


#include <string.h>
#include "sqlite3.h"

#include "datastruct.h"     // cppbuffer_t
#include "misc.h"           // basicString_t


#define NO_DB_NAME "unnamed.db" 


enum DBStatementType
{
STMT_ERROR = -1,
STMT_NONE = 0,
STMT_INSERT = 11,
STMT_SELECT,
STMT_UPDATE,
STMT_CREATE,
STMT_DELETE,
STMT_ALTER,
STMT_DROP,
};



/********************************************************
 *
 *  DBValue
 *
 */
class DBValue
{
protected:
    int             ival;
    double          fval;
    basicString_t   sval;

    const char*     _name; // points to DBResult::col_names[N].str

public:

    DBValue() : ival(0),fval(0),sval(),_name(0) 
    { }

    DBValue( int i );
    DBValue( double f );
    DBValue( const char * );

    void setInt( int );
    void setFloat( double );
    void setString( const char * );
    void setName( const char *n ) { _name = n; }

    int getInt() const { return ival; }
    double getFloat() const { return fval; }
    const char * getString() const { return sval.str; }
    const char * name() { return _name; }
    
    ~DBValue(); 
};


/********************************************************
 *
 *  DBRow
 *
 */
class DBRow
{
protected:

    cppbuffer_t<DBValue*> values;
    int _rowNum;

public:

    DBRow() : values(), _rowNum(0)
    { }
    DBRow( int _num ) : values(), _rowNum(_num)
    { }
    ~DBRow(); 

    //void addVal( const DBValue & v ) { values.push_back( v ); }
    void addVal( const char * );

    unsigned int size() { return values.size(); }
    unsigned int numCols() { return values.size(); }

    DBValue & operator[] ( unsigned int );

    int rowNum() const { return _rowNum; }

    DBValue * FindByName( const char * colname );
    DBValue * FindByName( const basicString_t& colname );

    // these all return 0 if column name is not found
    const char * getString( const char * );
    int getInt( const char * );
    double getFloat( const char * );
};


// abstract vessel for polymorphic use of different result types
struct result_t
{
    virtual DBRow & operator[] ( unsigned int ) = 0;
    virtual unsigned int numRows() = 0;
    virtual ~result_t() { }
};


/********************************************************
 *
 *  DBResult
 *
 */
class DBResult : public result_t
{
protected:

    cppbuffer_t<DBRow*> rows;
    
    cppbuffer_t<basicString_t*> col_names;
    
    int last_insert_id;

    DBStatementType _statementType;

    basicString_t query_string;

    int _rows_updated;

    basicString_t separator;

    unsigned int nextCount;

public:

    DBResult() : rows(), col_names(), last_insert_id(-1), _statementType(STMT_NONE), query_string(), _rows_updated(0), separator("\t"), nextCount((unsigned)-1)
    {}

    virtual ~DBResult();

    void addRow( DBRow* r ) { rows.push_back( r ); }

    unsigned int numRows() { return rows.size(); }
    unsigned int size() { return rows.size(); }

    DBRow & getRow( unsigned int );
    DBRow & operator[] ( unsigned int );
    DBRow * NextRow();
    void resetRowCount() { nextCount = (unsigned)-1; }

    void addColName ( const char * );

    unsigned int colCount() { return col_names.size(); }
    const char * colName( unsigned int );

    // returns -1 when not STMT_INSERT
    int lastInsertId() const { return last_insert_id; }
    void setInsertId( int i ) { last_insert_id = i; }

    // is set on DELETE, UPDATE, INSERT
    int rowsUpdated() { return _rows_updated; }
    void setUpdated( int i ) { _rows_updated = i; }

    DBStatementType statementType() const { return _statementType; }
    void setType( DBStatementType t ) { _statementType = t; }

    const char * getQueryString() const { return query_string.str; }
    void setQueryString( const char * );

    void eraseAndReset(); 
    void freeResultData() { eraseAndReset(); } // alias

    void Print();

    DBValue * FindByNameFirstRow( const char * );

};


/********************************************************
 *
 *  DBSqlite
 *
 *  - Main wrapper / Interface
 *
 */
class DBSqlite
{
protected:
    basicString_t db_name; 

    basicString_t fullpath;

    sqlite3 *db;

    cppbuffer_t<DBResult *> savedResults;


    //
    int try_open_db();

    DBStatementType determine_statement_type( const char * );

    void setColumnNamePointers( DBResult& );

public:
    // not wise, you almost always want a named DB to do any real work
    //  never-the-less, this might be useful for debugging/testing
    DBSqlite() : db_name( NO_DB_NAME ), fullpath(), db(0)
    { }
    
    DBSqlite( const char * name ) : db_name( name ), fullpath(), db(0)
    { }

    void setName( const char * new_name ) {
        db_name.set( new_name );
    }

    virtual ~DBSqlite();

    // free's previously returned results, including any that may be in use.
    void nukeSavedResults(); 

    // interface
    DBResult * query( const char * );
    DBResult * operator()( const char * s ) { return query(s); }
    
    const char * name() const { return db_name.str; }

    void fixQuotes( basicString_t & );

    void BeginTransaction();
    void Commit();

}; // DBSqlite




#endif /* __DBA_SQLITE__ */
