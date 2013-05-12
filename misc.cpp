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

#include <limits.h>
#include <stdlib.h>
#include <ctype.h> // tolower
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
//#include <SDL/SDL.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/utsname.h>

#include "misc.h"
#include "tokenizer.h"

//#define _VA_BUF_SZ 8192

// these can be redefined for hooking into external text handling systems
#define _printf printf
#define _error error
#define _warn warning

const bool STDIO_ENABLED = 1;


char *va( const char *format, ... ) 
{
	va_list	argptr;
	static char	string[ _VA_BUF_SZ ];
	
	va_start( argptr, format );
	vsnprintf( string, sizeof( string ), format, argptr );
	va_end( argptr );
	
	string[ sizeof( string ) - 1 ] = '\0';
	
	return string;	
}

void error( const char * fmt, ... )
{
    if ( !STDIO_ENABLED )
        return;
    char buffer[ _VA_BUF_SZ ];
    va_list argptr;
    va_start( argptr, fmt );
    vsnprintf( buffer, sizeof( buffer ), fmt, argptr );
    va_end( argptr );
    fprintf(stderr, "error: %s", buffer );
    exit( EXIT_FAILURE );
}

void warning( const char * fmt, ... )
{
    if ( !STDIO_ENABLED )
        return;
    char buffer[ _VA_BUF_SZ ];
    va_list argptr;
    va_start( argptr, fmt );
    vsnprintf( buffer, sizeof( buffer ), fmt, argptr );
    va_end( argptr );
    fprintf(stderr, "warning: %s", buffer );
}


// disabled when compiler _DEBUG not set
void _hidden_Assert( int die_if_false, const char *exp, const char *file, int line ) 
{
	if ( !die_if_false ) 
    {
        fprintf( stderr, "Assert failed with expression: \"%s\" at: %s, line: %d\n", exp, file, line );
		fflush( stderr );

#ifdef _WINDOWS
		__asm {
			int 0x3;
		}
#endif

        _error( "Assert failed with expression: \"%s\" at: %s, line: %d\n", exp, file, line );
	}
}


// using stdio 
unsigned int file_size( const char * path )
{
    FILE *fp = fopen ( path, "r" );
    if ( !fp ) {
        return (unsigned int)-1;
    }
    long a = ftell(fp);
    fseek(fp,0L,SEEK_END);
    long b = ftell(fp);
    fclose(fp);
    return (unsigned int) (b - a);
}

// using sys/stat
unsigned int get_filesize( const char * path )
{
    struct stat ss;
    stat( path, &ss );
    return (unsigned int)ss.st_size;   
}

// looks for both kinds of path seperator
const char * strip_path( const char *s ) 
{
    const char *w = strrchr ( s, '\\' );
    const char *u = strrchr ( s, '/' );

    if ( !w && !u )
        return s;

    if ( w && u )
        return s;

    if ( w && !u ) {
        return (const char *)(w+1);
    }

    if ( !w && u ) {
        return (const char *)(u+1);
    }

    return s;
}

const char * strip_extension( const char *s ) 
{
    static char buf[ 1000 ];

    if ( !s || !s[0] ) {
        buf[0] = 0;
        return buf;
    }

    memset( buf, 0, sizeof(buf) );
    strcpy( buf, s );

    char * ne = strrchr( buf, '.' );
    if ( !ne )
        return buf;

    *ne = '\0';
    return buf;
}

const char * strip_path_and_extension( const char * s ) 
{
    static char buf[ 100 ];

    if ( !s || !s[0] ) {
        buf[0] = 0;
        return buf;
    }

    const char * basename = strip_path( s );
    memset( buf, 0, sizeof(buf) );
    strcpy( buf, basename );

    char * ne = strrchr( buf, '.' );
    if ( !ne )
        return basename;

    *ne = '\0';
    return buf;
}

char * copy_string( const char * s ) 
{
    if ( !s || !s[0] ) 
        return '\0';
    unsigned int len = strlen(s);
    unsigned int bytes = len * sizeof(char) ;
    // checking for null-terminator stops string from growing 1-byte each-time it's copied
    if ( s[len - 1] != '\0' ) 
        ++bytes;
    char * str = (char *) malloc( bytes );
    memset( str, 0, bytes );
    strcpy( str, s );
    return str;
}

/*
====================
 FlipULong

    endian utility
====================
*/
unsigned long FlipULong( unsigned long x ) {
    return ((x>>24)&255) | (((x>>16)&255)<<8) | (((x>>8)&255)<<16) | ((x&255)<<24);
}

