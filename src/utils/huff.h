
/* huff.h
 * Copyright (C) 2011, Antonio Fariña, all rights reserved.
 * Based on a previous implementation from Gonzalo Navarro (compression
 * con Psi from CSA, using Huffman+RLE compression)
 *
 * huff.h: Implements Canonical Huffman.
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
 
// implements canonical Huffman 

#ifndef HUFFINCLUDED2
#define HUFFINCLUDED2

#include "basics.h"
#define SORTED 1
#define UNSORTED 0

typedef struct
   { uint max,lim;   // maximum symbol (0..max), same excluding zero freqs
     uint depth; // max symbol length
     union
       { uint *spos; // symbol positions after sorting by decr freq (enc)
	 uint *symb; // symbols sorted by freq (dec)
       } s;
     uint *num;  // first pos of each length (dec), number of each length (enc)
     uint *fst;  // first code (numeric) of each length (dec)
     size_t total; // total length to achieve, in bits
   } THuff;

	// Creates Huffman encoder given symbols 0..lim with frequencies 
	// freq[i], ready for compression
	// sorted --> are the symbols already sorted ?

THuff createHuff (uint *freq, uint lim, uint sorted);

	// Encodes symb using H, over stream[ptr...lim] (ptr and lim are
	// bit positions of stream). Returns the new ptr.
	
size_t encodeHuff (THuff H, uint symb, uint *stream, size_t ptr);

	// Decodes *symb using H, over stream[ptr...lim] (ptr and lim are
	// bit positions of stream). Returns the new ptr.
	
size_t decodeHuff (THuff *H, uint *symb, uint *stream, size_t ptr);

	//Prepares a Huffman tree for decoding (changes in spos & symb)
	
void prepareToDecode(THuff *H);

	// Writes H in file f
	
void saveHuff (THuff H, FILE *f);

	// Size of H written on file
	
uint sizeHuffDisk (THuff H);

	//Size of H in memory
uint sizeHuff (THuff H);
	
	// Frees H
	
void freeHuff (THuff H);

	// Loads H from file f, prepared for encoding or decoding depending
	// on enc
	
THuff loadHuff (FILE *f, int enc);

//Decodes a code starting in position ptr from stream. Returns the ranking in the
//vector of symbols.

#define decodeNormalHuffMacro(H, symbol, stream, ptr) \
   { uint pos; \
     int d; \
     pos = 0; \
     d = 0; \
     while (pos < H->fst[d]) \
        { pos = (pos << 1) | bitget(stream,ptr); \
          ptr++; d++; \
        } \
    symbol = (H->s.symb[ H->num[d] + pos - H->fst[d] ]); \
   }


#endif

