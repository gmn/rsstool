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

#ifndef __MISC_H__
#define __MISC_H__

#include "datastruct.h"

#define _VA_BUF_SZ 8192


inline char char_tolower( char c ) {
    return ( c >= 'A' && c <= 'Z' ) ? (c - 'A') + 'a' : c;
}

inline char char_toupper( char c ) {
    return ( c >= 'a' && c <= 'z' ) ? (c - 'a') + 'A' : c;
}

// converts whole string, in place
inline char * str_toupper( char * s ) {
    char * p = s;
    while ( *p ) {
        int n = *p;
        *p = char_toupper( n );
        ++p;
    }
    return s;
}

inline char * str_tolower( char * s ) {
    char * p = s;
    while ( *p ) {
        int n = *p;
        *p = char_tolower( n );
        ++p;
    }
    return s;
}

/* replaces instance of "search" with "replace" in subject, in place */
inline void str_replace( const char s, const char r, char * q )
{
    while ( *q++ )
        if ( *q == s )
            *q = r;
}

class basicArray_t;

/////////////////////////////////////////////////////////////////////////////
//
//  basicString_t
//
struct basicString_t
{
    char *          str;

    unsigned int    len;        // current strlen
    unsigned int    memlen;     // length of memory in bytes; 
                                // -these will differ if overwritten with shorter string

    basicString_t() : str(0), len(0), memlen(0)
    { }

    basicString_t( const char * A );

    // correct copy structor: needed when there is pointer/heap ("deep data")
    basicString_t( const basicString_t& t );

    // this is also needed, or else if assignment is used the compiler 
    //  will provide one that copies the pointer
    basicString_t& operator=( const basicString_t& t );

    virtual ~basicString_t() { 
        if (str) 
            delete[] str; 
    }
    
    void clearMem() {
        if (str) 
            delete[] str; 
        str = 0;
        memlen = len = 0;
    }

    void erase() {
        if (str)
            memset( str, 0, memlen );
        len = 0;
    }
    void clear() { return erase(); }

    // pre-allocate or explicitly set the internal memory; will truncate string if 
    //  new value is less than len, otherwise, will pad with \0
    void setMem( unsigned int ) ;
   

    virtual basicString_t& set( const char * );

    virtual basicString_t& set( const basicString_t& );

    basicString_t& operator=( const char * in ) { return set( in ); }

    virtual void Print( const char * =0 );

    basicString_t& strncpy( const char *, unsigned int );
    basicString_t& sprintf( const char *, ... );

    char& operator[]( unsigned int );
    char& operator[]( int w ) { return (*this)[ (unsigned int)w ]; }
    char& operator[]( long int w ) { return (*this)[ (unsigned int)w ]; }

    basicString_t& toLower() { str_tolower(str); return *this; }
    basicString_t& toUpper() { str_toupper(str); return *this; }
    unsigned int length() const { return len; }

    basicString_t& append( const char *, unsigned int =0 );
    basicString_t& append( const basicString_t & );
    basicString_t& operator+=( const char * );
    basicString_t& operator+=( const basicString_t& );
    basicString_t& operator+=( const char );
    basicString_t& operator+=( const int );
    basicString_t& operator+=( const float );
    basicString_t& operator+=( const double );

    bool compare( const char * );
    bool compare( const basicString_t& );
    bool operator==( const char * );
    bool operator==( const basicString_t& );
    bool operator!=( const char * );
    bool operator!=( const basicString_t& );
    bool icompare( const char * );
    bool icompare( const char * ) const;
    bool icompare( const basicString_t& );

    basicString_t& trim();

    const char * strstr( const char * );
    const char * stristr( const char * );
    const char * strcasestr( const char * ); // alias
    const char * strstr( const basicString_t& );
    const char * stristr( const basicString_t& );
    const char * strcasestr( const basicString_t& ); // alias

    basicString_t operator+( const char * );
    basicString_t operator+( const basicString_t& );

    basicString_t substr( unsigned int, unsigned int );

    // null arg is std whitespace
    basicArray_t * explode( const char * =0, int stripcom =0 ); 

    char last() {
        if ( !str || !*str || !len )
            return 0;
        return str[len-1];
    }