/*
====================
 little_endian

    helper function to tell if system is little endian
====================
*/
int is_little_endian( void ) {
    short _short_int = 0x0001;
    return (int)((char*)&_short_int)[0];
}


// strspn = returns pointer to the first character in haystack which is not in whitelist,
//          if whitelist begins with '\0', haystack is returned
const char * strspn_p( const char * haystack, const char * whitelist )
{   
    if ( !haystack || !whitelist || !*haystack )
        return 0;

    if ( !*whitelist && *haystack )
        return haystack;

    const char * p = haystack;
    while ( *p )
    {   
        const char * w = whitelist;
        int found = 0;
        while ( *w )
        {   
            if ( *w++ == *p ) {
                found = 1;
                break;
            }
        }
        if ( !found )
            return p;
        ++p;
    }
    return 0;
}

// strspn = returns pointer to the first character in haystack that is in whitelist.
//          If whitelist is 0, 0 is returned. If whitelist begins with '\0',
//          strcspn_p will try to match that. If end of haystack is hit, and
//          whitelist not found, then terminating '\0' is returned.
const char * strcspn_p( const char * haystack, const char * whitelist )
{   
    if ( !haystack || !whitelist || !*haystack )
        return 0;

    const char * p = haystack;
    while ( *p )
    {  
        const char * w = whitelist;
        while ( *w )
        {  
            if ( *w++ == *p ) {
                return p;
            }
        }
        ++p;
    }

    if ( p != haystack )
        return p;
    return 0;
}



/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  
| 
|       Classes Section 
| 
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */





/////////////////////////////////////////////////////////////////////////////
//
// dirlist_t
//
void dirlist_t::free_listdir( char ** pp )
{   
    char ** save_p = pp;
    while ( *pp )
        free( *pp++ );
    free ( save_p );
}

dirlist_t::~dirlist_t() {
    this->free_listdir( filenames );
}

 
int dirlist_t::_read_dir( const char * dir )
{
    char * fullpath = realpath( dir, 0 );

    DIR * dirp = opendir( dir );
    if ( !dirp ) {
        _warn( "bad dir: \"%s\"\n", dir );
        return 0;
    }

    struct dirent * ent = 0;
    int files_count = 0;

    // count total files in directory, so that we can allocate
    while ( (ent = readdir( dirp )) )
    {
        if ( *ent->d_name != '.' && (ent->d_type == DT_DIR || ent->d_type == DT_REG) )
            ++files_count;
    }
    closedir( dirp );

    unsigned int sz = (files_count+1) * sizeof(char*) ;
    char ** pp = (char**) malloc ( sz );
    memset( pp, 0, sz );

    dirp = opendir( dir );
    int files_got = 0;
    while ( (ent = readdir( dirp )) )
    {
        if ( *ent->d_name != '.' && (ent->d_type == DT_DIR || ent->d_type == DT_REG) )
        {
            // 
            // test against suffix filters (if any), skipping any dirents that do not match any of the filters
            // 
            int match = suffix_count == 0 ? 1 : 0;

            for ( unsigned int cur_sfx = 0; cur_sfx < suffix_count; cur_sfx++ ) 
            {
                int len = strlen( ent->d_name );
                int slen = suffix[cur_sfx].length();

                // long enough to hold it, lets take a look
                if ( len >= slen ) 
                {
                    char * src = &ent->d_name[len-slen];
                    if ( strcasestr( src, suffix[cur_sfx].str ) ) {
                        match = 1; // found a match
                        break;
                    }
                }
            }

            if ( !match )
                continue;

            char buf[2048];
            sprintf( buf, "%s/%s", fullpath, ent->d_name );
            pp[files_got] = copy_string( buf );
            ++total_entries;
            if ( ++files_got >= files_count )
                break;
        }
    }
    closedir( dirp );
    free( fullpath );

    filenames = pp;

    return 1;
}

int dirlist_t::read( const char * _path ) 
{
    if ( !_path )
        _path = path;
    if ( !_path )
        return 0;

    // check if it's already open/been-read. if so, empty it and start over
    if ( total_entries > 0 ) { 
        free_listdir( filenames );
    }

    return _read_dir( _path );
}

void dirlist_t::add_suffix_filter( const char * filter )
{
    basicString_t suf(filter);
    suffix[suffix_count++] = suf;
}

void dirlist_t::sort_alphabetical( void )
{
    unsigned int i, j;
    for ( j = 1; j < total_entries; j++ ) 
    {
        i = j;
        char * tmp = filenames[j];
        while ( i > 0 && strcmp( filenames[i-1], tmp ) > 0 ) {
            filenames[i] = filenames[i-1];
            i--;
        }
        filenames[i] = tmp;
    }
}



