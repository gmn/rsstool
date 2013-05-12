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

#ifndef __HTML_ENTITIES__
#define __HTML_ENTITIES__

// from: HTML entities print reference -- http://www.html-entities.org/print.php
//
// best possible html entity translation into ascii replacement
//


class HtmlEntities_t
{
    const char * unknown_ent;


public:

    const char * swap_numeric( const char * test ) ;

    const char * swap_literal( const char * test ) ;

    unsigned int longest_entity();
    unsigned int longest_literal();

    HtmlEntities_t() ;

    const char * swap_uri( const char );

}; // HtmlEntities_t


#endif /* __HTML_ENTITIES__ */
