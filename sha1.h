#ifndef __SHA1_H__
#define __SHA1_H__
//-----------------------------------------------------------------------------
//
// Copyright(C) 2012 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
// DESCRIPTION:
//     SHA-1 digest.
//
//-----------------------------------------------------------------------------
#include <stdint.h>

typedef uint8_t sha1_digest_t[20];

struct sha1_context_t {
    uint32_t h0,h1,h2,h3,h4;
    uint32_t nblocks;
    uint8_t buf[64];
    int count;
};

void SHA1_Init( sha1_context_t *context );
void SHA1_Update( sha1_context_t *context, const unsigned char * buf, unsigned int len );
void SHA1_Final( sha1_digest_t digest, sha1_context_t *context );
void SHA1_UpdateInt32( sha1_context_t *context, unsigned int val );

unsigned int SHA1_BlockSum( const void * data, unsigned int length );
const char * SHA1_BlockSumPrintable( const void * data, unsigned int length );

#endif /* __SHA1_H__ */