//////////////////////////////////////////////////////////////////// 
//
// basicString_t 
//

#define APPEND_MIN_ALLOC_SZ 32

basicString_t::basicString_t( const char * A ) : str(0), len(0), memlen(0) {
    if ( !A || !*A )
        return;
    unsigned int _len = strlen( A );
    if ( _len > 0 ) {
        len = _len;
        memlen = len + 1;
        str = new char[ memlen ];
        ::strncpy( str, A, len );
        str[len] = 0;
    }
}

basicString_t::basicString_t( const basicString_t& t ) : str(0), len(0), memlen(0) {
    if ( t.str ) {
        len = t.len;
#ifdef _DEBUG
        unsigned int _len = strlen( t.str );
        Assert( _len == len );
#endif
        memlen = len + 1;
        str = new char[ memlen ];
        ::strncpy( str, t.str, len );
        str[len] = 0;
    }
}

basicString_t& basicString_t::set( const char * A ) 
{
    if ( !A || !*A ) 
    {
        // since we are setting here, if the thing we are setting to is an empty string, we clip the string
        erase();
    } 
    else 
    {
        len = strlen( A );

        if ( len >= memlen && str ) {
            delete[] str;
            memlen = len + 1;
            str = new char[ memlen ];
        } 
        else if ( !str ) {
            str = new char[len+1];
            memlen = len + 1;
        }
        ::strncpy( str, A, memlen );
        str[len] = 0;
    }
    return *this;
}

basicString_t& basicString_t::set( const basicString_t& ss ) 
{
    this->set( ss.str );
    return *this;
}

basicString_t& basicString_t::operator=( const basicString_t& ss ) 
{
    this->set( ss.str );
    return *this;
}



void basicString_t::Print( const char * fmt ) {
    if ( fmt )
        _printf( fmt, str );
    else
        _printf( "%s", str );
}

basicString_t& basicString_t::strncpy( const char *newstr, unsigned int newlen ) 
{
    if ( !newstr || !*newstr ) {
        erase();
        return *this;
    } 
    else if ( !str )
    {
        str = new char[newlen+1];
        len = newlen;
        memlen = newlen + 1;
        ::strncpy( str, newstr, newlen );
        str[newlen] = 0;
    }
    else if ( newlen >= memlen ) 
    {
        delete[] str;
        str = new char[newlen+1];
        memlen = newlen + 1;
        len = newlen;
        ::strncpy( str, newstr, newlen );
        str[newlen] = 0;
    } 
    else 
    {
        len = newlen;
        ::strncpy( str, newstr, newlen );
        str[newlen] = 0;
    }
    return *this;
}

// Access with array operator causes string to be allocated if there is no string yet. 
//  If array-bound accessed is out of range, it also allocates. 
//  This way a char is always returned.
char& basicString_t::operator[]( unsigned int index ) 
{
    if ( !str ) {
        str = new char[index+1];        
        memlen = index + 1;
        len = index;
        memset( str, 0, sizeof(char) * memlen );
    } 
    // accessing outside of memlen doesn't increase string length
    else if ( index >= memlen ) 
    { 
        memlen = index + 1;
        char * newstr = new char[ memlen ];
        memset( newstr, 0, sizeof(char)*memlen );
        ::strncpy( newstr, str, len );
        delete[] str;
        str = newstr;
    }
    // FIXME: note that accessing an index > len, doesn't set len 
    return str[index];
}

basicString_t& basicString_t::sprintf( const char *fmt, ... )
{
    char buf[ _VA_BUF_SZ ];
    char * buffer = &buf[0];
    va_list argptr;
    va_start( argptr, fmt );
    int len_actual = vsnprintf( buffer, sizeof( buf ), fmt, argptr );
    va_end( argptr );

    if ( len_actual >= _VA_BUF_SZ )
    {
        buffer = (char*) malloc( len_actual+1 );
        va_start( argptr, fmt );
        vsnprintf( buffer, len_actual+1, fmt, argptr );
        va_end( argptr );
        set( buffer );
        free( buffer );
    }
    else
        set( buffer );

    return *this;
}


