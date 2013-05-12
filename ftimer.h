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

#ifndef __FTIMER_H__
#define __FTIMER_H__

#include <sys/time.h>

// 3 kinds of timer: utimer_t, mtimer_t, stimer_t


// TODO:
//  invariably there's probably a better, more portable way to write these across Un*xes
//
inline long long int microseconds( void ) {
    struct timeval tv;
    gettimeofday( &tv, 0 );
    return ((long long int) tv.tv_sec)*1000000 + (long long int) tv.tv_usec;
}

inline long int milliseconds( void ) {
    struct timeval tv;
    gettimeofday( &tv, 0 );
    return ((long int) tv.tv_sec)*1000 + (long int)(tv.tv_usec/1000);
}

inline long int seconds( void ) {
    struct timeval tv;
    gettimeofday( &tv, 0 );
    return (long int) tv.tv_sec;
}


//==================================================================
//   ftimer_t
//==================================================================

// abstract template
template <typename type>
struct ftimer_t
{
    int flags;
    type _start;
    type length;

    ftimer_t() : flags(0),_start(0),length(0) 
    { }

    ftimer_t( type T ) {
        set( T );
    }

    ftimer_t( const ftimer_t<type>& T ) {
        set( T );
    }
    
    virtual ~ftimer_t()
    { }

    void set( ftimer_t const& t ) {
		flags = t.flags;
		_start = t._start;
		length = t.length;
    }
    void operator=( const ftimer_t& T ) { set( T ); }


    void stop() {
        flags = _start = length = 0;
    }
    void reset() { 
        flags = _start = length = 0;
    }

    virtual int check() = 0;
    virtual int timeup() = 0;
    virtual void start() = 0;
    virtual void set( type len ) = 0;
    virtual void set( type len, int flag ) = 0;
	virtual type delta( void )  = 0;
    virtual void increment( type ) = 0;

    type time( void ) { return delta(); }

	double ratio( void ) {
		return (double) delta() / (double) length;
	}

};



#define type long long int
struct utimer_t : public ftimer_t<long long int>
{
    int check() {
        return microseconds() - _start > length;
    }
    int timeup() {
        return microseconds() - _start > length;
    }
    void start() { 
        length = 0;
        _start = microseconds();
        flags = 1;
    }

    void set( type len =0 ) {
        length = len;
        _start = microseconds();
        flags = 1;
    }

    void set( type len, int flag ) {
        length = len;
        _start = microseconds();
        flags = flag;
    }

	type delta( void ) {
		return microseconds() - _start;
    }

    void increment( type inc ) {
        type left = length - delta();
        if ( left < 0 ) left = 0;
        _start = microseconds();
        length = left + inc;
        if ( 0 == flags ) flags = 1;
    }
};
#undef type



#define type long int
struct mtimer_t : public ftimer_t<long int>
{
    int check() {
        return milliseconds() - _start > length;
    }
    int timeup() {
        return milliseconds() - _start > length;
    }
    void start() { 
        length = 0;
        _start = milliseconds();
        flags = 1;
    }

    void set( type len =0 ) {
        length = len;
        _start = milliseconds();
        flags = 1;
    }

    void set( type len, int flag ) {
        length = len;
        _start = milliseconds();
        flags = flag;
    }

	type delta( void ) {
		return milliseconds() - _start;
    }

    void increment( type inc ) {
        type left = length - delta();
        if ( left < 0 ) left = 0;
        _start = milliseconds();
        length = left + inc;
        if ( 0 == flags ) flags = 1;
    }

};
#undef type



#define type long int
struct stimer_t : public ftimer_t<long int>
{
    int check() {
        return seconds() - _start > length;
    }
    int timeup() {
        return seconds() - _start > length;
    }
    void start() { 
        length = 0;
        _start = seconds();
        flags = 1;
    }

    void set( type len =0 ) {
        length = len;
        _start = seconds();
        flags = 1;
    }

    void set( type len, int flag ) {
        length = len;
        _start = seconds();
        flags = flag;
    }

	type delta( void ) {
		return seconds() - _start;
    }

    void increment( type inc ) {
        type left = length - delta();
        if ( left < 0 ) left = 0;
        _start = seconds();
        length = left + inc;
        if ( 0 == flags ) flags = 1;
    }
};
#undef type


#endif /* __FTIMER_H__ */
