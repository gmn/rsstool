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

#ifndef __TYPES_H__
#define __TYPES_H__


#include <inttypes.h>

typedef uint8_t byte;

#include <limits.h>

#ifdef _WIN32
    #define DIR_SEPARATOR '\\'
    #define PATH_SEPARATOR ';'
#else
    #define DIR_SEPARATOR '/'
    #define PATH_SEPARATOR ':'
#endif

#endif /* __TYPES_H__ */