basicString_t& basicString_t::append( const char * app, unsigned int app_len )
{
    if ( !app || !*app )
        return *this;

    // if app_len is 0, we rely on app to be null-terminated for length
    if ( 0 == app_len ) {
        app_len = strlen( app );
    }

    if ( !str )
    {
        str = new char[app_len+1];
        len = app_len;
        memlen = app_len + 1;
        ::strncpy( str, app, app_len );
        str[app_len] = 0;
    }
    else if ( len+app_len+1 <= memlen ) // will fit in the memory we already have 
    {
        ::strncpy( &str[len], app, app_len );
        str[ len + app_len ] = 0;
        len = len + app_len;
    }
    else 
    {
        // staves off tiny, repeated allocations of 1-byte (which are common in certain applications)
        // 1-byte allocations can throw nasty errors in various stdlib functions
        unsigned int alloc_sz = app_len >= APPEND_MIN_ALLOC_SZ ? app_len : APPEND_MIN_ALLOC_SZ;

        char *tmp = new char[len+alloc_sz+1];
        ::strncpy( tmp, str, len );
        ::strncpy( &tmp[len], app, app_len );
        tmp[ len + app_len ] = 0;
        delete[] str;
        memlen = len + alloc_sz + 1;
        len = len + app_len;
        str = tmp;
    }
    return *this;
}

basicString_t& basicString_t::append( const basicString_t& more )
{
    return append ( more.str );
}

basicString_t& basicString_t::operator+= ( const char * more )
{
    return append( more );
}

basicString_t& basicString_t::operator+= ( const basicString_t& more )
{
    return append( more );
}

basicString_t& basicString_t::operator+= ( const char c )
{
    char tmp[2] = { 0, 0 };
    tmp[0] = c;
    return append( tmp, 1u );
}
basicString_t& basicString_t::operator+= ( const int i )
{
    char _buf[32];
    memset(_buf,0,sizeof(_buf));
    ::sprintf( _buf, "%i", i );
    return append( _buf );
}
basicString_t& basicString_t::operator+= ( const float f )
{
    char buf[32];
    memset(buf,0,sizeof(buf));
    ::sprintf( buf, "%f", f );
    return append( buf );
}
basicString_t& basicString_t::operator+= ( const double d )
{
    char buf[32];
    memset(buf,0,sizeof(buf));
    ::sprintf( buf, "%lf", d );
    return append( buf );
}


// all compare functions only return true|false
bool basicString_t::compare( const char * test )
{
    if ( !str || !*str ) { 
        if ( !test || !*test )
            return true;
        return false;
    } 
    else if ( !test || !*test )
        return false;
    
    return strcmp( str, test ) == 0;
} 

bool basicString_t::compare( const basicString_t& test )
{
    return compare( test.str ); 
}

bool basicString_t::icompare( const char * _str )
{
    if ( !str || !*str ) {
        if ( !_str || !*_str )
            return true;
        return false;
    }
    else if ( !_str || !*_str )
        return false;

    char * s = const_cast<char *>( _str );
    char * n = const_cast<char *>( str );

    do
    {
        if ( char_tolower(*s) != char_tolower(*n) )
        {
            return false;
        }
        ++s; ++n; 
    }
    while ( *s && *n );

    return (char_tolower(*s) - char_tolower(*n)) == 0;
}

// note: duplicate of above to satisfy compiler. Any changes need to be made to both!!!
bool basicString_t::icompare( const char * _str ) const
{
    if ( !str || !*str ) {
        if ( !_str || !*_str )
            return true;
        return false;
    }
    else if ( !_str || !*_str )
        return false;

    char * s = const_cast<char *>( _str );
    char * n = const_cast<char *>( str );

    do
    {
        if ( char_tolower(*s) != char_tolower(*n) )
        {
            return false;
        }
        ++s; ++n; 
    }
    while ( *s && *n );

    return (char_tolower(*s) - char_tolower(*n)) == 0;

}

bool basicString_t::icompare( const basicString_t& ref )
{
    return this->icompare( ref.str );
}

bool basicString_t::operator==( const char * test )
{
    return compare( test );
}

bool basicString_t::operator==( const basicString_t& test )
{
    return compare( test );
}

bool basicString_t::operator!=( const char * test )
{
    return !compare( test );
}

bool basicString_t::operator!=( const basicString_t& test )
{
    return !compare( test );
}

// pre-allocate or explicitly set the internal memory; will truncate string if 
//  new value is less than len, otherwise, will pad with \0
void basicString_t::setMem( unsigned int newlen )
{
    if ( newlen == 0 || newlen == 1 ) {
        if ( str )
            delete[] str;
        memlen = len = 0;
    }
    else if ( str )
    {
        if ( newlen < len ) { // truncate
            char * _str = new char[ newlen ];
            ::strncpy( _str, str, newlen-1 );
            _str[ newlen - 1 ] = '\0';
            len = newlen - 1;
            memlen = newlen;
            delete[] str;
            str = _str;
        } else {
            char * _str = new char[ newlen ];
            memset( _str, 0, newlen );
            ::strncpy( _str, str, len );
            memlen = newlen;
            delete[] str;
            str = _str;
        }
    }
    else
    {
        str = new char[newlen];
        memlen = newlen;
        memset( str, 0, memlen );
    }
}

