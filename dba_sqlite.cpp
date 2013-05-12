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

// dba_sqlite.cpp

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>


#include "dba_sqlite.h"
#include "misc.h"
#include "datastruct.h"

const char * sqlite_error_string( int c ) 
{
    switch ( c )
    {
    case SQLITE_OK: return "SQLITE_OK";
    case SQLITE_ERROR: return "SQLITE_ERROR";       
    case SQLITE_INTERNAL: return "SQLITE_INTERNAL";
    case SQLITE_PERM: return "SQLITE_PERM";
    case SQLITE_ABORT: return "SQLITE_ABORT";
    case SQLITE_BUSY: return "SQLITE_BUSY";
    case SQLITE_LOCKED: return "SQLITE_LOCKED";
    case SQLITE_NOMEM: return "SQLITE_NOMEM";
    case SQLITE_READONLY: return "SQLITE_READONLY";
    case SQLITE_INTERRUPT: return "SQLITE_INTERRUPT";
    case SQLITE_IOERR: return "SQLITE_IOERR";
    case SQLITE_CORRUPT: return "SQLITE_CORRUPT";
    case SQLITE_NOTFOUND: return "SQLITE_NOTFOUND";
    case SQLITE_FULL: return "SQLITE_FULL";
    case SQLITE_CANTOPEN: return "SQLITE_CANTOPEN";
    case SQLITE_PROTOCOL: return "SQLITE_PROTOCOL";
    case SQLITE_EMPTY: return "SQLITE_EMPTY";
    case SQLITE_SCHEMA: return "SQLITE_SCHEMA";
    case SQLITE_TOOBIG: return "SQLITE_TOOBIG";
    case SQLITE_CONSTRAINT: return "SQLITE_CONSTRAINT";
    case SQLITE_MISMATCH: return "SQLITE_MISMATCH";
    case SQLITE_MISUSE: return "SQLITE_MISUSE";
    case SQLITE_NOLFS: return "SQLITE_NOLFS";
    case SQLITE_AUTH: return "SQLITE_AUTH";
    case SQLITE_FORMAT: return "SQLITE_FORMAT";
    case SQLITE_RANGE: return "SQLITE_RANGE";
    case SQLITE_NOTADB: return "SQLITE_NOTADB";
    case SQLITE_ROW: return "SQLITE_ROW";
    case SQLITE_DONE: return "SQLITE_DONE";
    }
    return "";
}

/********************************************************************
 *
 * DBValue
 *
 ********************************************************************/

DBValue::DBValue( int i ) : ival(0),fval(0),sval(),_name(0)
{
    setInt( i );
}

DBValue::DBValue( double f ) : ival(0),fval(0),sval(),_name(0)
{
    setFloat( f );
}

DBValue::DBValue( const char * str ) : ival(0),fval(0),sval(),_name(0)
{
    setString( str );
}

void DBValue::setInt( int i )
{
    ival = i;

    char buf[50];
    memset(buf,0,50);
    snprintf( buf, 50, "%i", i );
    sval.strncpy( buf, 50 );

    fval = (double) ival;
}

void DBValue::setFloat( double f )
{
    ival = (int) f;

    char buf[50];
    memset(buf,0,50);
    snprintf( buf, 50, "%f", f );
    sval.strncpy( buf, 50 );

    fval = f;
}

void DBValue::setString( const char * str )
{
    if ( !str || !*str ) {
        sval.erase();
        ival = 0;
        fval = 0.0;
        return;
    }
    sval = str;
    ival = atoi( str );
    fval = atof( str );
}

DBValue::~DBValue() 
{ }



/********************************************************************
 *
 * DBRow
 *
 ********************************************************************/

DBRow::~DBRow() 
{ 
    for ( unsigned int i = 0 ; i < values.count(); i++ ) {
        delete values[i];
    }
    values.reset(); // resets the push_back counter
}

void DBRow::addVal( const char * str ) 
{
    DBValue * v = new DBValue( str );
    values.push_back( v );
}

