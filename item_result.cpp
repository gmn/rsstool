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

#include "item_result.h"

//
// query a slice of results from ( page_start to page_start+limitSz )
// slices are of size limitSz, except for the last one
// store in linked-list. so we only get slices that are accessed
// 
DBResult * ItemResult::inlineQuery( unsigned int page_start )
{
    basicString_t buf;
    basicString_t query;
    query.sprintf( "select%sfeed.title as ftitle,feed_id,item.* from item,item_feeds,feed where item_feeds.item_id = item.id and item_feeds.feed_id = feed.id", distinct ? " distinct " : " " ); 
    // conditional
    if ( clause.length() ) {
        query += " and ";
        query += clause;
    }

    // finish
    query += buf.sprintf( " order by %s %s limit %u offset %u;", field.str, sort.str, limitSz, page_start );
    DBResult * res = DB->query( query.str );


    // alocate holder and set
    node_t<res_t> * res_p = results.allocate();
    res_p->val.set( page_start, res );
    
    // insert into list
    if ( results.size() == 0 ) { // empty list
        results.insertAfter( res_p );
        return res;
    }
    
    node_t<res_t> * node = results.gethead();

    // insert before head
    if ( page_start < node->val.index ) {
        results.insertAfter( res_p, 0 );
        return res;
    } 

    node = results.gettail();
    
    while ( node && page_start < node->val.index ) 
    {
        node = node->prev;
    }

    results.insertAfter( res_p, node );

    return res;
}

// 
DBRow & ItemResult::operator[]( unsigned int index )
{
    if ( !results.size() ) {
        DBResult * res = inlineQuery( (index/limitSz) * limitSz );
        return (*res)[ index % limitSz ];
    }

    // look for result page
    node_t<res_t> * node = results.gethead();
    while ( node ) 
    {
        if ( index >= node->val.index && index < node->val.index + limitSz )
            return node->val.res->getRow( index - node->val.index );
        node = node->next;
    }

    // not found, get results from query
    DBResult * res = inlineQuery( (index/limitSz) * limitSz );
    return res->getRow( index % limitSz );
}


// total items as if this where a full query
unsigned int ItemResult::numRows()
{
    if ( 0 == totalRows ) 
    {
        basicString_t query( "select count(item.id) as c from item,item_feeds,feed where item_feeds.item_id = item.id and item_feeds.feed_id = feed.id" );
        if ( clause.length() ) {
            query += " and ";
            query += clause;
        }
        query += ';';
        
        DBResult * res = DB->query( query.str );
        if ( res ) {
            DBValue * v = res->FindByNameFirstRow( "c" );
            totalRows = v ? v->getInt() : 0;
        }
    }

    return totalRows;
}