// trims in place, returns self
basicString_t& basicString_t::trim()
{
    if ( !str || !*str )
        return *this;

    char * p = str;
    while ( *p != '\0' && (*p==' '||*p=='\t'||*p=='\n'||*p=='\r') )
        ++p;

    char * e = &str[len];

    int set = 0;
    do {  
        long long end = reinterpret_cast<long long>(e);
        long long beg = reinterpret_cast<long long>(p);
        if ( end <= beg )
            break;
        if ( *(e-1)==' '||*(e-1)=='\t'||*(e-1)=='\n'||*(e-1)=='\r')
            --e, ++set;
        else
            break;
    } while(1);

    if ( set ) {
        *e = '\0';
    }

    // if beginning of string didn't change we don't need to reallocate
    if ( p == str )
        return *this; 

    len = strlen(p);
    memlen = len + 1;
    char * str2 = new char[memlen];
    strcpy( str2, p );
    delete[] str;
    str = str2;
    return *this;
}

const char * basicString_t::strstr( const char * needle ) 
{
    if ( !str || !*str || !needle || !*needle )
        return 0;
    return ::strstr( str, needle );
}

const char * basicString_t::strstr( const basicString_t& ref )
{
    return this->strstr( ref.str );
}

// strcasestr is a gnu extension, so we write our own
const char * basicString_t::stristr( const char * arg )
{
    if ( !str || !*str ) {
        return 0;
    }
    else if ( !arg || !*arg )
        return 0;

    // length of fragment & haystack
    const unsigned int alen = strlen( arg );
    const unsigned int slen = strlen( str );
    // starting char of fragment
    int a_start = char_tolower(*arg);
    // pointer to haystack
    char * s = const_cast<char *>( str );

    unsigned int i;
    char * p = s;
    
    while ( *s && s-str <= slen-alen )
    {
        if ( char_tolower(*s) != a_start )
            ++s;
        else
        {
            p = s;
            i = 0;
            char * a = const_cast<char *>( arg );
            do
            {
                if ( char_tolower(*p) == char_tolower(*a) )
                    ++i;

                if ( i == alen )
                    return s;

                ++p; ++a;
            } 
            while( *p && *a );

            if ( i == alen )
                return s;
            ++s;
        }
    }
    return 0;
}

// alias
const char * basicString_t::strcasestr( const char * arg )
{
    return this->stristr( arg );
}

const char * basicString_t::stristr( const basicString_t& ref )
{
    return this->stristr( ref.str );
} 

const char * basicString_t::strcasestr( const basicString_t& ref )
{
    return this->stristr( ref.str );
} 


basicString_t basicString_t::operator+( const char * c_str )
{
    basicString_t internal( this->str );
    internal.append( c_str );
    return internal; // temporary object
}

basicString_t basicString_t::operator+( const basicString_t & ref )
{
    basicString_t internal( str );
    internal.append( ref );
    return internal; // temporary object
}


basicString_t basicString_t::substr( unsigned int start, unsigned int length )
{
    if ( !str || !*str )
        return basicString_t();


    if ( start < len )
    {
        unsigned int maxlen_can_be_copied = len - start;

        if ( length > maxlen_can_be_copied )
            length = maxlen_can_be_copied; 

        basicString_t A;
        return A.strncpy( &str[start], length );
    }

    return basicString_t();
}


basicArray_t * basicString_t::explode( const char * sep, int stripcom )
{
    if ( !str || !*str || !length() )
        return 0;

    basicArray_t * A = new basicArray_t;
    
    Tokenizer_t tokens( str, length() );
    tokens.setStripComments( stripcom ); // defaults to off
    tokens.tokenize( sep ); // can be null, if null deals in whitespace

    Token_t * tok = tokens.getHead();
    while ( tok ) {
        A->push_back( *tok );
        tok = tok->next;
    }
    
    return A;
}

// replaces inline, returning self
basicString_t& basicString_t::replace( const char * find, const char * replace )
{
    // we allow replace to be null, signifying remove instances of find
    if ( !find || !*find || !len ) 
        return *this;

    basicString_t res;
    const char * p = str;
    const char * s = str;
    while ( *p && (p = ::strstr( p, find )) )
    {
        // copyin everything upto match
        while ( s != p ) {
            char c = *s++;
            res += c;
        }

        // copyin replace
        if ( replace && *replace )
            res += replace;

        // move p past match
        s = find;
        while ( *s && *p && *p == *s ) {
            ++p;
            ++s;
        }
        s = p;
    }

    if ( s != str ) {
        set( res += s );
    }

    return *this;
}
//
//
//
/////////////////////////////////////////////////////////////////////////////