DBValue & DBRow::operator[]( unsigned int ind ) 
{
    if ( ind >= values.size() && ind < values.size() )
        return *values[0];

    return *values[ind];
}

DBValue * DBRow::FindByName( const char * colname )
{
    if ( !colname || !*colname )
        return 0;

    basicString_t tester( colname );

    return FindByName( tester );
}

DBValue * DBRow::FindByName( const basicString_t& col )
{
    // not matching empty names
    if ( col.length() == 0 )
        return 0;

    for ( unsigned int i = 0 ; i < values.count(); i++ ) {
        const char * _name = values[i]->name();
        if ( !_name )
            return 0;
        if ( col.icompare( _name ) )
            return values[i];
    }
    return 0;
}

const char * DBRow::getString( const char * col )
{
    if ( !col || !*col )
        return 0;
    basicString_t tester( col );
    DBValue * v = FindByName( tester );
    if ( !v || !v->getString() )
        return 0;
    return v->getString();
}

int DBRow::getInt( const char * col )
{
    if ( !col || !*col )
        return 0;
    basicString_t tester( col );
    DBValue * v = FindByName( tester );
    if ( !v )
        return 0;
    return v->getInt();
}

double DBRow::getFloat( const char * col )
{
    if ( !col || !*col )
        return 0.0;
    basicString_t tester( col );
    DBValue * v = FindByName( tester );
    if ( !v )
        return 0;
    return v->getFloat();
}


/********************************************************************
 *
 * DBResult
 *
 ********************************************************************/

DBResult::~DBResult() 
{
    eraseAndReset();
}

DBRow& DBResult::getRow( unsigned int r )
{
    if ( 0 == rows.size() || r >= rows.size() ) 
        return *rows[0];
    
    return *rows[ r ];
}

DBRow& DBResult::operator[]( unsigned int r )
{
    return getRow(r);
}

DBRow * DBResult::NextRow()
{
    if ( ++nextCount >= numRows() ) {
        return 0;
    }
    return rows[nextCount];
}

void DBResult::addColName ( const char * c_str )
{
    basicString_t * str_p = new basicString_t( c_str );
    col_names.push_back( str_p );
}

const char * DBResult::colName( unsigned int j ) 
{
    if ( j >= col_names.size() || col_names.size() == 0 )
        return 0;

    return const_cast<const char *>( col_names[j]->str );
}


// erases contents and resets. safe to use in dtor. safe to use
//  prematurely before the dtor() as well.
void DBResult::eraseAndReset()
{
    if ( _statementType == STMT_NONE )
        return;

    for ( unsigned int i = 0; i < rows.count(); i++ ) {
        delete rows[i];
    }
    rows.reset();

    for ( unsigned int i = 0; i < col_names.count(); i++ ) {
        delete col_names[i];
    }
    col_names.reset();

    _statementType = STMT_NONE;
    last_insert_id = -1;
    query_string.erase();
    _rows_updated = 0;
}
    
void DBResult::setQueryString( const char * str )
{
    query_string.set( str );
}

void DBResult::Print() 
{
    for ( unsigned int i = 0; i < col_names.count(); i++ )
    {  
        printf( "%s%s", col_names[i]->str, separator.str );
    }
    printf( "\n" );

    for ( unsigned int j = 0; j < numRows(); j++ )
    {  
        DBRow & row = (*this)[ j ];
        for ( unsigned int i = 0; i < row.numCols(); i++ )
        {  
            printf( "%s%s", row[i].getString(), separator.str );
        }
        printf( "\n" );
    }
}

// find a column value in the first row of results
DBValue * DBResult::FindByNameFirstRow( const char *chk )
{
    if ( numRows() == 0 )
        return 0;
    DBRow& row = getRow( 0 );
    return row.FindByName( chk );
}



/********************************************************************
 *
 * DBSqlite
 *
 ********************************************************************/

