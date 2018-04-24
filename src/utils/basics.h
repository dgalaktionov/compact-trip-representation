
/* basics.h
 * Copyright (C) 2011, Antonio Fariña all rights reserved.
 * Original version from pizza-chili code.
 * Modified by Antonio Fariña in 2010.
 *
 * basics.h: Basic bitwise operations + bitVector manipulation, and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

 // Basics
 
#ifndef BASICSINCLUDED
#define BASICSINCLUDED

#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <utils/libcdsBasics.h>

using namespace cds_utils;

void bitzero (uint *e, size_t p, uint len);

inline void bitset(uint * e, size_t p) {
    e[p/W] |= (1<<(p%W));
}


// bits needed to represent a number between 0 and n
uint bits (size_t n);
// returns e[p..p+len-1], assuming len <= W
uint bitread (uint *e, size_t p, size_t len);
// writes e[p..p+len-1] = s, assuming len <= W
void bitwrite (uint *e, size_t p, size_t len, uint s);
// writes e[p..p+len-1] = 0, no assumption on len        

/** ulongs required to represent n integers of e bits each */
inline size_t ulong_len(const uint e, const size_t n) {
    return (((unsigned long long)e)*n+WW-1)/WW; //+((unsigned long long)e*n%W>0));
}

size_t bitread64 (size_t *e, size_t p, size_t len);
// writes e[p..p+len-1] = s, assuming len <= W
void bitwrite64 (size_t *e, size_t p, size_t len, size_t s);
// writes e[p..p+len-1] = 0, no assumption on len  

#endif

#if 0
  // Memory management
/*
#define malloc(n) Malloc(n)
#define free(p) Free(p)
#define realloc(p,n) Realloc(p,n)

void *Malloc (size_t n);
void Free (void *p);
void *Realloc (void *p, size_t n);
*/
  // Data types

#ifndef byte
	#define byte unsigned char
#endif 

//typedef unsigned char byte;
// typedef unsigned int uint;

//typedef int bool;
//#define true 1
//#define false 0

#define max(x,y) ((x)>(y)?(x):(y))
#define min(x,y) ((x)<(y)?(x):(y))

  // Bitstream management

//#define W (8*sizeof(uint))
//#define W (32)

		/** mask for obtaining the first 5 bits */
    const uint mask31 = 0x0000001F;

    /** number of bits in a uint */
    const uint W = 32;
    /** W-1 */
    const uint Wminusone = 31;

    /** 2W*/
    const uint WW = 64;
    
    /** 4W*/
    const uint WWW = 128;    

    /** number of bits per uchar */
    const uint bitsM = 8;

    /** number of bytes per uint */
    const uint BW = 4;

    /** number of different uchar values 0..255 */
    const uint size_uchar = 256;

	// bits needed to represent a number between 0 and n
uint bits (size_t n);
        // returns e[p..p+len-1], assuming len <= W
uint bitread (uint *e, size_t p, size_t len);
        // writes e[p..p+len-1] = s, assuming len <= W
void bitwrite (uint *e, size_t p, size_t len, uint s);
    // writes e[p..p+len-1] = 0, no assumption on len        

void bitzero (uint *e, size_t p, uint len);

	// reads bit p from e
	//#define bitget(e,p) (((e)[(p)/W] >> ((p)%W)) & 1)
    /** reads bit p from e */
    #define bitget(e,p) ((((e)[(p)/W] >> ((p)%W))) & 1)	

    /** sets bit p in e */
    inline void bitset(uint * e, size_t p) {
        e[p/W] |= (1<<(p%W));
    }

    /** cleans bit p in e */
    inline void bitclean(uint * e, size_t p) {
        e[p/W] &= ~(1<<(p%W));
    }
    


   /** uints required to represent n integers of e bits each */
    inline size_t uint_len(const uint e, const size_t n) {
        return (((unsigned long long)e)*n+W-1)/W; //+((unsigned long long)e*n%W>0));
    }
    
/******************************************************************************/
/** to work with 64bit-values, only 64bit code (non portable code!!!!)        */
/******************************************************************************/
 

    /** ulongs required to represent n integers of e bits each */
    inline size_t ulong_len(const uint e, const size_t n) {
        return (((unsigned long long)e)*n+WW-1)/WW; //+((unsigned long long)e*n%W>0));
    }

size_t bitread64 (size_t *e, size_t p, size_t len);
        // writes e[p..p+len-1] = s, assuming len <= W
void bitwrite64 (size_t *e, size_t p, size_t len, size_t s);
    // writes e[p..p+len-1] = 0, no assumption on len  


#endif