const char * file_exists( const char * file )
{
    // won't get allocated, unless function is called 
    static basicString_t internal; 

    struct stat ss;
    if ( -1 == stat( file, &ss ) )
        return 0;

    char * path = realpath( file, NULL );
    if ( !path )
        return 0;

    internal = path;
    free( path );
    return internal.str;
}

    
// if len = 0, data assumed to be null-terminated.
// puts data into a file named fname, and returns bytes written
int file_put_contents( const char *fname, const char *data, unsigned int len )
{
    FILE *fp = fopen( fname, "w" );
    if ( !fp )
        return -1;
    unsigned int length = len ? len : strlen( data );
    unsigned int got = fwrite( data, 1, length, fp ); 
    fclose( fp );
    return got;
}

int file_get_contents( const char *fname, basicString_t& ref )
{
    if ( !fname || !*fname )
        return 0;

    FILE *fp = fopen( fname, "r" );
    if ( !fp )
        return -1;

    unsigned int read = 0, total = 0;
    char buf[ 1000 ];
    while ( (read = fread( buf, 1, 1000, fp)) > 0 )
    {
        ref.append( buf, read );
        total += read;
    }

    fclose( fp );
    
    return (signed)total;
}


int make_dir( const char *path, int mode )
{
    return mkdir ( path, mode );
}

const char * system_id()
{
    static struct utsname buf;
    uname( &buf );
    return buf.sysname;
}



/////////////////////////// HTML TAG STRIPPER ///////////////////////////////
//

// sort alphabetically similar by longest string first, so that <body matches before <b for instance
const char * w3tags[] = { "<!--", "<!DOCTYPE", 
    "<acronym", "<address", "<article", "<applet", "<aside", "<audio", "<area", "<abbr", "<a", 
    "<blockquote", "<basefont", "<button", "<base", "<body", "<bdi", "<bdo", "<big", "<br", "<b", 
    "<colgroup", "<caption", "<command", "<canvas", "<center", "<cite", "<code", "<col", 
    "<datalist", "<dd", "<del", "<details", "<dfn", "<dialog", "<dir", "<div", "<dl", "<dt", 
    "<embed", "<em", 
    "<figcaption", "<fieldset", "<frameset", "<figure", "<footer", "<frame", "<form", "<font", 
    "<header", "<hgroup", "<head", "<html", "<hr", "<h1", "<h2", "<h3", "<h4", "<h5", "<h6", 
    "<iframe", "<input", "<ins", "<img", "<i", 
    "<keygen", "<kbd", 
    "<legend", "<label", "<link", "<li", 
    "<meter", "<mark", "<menu", "<meta", "<map", 
    "<noscript", "<noframes", "<nav", 
    "<object", "<ol", "<optgroup", "<option", "<output", 
    "<progress", "<param", "<pre", "<p", 
    "<q", 
    "<ruby", "<rp", "<rt", 
    "<summary", "<section", "<source", "<select", "<script", "<strike", "<strong", "<small", "<style", "<span", "<samp", "<sub", "<sup", "<s", 
    "<textarea", "<table", "<tbody", "<tfoot", "<thead", "<title", "<track", "<time", "<th", "<tr", "<tt", "<td", 
    "<ul", "<u", 
    "<video", "<var", "<wbr", 
    0 }; 

HtmlTagStripper::HtmlTagStripper() : tag(), lastTag(), line_buffer()
{
    line_buffer.setMem( 2048 );
    lastTag.setMem( 2048 );
    lastTag.contents.setMem( 2048 );
    tag.setMem( 2048 );
    tag.contents.setMem( 2048 );
}

int HtmlTagStripper::classifyTag( tag_t& in )
{
    // guarantee at least 2 characters
    if ( in.length() < 2 )
        return 0;

    // trim it
    in.trim();

    // again
    if ( in.length() < 2 )
        return 0;

    char close[20];

    const char ** p = w3tags;

    in.contents.erase();

    do {
        // found a match
        if ( in.stristr( *p ) ) 
        {
            // set contents
            in.contents = in.str;

            // simplify tag
            in = *p;

            break; // we're done
        }

        // try the closing equiv
        else 
        {
            strcpy( &close[1], *p );
            close[0] = '<';
            close[1] = '/';

            if ( in.stristr( close ) )
            {
                in.contents = in.str;
                in = close;
                break; 
            }
        }
    } 
    while ( *++p );
    
    return in.contents.length() > 0;
}