int DBSqlite::try_open_db()
{
    if ( db ) 
        return 1;

    char * path = 0;

    // fullpath not set
    if ( 0 == fullpath.length() )
    {
        // try to establish fullpath from existing file
        path = realpath( db_name.str, NULL );
        if ( !path )
        {
            //warning( "\"%s\" not found. Trying to create it.\n", db_name.str ); 

            if ( 0 == db ) { 
                // not found: try to open by db_name
                sqlite3_open( db_name.str, &db );
                if ( 0 == db ) {
                    error( "database: \"%s\" could not be opened\n", db_name.str );
                }
            }

            // try to expand path again
            path = realpath( db_name.str, NULL );
            if ( path ) {
                fullpath = path;
                free( path );
                path = 0;
            }
        }
        // file found: set fullpath and try to open
        else 
        {
            fullpath = path;
            free( path );
            path = 0;
            if ( 0 == db ) { 
                sqlite3_open( fullpath.str, &db );
                if (db == 0) {
                    error( "database: \"%s\" found but could not be opened\n", fullpath.str );
                }
            }
        }
    }
    // open using fullpath, if not already open
    else
    {
        if ( 0 == db ) {
            sqlite3_open( fullpath.str, &db );
            if (db == 0) {
                error( "database: \"%s\" could not be opened\n", fullpath.str );
            }
        }
    }

    return 1;
}




// 3 broad statement types, detected in this order: 
//  INSERT, SELECT, EVERYTHING ELSE
DBStatementType DBSqlite::determine_statement_type( const char * str )
{
    if ( !str || !*str )
        return STMT_ERROR;

    // GNU strcasestr is not null safe; use basicString_t::stristr() instead
    basicString_t query( str );

    // detecting first token at beginning of string should ensure the type of the query

    if ( query.stristr( "INSERT" ) == query.str )
    {
        return STMT_INSERT;
    }
    else if ( query.stristr( "SELECT" ) == query.str )
    {
        return STMT_SELECT;
    }
    else if ( query.stristr( "UPDATE" ) == query.str )
    {
        return STMT_UPDATE;
    }
    else if ( query.stristr( "CREATE" ) == query.str )
    {
        return STMT_CREATE;
    }
    else if ( query.stristr( "DELETE" ) == query.str )
    {
        return STMT_DELETE;
    }
    else if ( query.stristr( "ALTER TABLE" ) == query.str )
    {
        return STMT_ALTER;
    }
    else if ( query.stristr( "DROP TABLE" ) == query.str )
    {
        return STMT_DROP;
    }

    return STMT_ERROR;
}


void DBSqlite::setColumnNamePointers( DBResult& result )
{
    for ( unsigned int j = 0; j < result.numRows(); j++ )
    {  
        DBRow & row = result[ j ];
        for ( unsigned int i = 0; i < row.numCols(); i++ )
        {  
            row[i].setName( result.colName(i) );
        }
    }
}

