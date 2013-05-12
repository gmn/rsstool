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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "tokenizer.h"
#include "misc.h"



/////////////////////////////////////////////////////////////////////////////
//
//  Token_t
//
char Token_t::lastChar()
{
    if ( !str ) return 0;
    unsigned int l = strlen(str);
    return str[l-1];
}
char Token_t::firstChar()
{
    if ( !str ) return 0;
    return *str;
}

Token_t * Token_t::tokenize( const char * separator )
{
    if ( !str )
        return 0;

    const char *white_space = " \t\n\r";

    if ( !separator || !*separator )
        separator = white_space;

    basicString_t buf;

    tokenStack_t stack;
    stack.setOpt( STACK_NO_DELETE );

    unsigned int index = 0, data_len = length();
    do
    {
        // first char which is not white space
        const char * black = strspn_p( &str[index], separator );

        if ( black == 0 )
            break;

        // first char after that which IS white space
        const char * white = strcspn_p( black, separator );

        if ( white == 0 )
            break;

        // isolate just the string
        buf.strncpy( black, white - black + 1 );
        buf[ white - black ] = '\0';

        stack.push( buf.str );

        index = white - &str[0];

        if ( index >= data_len )
            break;
    }
    while(1);

    return stack.head;
}

/////////////////////////////////////////////////////////////////////////////
//
//  tokenStack_t
//
const char ** tokenStack_t::getNestedArray() // creates nested array copies of the strings
{
    if ( !head ) {
        clear_array();
        return 0;
    }

    if ( array )
        clear_array();

    // quick count
    array_len = 0;
    Token_t * p = head;
    while ( p ) {
        ++array_len;
        p = p->next;
    }
    
    array = new char*[array_len+1];
    memset( array, 0, sizeof(char*) * (array_len+1) );

    p = head;
    int i = 0;
    while ( p ) {
        array[i++] = p->str; // just attach string pointer; make no copy
        p = p->next;
    }

    return const_cast<const char **>( array );
}

unsigned int tokenStack_t::getLength()
{
    return array_len;
}

void tokenStack_t::sort( int dir )
{
}

/////////////////////////////////////////////////////////////////////////////
//
// Tokenizer_t
//
Tokenizer_t::Tokenizer_t( const char * filename ) : data(0), data_len(0), stack(), own_data(0), strip_comments(0) 
{
    setFile( filename );
}

Tokenizer_t::Tokenizer_t( const char * _data , int len ) : filename(0), stack(), own_data(0), strip_comments(0)
{
    setData( _data, len );
}

Tokenizer_t::~Tokenizer_t()
{
    if ( own_data )
        delete[] data;
    if (filename)
        delete[] filename;
}

void Tokenizer_t::setFile( const char * file )
{
    assert( file && "null filename pointer in Tokenizer" );
    filename = copy_string( file );
}

// this is for tokenizing stream-input, therefor we assume the stream will be available to us,
// read-only until we have tokenized it at least, so no local copy is made.
void Tokenizer_t::setData( const char * _data, int len )
{
    if ( data && own_data ) {
        delete[] data;
    }

    data = const_cast<char *>( _data );
    data_len = len;
    own_data = 0;
}


int Tokenizer_t::read_file()
{
    char * _fullpath = realpath ( filename, NULL );

    unsigned int sz = get_filesize( _fullpath );

    FILE *fp = fopen ( _fullpath, "r" );
    if ( !fp ) {
        warning( "couldn't open \"%s\"\n", filename );
        free( _fullpath );
        return -1;
    }

    data = (char *) malloc( sz + 1 );
    memset( data, 0, sz + 1 );
    data_len = (int) sz;

    unsigned int pointer = 0, read = 0;

    do
    {  
        read = fread( &data[ pointer ], 1, 8000, fp );
        pointer += read;
        if ( 0 == read ) {
            break;
        }
    }
    while (true);
    fclose ( fp );

    assert( sz == pointer );

    own_data = 1; // so we know to free it

    free( _fullpath );
    return 0;
}

void Tokenizer_t::tokenize( const char * separator )
{
    if ( !filename && !data )
        return;

    else if ( filename )
        read_file();

    if ( ! data )
        return;

    const char *white_space = " \t\n\r";

    if ( !separator || !*separator )
        separator = white_space;

    basicString_t buf;

    int index = 0;
    do
    {
        // first char which is not white space
        const char * black = strspn_p( &data[index], separator );
        if ( black == 0 )
            break;

        // NOTE: doesn't handle /* .. */
        if ( strip_comments ) {
            // rest of line commented out?
            bool is_comment = false;
            switch (*black) {
            case '/':
                if ( *(black+1) && *(black+1) == '/' )
                    is_comment = true;
                break;
            case '#':
                is_comment = true;
                break;
            case '-':
                if ( *(black+1) && *(black+1) == '-' )
                    is_comment = true;
                break;
            default:
                break;
            }

            // got one; eat to the end of the line
            if ( is_comment ) 
            {
                // seek to end of line
                if ( 0 == (black = strcspn_p( black, "\n" )) )
                    break;

                // update index
                index = black - &data[0];
                if ( index >= data_len )
                    break;

                // back to the top
                continue;
            }

        } // strip comments

        // first char after that which IS white space
        const char * white = strcspn_p( black, separator );
        if ( white == 0 )
            break;

        // isolate just the string
        buf.strncpy( black, white - black + 1 );
        buf[ white - black ] = '\0'; // this is redundant, yes?

        stack.push( buf.str );

        index = white - &data[0];
        if ( index >= data_len )
            break;
    }
    while(1);
}

// TODO: Not implemented
const char * Tokenizer_t::minify()
{
    return const_cast<const char *>( data );
}
// TODO: Not implemented
const char * Tokenizer_t::join( const char * glue )
{
    return const_cast<const char *>( data );
}