void HtmlTagStripper::strip( const basicString_t& in, basicString_t& out )
{
    out.erase();

    // internals
    tag.erase();
    entity.erase(); // & ampersand ; semi-colon html special entities
    lastTag.erase();
    line_buffer.erase();
    
    enum ent_type_t {
        ENT_NONE,
        ENT_LITERAL,
        ENT_NUMERIC
    } ent_type = ENT_NONE;


    char tmp[4] = {0,0,0,0};

    unsigned int i = 0;

    while ( i < in.length() )
    {
        const char c = in.str[i];

        // normal text; not inside a tag, not inside an entity
        if ( c != '<' && !tag.length() && c != '&' && !entity.length() ) {
            if ( c != '\t' && c != '\n' )
                line_buffer += c;
            else if ( line_buffer.str && line_buffer.length() > 1 && line_buffer.str[line_buffer.length()-1] != '\n' )
                line_buffer += ' '; // newlines & tabs trade for spaces

            ent_type = ENT_NONE;
        }

        // first char after '<' has to be: a-zA-Z/!
        else if ( tag.length() == 1 )
        {
            // def looks like start of a tag
            if ( (c >= 'a' && c <= 'z') || 
                 (c >= 'A' && c <= 'Z') || 
                  c == '/' || c == '!' )
            {
                tmp[0] = c;
                tag.append(tmp,1u);
            }
            // not a tag
            else
            {
                line_buffer += tag.str[0];
                line_buffer += c ;
                tag.erase();
            }
        }

        else if ( entity.length() == 1 )
        {
            // looks like start of entity
            if ( ( c >= 'A' && c <= 'Z' ) ||
                ( c >= 'a' && c <= 'z' ) )
                ent_type = ENT_LITERAL;
            else if ( c == '#' ) 
                ent_type = ENT_NUMERIC;

            // not an entity
            else
            {
                line_buffer += entity;
                line_buffer += c ;
                entity.erase();
            }

            if ( ent_type ) {
                tmp[0] = c;
                entity.append(tmp,1u);
            }
        }
        else if ( entity.length() )
        {
            // longest ent: 7   - &#8201;
            // longest lit: 10  - &literal;
            tmp[0] = c;
            entity.append(tmp,1u);

            // too long w/o ';'. must not be entity
            if ( entity.length() > 10 ) {
                line_buffer += entity;
                line_buffer += c ;
                entity.erase();
                ent_type = ENT_NONE;
            }

            else if ( c == ';' )
            {
                if ( ent_type == ENT_LITERAL && entity.length() <= 10 ) {
                    line_buffer += html_ent.swap_literal( entity.str );
                } else if ( ent_type == ENT_NUMERIC && entity.length() <= 7 ) {
                    line_buffer += html_ent.swap_numeric( entity.str );
                }
                entity.erase();
                ent_type = ENT_NONE;
            }
        }

        // inside of a legit tag 
        else if ( tag.length() )
        {
            tmp[0] = c;
            tag.append(tmp,1u);

            // close tag, and identify it
            if ( c == '>' && classifyTag( tag ) )
            {
                // have a tag, add line_buffer

                // closing link, get href= and presentation text
                if ( tag.icompare( "</a" ) )
                {
                    // name the url
                    out += line_buffer;
                    line_buffer.erase();

                    // get the href from the opening tag
                    const char * href = lastTag.contents.stristr( "href=" );
                    if ( href ) 
                    {
                        out += "-[";

                        const char * rov = href + 5; 

                        // find first quote
                        while ( *rov != '"' && *rov != '\'' && rov - lastTag.contents.str < lastTag.contents.length() )
                            ++rov; 

                        // next char after '"'
                        const char * end = ++rov; 

                        // find last quote
                        while ( *end != '"' && *end != '\'' && end - lastTag.contents.str < lastTag.contents.length() ) 
                            ++end;

                        line_buffer.strncpy( rov, (end-rov) * sizeof(char) );
                        out += line_buffer += "]";
                    }

                    line_buffer.erase();
                    lastTag.erase();
                }
                else if ( tag.icompare( "</script" ) )                          // never print
                    line_buffer.erase(); 
                else if ( tag.icompare("</b") || tag.icompare("</strong") ||    // BOLD
                    tag.icompare("</h1") || tag.icompare("</h2") || 
                    tag.icompare("</h3") || tag.icompare("</h4") || 
                    tag.icompare("</h5") || tag.icompare("</h6") ) 
                {
                    str_toupper( line_buffer.str );
                    out += line_buffer;
                    line_buffer.erase();
                }
                else if ( tag.icompare("</i") )                                 // ITALIC
                {
                    str_replace( ' ', '_', line_buffer.str );
                    if ( line_buffer.str && line_buffer.str[0] != ' ' )
                        out += '_' ;
                    out += line_buffer;
                    if ( line_buffer.str && line_buffer.length() > 1 && line_buffer.str[line_buffer.length()-1] != ' ' )
                        out += '_' ;
                    line_buffer.erase();
                }
                else
                {
                    out += line_buffer;
                    line_buffer.erase();
                }


                // do tag specific stuff here
                if ( tag.icompare( "<br" ) )
                    out += '\n' ;
                else if ( tag.icompare("<p") || tag.icompare("<body") ) 
                    out += '\n' ;
                else if ( tag.icompare("</p") || tag.icompare("</body") ) 
                    out += '\n' ;

                // save '<a'
                else if ( tag.icompare( "<a" ) )
                    lastTag = tag;

                tag.erase(); // empties tag and turns off the boolean signal that we are in a tag
            }
        }
        else if ( c == '<' ) // c is a '<', but not necessarily a tag yet, but we start recording here
        {
            tmp[0] = c;
            tag.append(tmp);
        }
        else // c is '&' possibly start of entity
        {
            tmp[0] = c;
            entity.append(tmp);
        }

        i++;
    }

    // line left in line_buffer, we want it 
    if ( line_buffer.length() ) 
        out += line_buffer;

    out.trim();
}
//
/////////////////////// end HTML TAG STRIPPER ///////////////////////////////