DBResult * DBSqlite::query( const char * str ) 
{
    if ( !str || !*str )
        return 0;

    try_open_db();

    // 
    basicString_t trimmer( str );
    trimmer.trim();
    str = trimmer.str;

    DBStatementType _type = determine_statement_type( str );

    if ( STMT_ERROR == _type )
        return 0;

    DBResult * result = new DBResult;

    result->setType( _type );

    sqlite3_stmt *pStmt = NULL;
    const char * zSql = str;

    // points to first byte after processed SQL statment (can use for running multiple semi-colon separated statements)
    const char * zLeftover; 

    const char * errmsg;

    // creates & populates pStmt object
    int rc = sqlite3_prepare_v2( db, zSql, -1, &pStmt, &zLeftover );


    if ( rc != SQLITE_OK ) 
    {
        errmsg = sqlite3_errmsg(db);
        warning( "sqlite3_prepare_v2 returned: %s with message: \"%s\", on query string: \"%s\"\n", sqlite_error_string(rc), errmsg, str );
        delete result;
        return 0;
    }

    bool needColNames = true;
    int rowNum = 1;

    while( 1 )
    {
        // perform the first step.  this will tell us if we
        // have a result set or not and how wide it is.
        rc = sqlite3_step( pStmt );

        if( SQLITE_ROW == rc )
        {
            int nCol = sqlite3_column_count(pStmt);

            DBRow * row = new DBRow( rowNum++ ); 

            for ( int i = 0; needColNames && i < nCol; i++ ) 
            {
                const char * cName = sqlite3_column_name( pStmt, i );
                result->addColName( cName ); // copies it, therefor safe
            }
            needColNames = false;

            for ( int i = 0; i < nCol; i++ ) 
            {

                const char *colText = (const char *) sqlite3_column_text(pStmt, i);

                //  SQLITE_INTEGER, SQLITE_FLOAT, SQLITE_TEXT, SQLITE_BLOB, or SQLITE_NULL
                //int colType = sqlite3_column_type(pStmt, i);
                //-figure out how to get full-type as a string

                // stop these "(null)" strings from propagating
                if ( colText && strcmp( "(null)", colText ) )
                    row->addVal( colText );
                else
                    row->addVal( 0 );
            }

	        result->addRow( row );
        }
        else
        // FIXME check for other return types, ie. Errors
        {
            //errmsg = sqlite3_errmsg(db);
            //warning( "sqlite3_step returned: %s with message: \"%s\", on query string: \"%s\"\n", sqlite_error_string(rc), errmsg, str );
            break;
        }
    }

    // frees pStmt
    rc = sqlite3_finalize(pStmt);


    int changes = 0;
    sqlite3_int64 insert_id;
    

    switch ( result->statementType() )
    {
        case STMT_UPDATE:
        case STMT_DELETE:
            changes = sqlite3_changes( db );
            result->setUpdated( changes );
            break;
        case STMT_SELECT:
            // set column name pointers in DBValue results
            setColumnNamePointers( *result );
            break;
        case STMT_INSERT:
            insert_id = sqlite3_last_insert_rowid( db );
            result->setInsertId( insert_id );
            changes = sqlite3_changes( db );
            result->setUpdated( changes );
            break;
        default:
            break;
    }

    result->setQueryString( str );
    savedResults.add( result );

    return result;
}

DBSqlite::~DBSqlite()
{
    nukeSavedResults();

    if ( db ) 
        sqlite3_close( db );
}

void DBSqlite::nukeSavedResults()
{
    for ( unsigned int i = 0 ; i < savedResults.count(); i++ ) { 
        if ( savedResults[i] )
            delete savedResults[i];
        savedResults[i] = 0;
    }
    savedResults.reset();
}

// sqlite strings are quoted by single quotes. They are the only ones you need to fix.
void DBSqlite::fixQuotes( basicString_t & S )
{
    if ( S.length() == 0 )
        return;

    if ( !strchr( S.str, '\'' ) )
        return; // none found, leave unaltered

    basicString_t fixed;
    fixed.setMem( S.length() * 2 );

    // FIXME: you could totally use strchr to scan for \' and go right to them; appending chunks = much faster, but I don't care
    
    for ( unsigned int i = 0; i < S.length(); i++ ) {
        if ( S[i] == '\'' && i < S.length()-1 && S[i+1] != '\'' ) {
            fixed.append( "''", 2 );
        } else if ( S[i] == '\'' && i < S.length()-1 && S[i+1] == '\'' ) { 
            ++i;  // gotta skip ahead to avoid quoting 2nd single-quote
        } else if ( S[i] == '\'' && i == S.length()-1 ) { 
            fixed.append( "''", 2 ); // special case, single-quote is last character in the string
        } else {
            fixed.append( &S[i], 1 );
        }
    }

    S = fixed;
}

void DBSqlite::BeginTransaction()
{
    try_open_db();

    //sqlite3_exec(db, "BEGIN", 0, 0, 0);
    sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, 0);
}

void DBSqlite::Commit()
{
    try_open_db();

    //sqlite3_exec(db, "COMMIT", 0, 0, 0);
    sqlite3_exec(db, "END TRANSACTION;", 0, 0, 0);
}

