/* psiHuffmanRLE.h
 * Copyright (C) 2011, Antonio Fariña and Eduardo Rodriguez, all rights reserved.
 * Improved representation based on a previous version from Gonzalo Navarro's.
 *
 * psiHuffmanRLE.h: Compressed Representation for Psi in CSA.
 *   It uses Huffman+RLE to encode:
 *     1-runs, short-values(+), negative values(-) and long values(+)
 *
 * More details in:
 * Antonio Fariña, Nieves Brisaboa, Gonzalo Navarro, Francisco Claude, Ángeles Places, 
 * and Eduardo Rodríguez. Word-based Self-Indexes for Natural Language Text. ACM 
 * Transactions on Information Systems (TOIS), 2012. 
 * http://vios.dc.fi.udc.es/indexing/wsi/publications.html
 * http://www.dcc.uchile.cl/~gnavarro/ps/tois11.pdf	   
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
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <assert.h>

#include "../utils/huff.h"

/*

Compressing PSI with Huffman over gap-encoded values and RLE for the 1-runs.

Assuming that the maximum length of a run is R-1 (R is the sampling step in PSI)
We use a Huffman tree built over N symbols: Huffman codes are assigned to 
 the following 4 groups of gaps:

G1: Frequent "short positive values": gap values between 2 and 
    N - 32 <for negative> - 32 <for large positive> - (R -1)
G2: R-1 Huffman codes are enought to represent RUNS and their length.
G3: 32 Huffman codes are used to represent negative values. The 32 codes are needed
    not only to mark that they are negative values $V$, but to indicate also 
    the number of bits needed to represent abs($V$). 
G4: Similarly to G3. 32 Huffman codes are used to represent large positive numbers, 
	that is, numbers > those in G1.

From those groups (after decoding a huffman code)
 - values from G1 are directly obtained.
 - values from G2 are converted into a RUN of the len specified by the Huffman code. 
 - values from G3 are obtained by reading the K-bit binary representation of the number
   that follows the decoded codeword. (the Huffman codes especifies that it is a
   negative number, whose abs_value requires K bits (in binary).
 - values from G4 are obtained as those in G3, yet now re binary representation 
   includes the number itself rather than "abs(number)".
*/

// ESTRUCTURA DE PSI COMPRIMIDA
typedef struct {
	unsigned int T;					// Periodo de muestreo de PSI
	THuff diffsHT;					// Arbol de Huffman (codifica stream)
	unsigned int nS;				// Numero de simbolos para Huffman
	
/**/	//unsigned int numberOfSamples;
	size_t numberOfSamples;

	unsigned int sampleSize;		// Bits que ocupa cada mostra
	unsigned int *samples;			// Vector de mostras
	
	unsigned int pointerSize;		// Bits que ocupa cada punteiro	
/**/	size_t *samplePointers;	// Punteiros das mostras a stream

/**/	size_t streamSize;		// Bits que ocupa stream		
	unsigned int *stream;			// Secuencia Huffman + RLE

/**/	size_t totalMem;			// the size in bytes used;
} HuffmanCompressedPsi;


// PROTOTIPOS DE FUNCIÓNS

//	Crea as estructuras de Psi comprimida:
//	
//	Psi: Funcion Psi original
//	psiSize: Numero de elementos de Psi
//	T: Periodo de muestreo en Psi
//	nS: Numero de simbolos que se utilizaran no arbol de Huffman
//
//	Devolve unha estructura CompressedPSI que representa a Psi comprimida
HuffmanCompressedPsi huffmanCompressPsi(unsigned int *Psi, size_t psiSize, unsigned int T, unsigned int nS);

//	Obtén un valor de Psi
//
//	cPsi: A estructura que representa a Psi comprimida
//	position: A posicion da que queremos obter o valor de Psi
unsigned int getHuffmanPsiValue(HuffmanCompressedPsi *cPsi, size_t position);

void getHuffmanPsiValueBuffer(HuffmanCompressedPsi *cPsi, uint *buffer, size_t ini, size_t end);

void storeHuffmanCompressedPsi(HuffmanCompressedPsi *compressedPsi, char *filename);
HuffmanCompressedPsi loadHuffmanCompressedPsi(char *filename);

//frees the memory used.	
void destroyHuffmanCompressedPsi(HuffmanCompressedPsi *compressedPsi);

//for Nieves' statistics
unsigned long  getHuffmanPsiSizeBitsUptoPosition(HuffmanCompressedPsi *cPsi, size_t position);
	
	