    char first() {
        if ( !str || !*str || !len ) 
            return 0;
        return *str;
    }

    // in place, return *this
    basicString_t& replace( const char * find, const char * replace );

#if 0
    char& operator*( void ) {
        if ( !str ) return (char)str;
        return *str;
    } 
#endif

    // TODO
    // - replace(const char*,const char*) // alloc new string
    // - replace(basicString_t&,basicString_t&) // alloc new string
    // - basicString_t::set( int ),  // this does sprintf( "%d", int );
    // - basicString_t::set( float ), // ditto
    // - basicString_t::append( int ); // 
    // - basicString_t::append( float ); // double-ditto

};
//
/////////////////////////////////////////////////////////////////////////////


// 
struct dirlist_t
{   
private:
    int _read_dir( const char * ); 

public:
    int read( const char * =0 ); 

    //void add_filter( const char * ); // all or part must match this string

    void add_suffix_filter( const char * ); // file suffix must match a filter exactly, or else is skipped

    void sort_alphabetical( void );


    unsigned int current_entry;
    char ** filenames;
    unsigned int total_entries;
    char * path;
    unsigned int suffix_count;
    cppbuffer_t<basicString_t> suffix;

    dirlist_t() : current_entry(-1),filenames(0),total_entries(0),path(0),suffix_count(0)
    { }

    // sets the path to be opened
    dirlist_t(const char *_p) : current_entry(-1),filenames(0),total_entries(0),path((char*)_p),suffix_count(0)
    { }


    void rewind() {
        current_entry = -1;
    }

    char * next() {
        if ( ++current_entry < total_entries )
            return filenames[ current_entry ];
        return 0;
    }

    void free_listdir( char ** pp );

    ~dirlist_t() ;
};



const char * strspn_p( const char * haystack, const char * whitelist );
const char * strcspn_p( const char * haystack, const char * whitelist );

inline int is64() {
    return sizeof(void*) == 8;
}

void error( const char * fmt, ... );
void warning( const char * fmt, ... );
char * copy_string( const char * s );

const char * strip_path( const char *s );
const char * strip_extension( const char *s );
const char * strip_path_and_extension( const char * s );

unsigned int file_size( const char * path ); // using stdio 
unsigned int get_filesize( const char * path ); // using sys/stat

void _hidden_Assert( int, const char *, const char *, int );
#ifdef _DEBUG
#define Assert( v ) _hidden_Assert( ((v)), #v, __FILE__, __LINE__ )
#else
#define Assert( v )
#endif

//
#define BIT(x) (1<<x)


// returns 0 if not found, or fullpath if successful.
// return must be freed by free()
const char * file_exists( const char * );

// 
int file_put_contents( const char * filename, const char * data, unsigned int len=0 );
//
int file_get_contents( const char *fname, basicString_t& ref );

int make_dir( const char *path, int mode );

const char * system_id();


// assumes usage case of adding 'const char*' strings using 'push_back( const char * )'
struct stringbuffer_t : public cppbuffer_t<basicString_t *>
{
    void push_back( const char * str ) {
        basicString_t * str_p = new basicString_t( str );
        cppbuffer_t<basicString_t*>::push_back( str_p );
    }

    ~stringbuffer_t() {
        for ( unsigned int i = 0; i < count(); i++ ) {
            delete (*this)[i];
        }
    }
    // FIXME: why isn't this inheriting properly?  The compiler isn't recognizing the base methods?
    void push_back( basicString_t * str_p ) {
        cppbuffer_t<basicString_t*>::push_back( str_p );
    }

    void erase() {
        for ( unsigned int i = 0; i < count(); i++ ) {
            delete (*this)[i];
        }
        reset();
    }

    // FIXME: wanted:
    // - copy constructor
    // - operator=(const stringbuffer_t&)
};


struct uiPair_t
{
    unsigned int start;
    unsigned int stop;
};