// takes a string and marks line beginnings as an array of pointers, that's it
void lineScroller_t::process( const basicString_t& S )
{
    if ( S.length() == 0 || col_width == 0 )
        return;

    reset(); // make reusable 

    // mark beginning 
    push_back( &S.str[0] );

    unsigned int i = 0;
    while ( i < S.length() )
    {
        if ( i == S.length()-1 )  // last char
            break;

        // check for newline
        if ( S.str[i] == '\n' )
        {
            push_back( &S.str[++i] );
        }

        // try to find a full line 
        else
        {
            unsigned int nl = i + col_width;
            while ( i < nl && i < S.length() )
            {
                if ( S.str[i] == '\n' )
                    break;
                else
                    ++i;
            }

            if ( i == nl && i < S.length() ) {
                push_back( &S.str[i] );
            }
        }
    }
}

// brutish stripping, on source string
void strip_utf8_chars( basicString_t & w )
{
    basicString_t o;
    o.setMem( w.length() + 10 );
    unsigned int j = 0;
    for ( unsigned int i = 0; i < w.length(); i++ ) {
        if ( ((unsigned char)w.str[i]) < 128 )
            o.str[j++] = w.str[i];
    }
    o.str[j] = '\0';
    o.len = strlen( o.str );
    w = o;
}

// search path for various executable/binary, 
//  if found, return fullpath; null if not found
const char * search_path( const char * bin )
{
    basicString_t path = getenv( "PATH" );
    basicArray_t * ap = path.explode( ":" );

    const char * found;

    unsigned int i;
    for ( i = 0; i < ap->count(); i++ )
    {  
        basicString_t& maybe = (*ap)[i];
        if ( maybe.last() != '/' )
            maybe += '/';

        maybe += bin;

        if ( (found=file_exists(maybe.str)) ) {
            delete ap;
            return found;
        }
    }

    delete ap;
    return 0;
}



/////////////////////////////////////////////////////////////////////////////
//
//  basicArray_t
//
basicArray_t& basicArray_t::push_back( const basicString_t& ref )
{
    cppbuffer_t<basicString_t>::push_back( ref ); // returns void
    return *this;
}


// puts all [...] into a string
basicString_t * basicArray_t::implode( const char * glue ) 
{
    basicString_t * line = new basicString_t;
    if ( 0 == count() )
        return line;

    for ( unsigned int i = 0; i < count(); i++ )
    { 
        line->append( (*this)[i] );
        if ( glue )
            line->append( glue );
    }
    return line;
}

//
/////////////////////////////////////////////////////////////////////////////



// mimics the javascript version
// dont encode _.-*()'!a-zA-Z0-9
basicString_t encodeURIComponent( const basicString_t& in )
{
    HtmlEntities_t html;
    basicString_t out;

    const char * p = in.str;
    while ( *p )
    {
        const char c = *p;

        if ( (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ) 
            out += c;
        else
        {
            switch( c )
            {
            case '_':
            case '.':
            case '-':
            case '*':
            case '(':
            case ')':
            case '\'':
            case '!':
                out += c;
                break;
            default:
                out += html.swap_uri(c);
                break;
            }
        }

        ++p;
    }

    return out;
}

