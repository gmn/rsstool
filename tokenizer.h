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

#ifndef __TOKENIZER_H__
#define __TOKENIZER_H__

#include "misc.h"


// a doubly linked list of strings
struct Token_t : public basicString_t
{
    Token_t * next;
    Token_t * prev;
    Token_t() : next(0),prev(0)
    { }
    Token_t( const char * s ) : basicString_t(s), next(0),prev(0)
    { }
    Token_t( const Token_t& ref ) : basicString_t( ref ), next(0),prev(0)
    { }

    Token_t& operator=( const Token_t& t )
    {  
        if ( &t != this )
        {  
            basicString_t::operator=( t );
            //next = prev = 0; no point in unsetting, or setting these. 
            // just leave untouched in case setting tok in linked-list
        }
        return *this;
    }

    char lastChar();
    char firstChar();

    Token_t * tokenize( const char * =0 ); // tokenize token, null arg defaults to standard whitespace: " \t\n"
};

// 2 strings
struct doubleToken_t
{
    basicString_t A;
    basicString_t B;

    doubleToken_t * next;
    doubleToken_t * prev;

    doubleToken_t() : A(),B(), next(0),prev(0)
    { }
    doubleToken_t( const char * s, const char * t ) : A(s),B(t), next(0),prev(0)
    { }

    doubleToken_t& operator=( const doubleToken_t& t )
    {  
        if ( &t != this )
        {
            A.set( t.A );
            B.set( t.B );
            next = prev = 0;
        }
        return *this;
    }
};

enum
{
STACK_RESET = 0,
SORT_DIRECTION_ASCENDING = 1,
SORT_DIRECTION_DESCENDING = 2,
STACK_NO_DELETE = 4
};

struct tokenStack_t
{
protected:
    // only constructed upon request 
    char ** array;            
    unsigned int array_len;
    unsigned int options;

public:
    Token_t * head, * last;

    tokenStack_t() : array(0), array_len(0), options(STACK_RESET), head(0), last(0) 
    { }

    virtual ~tokenStack_t() {
        if ( (options & STACK_NO_DELETE) != STACK_NO_DELETE ) {
            Token_t * p = head;
            while ( p ) {
                Token_t * next = p->next;
                delete p;
                p = next;
            }
        }
        clear_array(); 
    }

    void setOpt( unsigned int opt )
    {
        if ( opt == STACK_RESET )
            options = STACK_RESET;
        else
            options |= opt;
    }

    void clear_array()
    {
        if ( array )
        {
            delete[] array;
        }
        array_len = 0;
        array = 0;
    }

    void push( const char * str ) 
    {
        if ( head ) {
            last->next = new Token_t( str );
            last->next->prev = last;
            last = last->next;
        } else {
            last = head = new Token_t( str );
        }
    }

    const char ** getNestedArray();
    unsigned int getLength();                   // only works for array, 0 if not built
    void sort( int dir=SORT_DIRECTION_ASCENDING );
};

struct doubleTokenStack_t
{
    doubleToken_t * head, * last;

    doubleTokenStack_t() : head(0), last(0) 
    { }

    ~doubleTokenStack_t() {
        doubleToken_t * p = head;
        while ( p ) {
            doubleToken_t * next = p->next;
            delete p;
            p = next;
        }
    }

    void push( const char * Astr, const char * Bstr ) 
    {
        if ( head ) {
            last->next = new doubleToken_t( Astr, Bstr );
            last->next->prev = last;
            last = last->next;
        } else {
            last = head = new doubleToken_t( Astr, Bstr );
        }
    }
};

// generates tokens from input
struct Tokenizer_t
{
    char * filename;
    char * data;
    int data_len;
    tokenStack_t stack;

    Tokenizer_t() : filename(0), data(0), data_len(0), own_data(0), strip_comments(0)
    { }

    // 1 argument assumes filename
    Tokenizer_t( const char * );

    // 2 args, assumes string input
    Tokenizer_t( const char *, int );

    ~Tokenizer_t();

    // hand in string to tokenize
    void setData( const char *, int );

    // set file to tokenize
    void setFile( const char * );

    // do it
    void tokenize( const char * =0 );

    // hand off singularly linked token list for processing elsewhere
    Token_t * getHead() { return stack.head; }

    // TODO: NOT IMPLEMENTED, but thought potentially useful
    const char * minify();
    void setStripComments( int in =1 ) { strip_comments = in; }
    const char * join( const char * glue );

private:
    int own_data;
    int strip_comments;
    int read_file();
};

#endif /* __TOKENIZER_H__ */