//////////////////////////////////// HTML TAG STRIPPER ////////////////////////////////////////
//
/*
 * need:
 *  X remove everything between <script> tags
 *  X HREF parsing, print out the URLs in rows
 *  X some special char recognition: &#8210; &nbsp; &lt; &amp; &gt; etc., or just throw them out
 *  X turn into a class and put in Misc.
 *  X better re-written tag identification.
 *  X H1-H5 newlines and upper-casing
 *  X B & STRONG upper-casing
 *  - line breaking; break whole words
 *
 */

#include "html_entities.h"

class HtmlTagStripper
{
    HtmlEntities_t html_ent;

    enum Tag_t
    {
        T_NONE,
        T_OPENING,  // <b>
        T_CLOSING,  // </b>
        T_CLOSED    // <br/>
    };

    // a string inside of a string, I love it.  Take that gang-of-4!
    struct tag_t : public basicString_t
    {
        Tag_t type;
        basicString_t contents;

        tag_t() : type( T_NONE )
        { }

        void erase() {
            type = T_NONE;
            basicString_t::erase();
            contents.erase();
        }

        tag_t& operator=( const tag_t& t )
        {
            if ( &t != this )
            {
                type = t.type;
                basicString_t::operator=( t );
                contents.set( t.contents );
            }
            return *this;
        }

        tag_t& operator=( const char * s )
        {
            basicString_t::set( s );
            return *this;
        }
    };


    tag_t tag;
    tag_t lastTag;
    basicString_t line_buffer;
    basicString_t entity;

    // returns 1 if valid tag, 0 if not. Sets tag.contents to entire tag
    int classifyTag( tag_t& in );

public:

    HtmlTagStripper() ;

    // interface
    void strip( const basicString_t& in, basicString_t& out, bool htmlUrls =false );
};
//
//////////////////////////////// end HTML TAG STRIPPER ////////////////////////////////////////

/*
class extends cppbuffer_t<const char *>, takes argument of col-width & builds list of pointers
    to lines. Increments internal counter, and supplies convenience method for getting pointer to line,
        const char * ::line(unsigned int).
    counting characters or scanning for '\n'.  That's all there is to it.  Just pointers, no copying
*/

struct lineScroller_t : public cppbuffer_t<const char *>
{
    unsigned int col_width;

    lineScroller_t() : col_width(0)
    { }
    lineScroller_t( unsigned int w ) : col_width(w)
    { }

    void process( const basicString_t& S );

    void setColumnCharWidth( unsigned int _n ) {
        col_width = _n;
    }
};

void strip_utf8_chars( basicString_t & w );

const char * search_path( const char * );

/////////////////////////////////////////////////////////////////////////////
//
//  basicArray_t
//
//    - simple array class intended to work in conjunction with basicString_t
//
struct basicArray_t : public cppbuffer_t<basicString_t>
{
    basicArray_t()
    { }

    ~basicArray_t() 
    { }

    void erase() {
        for ( unsigned int i = 0; i < count(); i++ ) {
            (*this)[i].erase();
        }
        reset(); // reset pushback counter
    }

    basicArray_t& push_back( const basicString_t & );

    // puts all [...] into a string
    basicString_t * implode( const char * glue ="" ); 

    // TODO:
    // - basicArray_t& push_back( const char * ) ;   
    // - basicArray_t& push_back( basicString_t * ); 
    // - basicArray_t& push_back( const basicString_t * ) ;
    // - basicArray_t& push_back( int ); // convert to basicString
    // - basicArray_t& push_back( float ); // convert to basicString
    // 
    // - basicArray_t( const basicArray_t& ); // ctor
    // - operator=(const basicArray_t&)
    // - ***basicString_t& operator()( const char * ); // set & get by associative
    //
    // - basicString_t * searchKey(const char *);
    // - basicString_t * searchVal( const char * );
    //
    // - operator+( basicArray_t& );
    // - operator+=( basicArray_t& );
    //
    // - basicString_t toString(); // returns basicString_t of concatenated array elts
    // - slice( unsigned int, unsigned int );
    // - splice( const basicArray_t& );
    //
    // - append( basicString_t& ); // alias of push_back
    // - append( basicArray_t& ): // impl of operator+=()
};
//
//
//////////////////////////////////////////////////

basicString_t encodeURIComponent( const basicString_t& in );

#endif /* __MISC_H__ */
