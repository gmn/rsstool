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

#ifndef __QUICKSORT_H__
#define __QUICKSORT_H__


template<typename type>
struct quicksortCompareFunc_t
{
    int operator() (type a, type b) ;
};

template<>
struct quicksortCompareFunc_t<const char *>
{
    int operator()( const char * a, const char * b ) {
        return strcmp( a, b ); // this is awesome. I just noticed that since this is templated, string.h doesn't need to be included here
    }
};

template<>
struct quicksortCompareFunc_t<doubleToken_t*>
{
    int operator()( doubleToken_t * a, doubleToken_t * b ) {
        return strcmp( a->A.str, b->A.str );
    }
};


template<>
struct quicksortCompareFunc_t<int>
{
    int operator()( int a, int b ) {
        return a < b ? -1 : a == b ? 0 : 1;
    }
};

template<>
struct quicksortCompareFunc_t<float>
{
    int operator()( float a, float b ) {
        return a < b ? -1 : a == b ? 0 : 1;
    }
};

template<>
struct quicksortCompareFunc_t<double>
{
    int operator()( double a, double b ) {
        return a < b ? -1 : a == b ? 0 : 1;
    }
};

template<>
struct quicksortCompareFunc_t<unsigned int>
{
    int operator()( unsigned int a, unsigned int b ) {
        return a < b ? -1 : a == b ? 0 : 1;
    }
};



/* // left is the index of the leftmost element of the array
   // right is the index of the rightmost element of the array (inclusive)
   //   number of elements in subarray = right-left+1
   function partition(array, 'left', 'right', 'pivotIndex')
      'pivotValue' := array['pivotIndex']
      swap array['pivotIndex'] and array['right']  // Move pivot to end
      'storeIndex' := 'left'
      for 'i' from 'left' to 'right' - 1  // left ≤ i < right
          if array['i'] < 'pivotValue'
              swap array['i'] and array['storeIndex']
              'storeIndex' := 'storeIndex' + 1
      swap array['storeIndex'] and array['right']  // Move pivot to its final place
      return 'storeIndex' */

template<typename type>
static inline void _swap( type * array, int a, int b )
{
    type tmp = array[b];
    array[b] = array[a];
    array[a] = tmp;
}

template<typename type>
static inline int _partition( type * array, int left, int right, int pivot )
{
    static quicksortCompareFunc_t<type> compare;

    type value = array[ pivot ];

    _swap<type>( array, pivot, right );

    int store = left;

    for ( int i = left; i < right; i++ )
    {
        if ( compare( array[i], value ) < 0 ) {
            _swap<type>( array, i, store );
            store = store + 1;
        }
    }

    _swap<type>( array, store, right );

    return store;
}

/* function quicksort(array, 'left', 'right')
 
      // If the list has 2 or more items
      if 'left' < 'right'
 
          // See "Choice of pivot" section below for possible choices
          choose any 'pivotIndex' such that 'left' ≤ 'pivotIndex' ≤ 'right'
 
          // Get lists of bigger and smaller items and final position of pivot
          'pivotNewIndex' := partition(array, 'left', 'right', 'pivotIndex')
 
          // Recursively sort elements smaller than the pivot
          quicksort(array, 'left', 'pivotNewIndex' - 1)
 
          // Recursively sort elements at least as big as the pivot
          quicksort(array, 'pivotNewIndex' + 1, 'right') */

template<typename type>
static inline void _quicksort( type * array, int left, int right )
{
    if ( left >= right )
        return;

    int pivot = left + ( right - left ) / 2;

    pivot = _partition<type>( array, left, right, pivot );

    _quicksort<type>( array, left, pivot - 1 );

    _quicksort<type>( array, pivot + 1, right );
}

// TODO: use insertion sort on small arrays < 20 length

template<typename type>
void quicksort( type * array, int array_len )
{ 
    _quicksort<type>( array, 0, array_len - 1 );
}

#endif
