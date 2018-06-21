/* icsa.h
 * Copyright (C) 2011, Antonio Fariña and Eduardo Rodriguez, all rights reserved.
 *
 * icsa.h: Definition of the functions of the interface "../intIndex/interfaceIntIndex.h"
 *   that permits to represent a sequence of uint32 integers with an iCSA: 
 *   An integer-oriented Compressed Suffix Array.
 *   Such representation will be handled as a "ticsa" data structure, that
 *   the WCSA self-index will use (as an opaque type) to 
 *   create/save/load/search/recover/getSize of the representation of the 
 *   original sequence.
 *   Suffix sorting is done via q-sort()
 * 
 *    
 * See more details in:
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

#include <stdio.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>

#include "../intIndexUtils/defValues.h"

#include "../utils/bitmap.h"
#include "../utils/huff.h"
#include "../utils/parameters.h"

#ifdef PSI_HUFFMANRLE
	#include "../intIndexUtils/psiHuffmanRLE.h"
#endif

#ifdef PSI_GONZALO
	#include "../intIndexUtils/psiGonzalo.h"
#endif

#ifdef PSI_DELTACODES
	#include "../intIndexUtils/psiDeltaCode.h"
#endif


typedef struct {
	uint sigma; // vocabulary size
	
	uint suffixArraySize;
	uint T_Psi;
	uint *D;
	bitmap bD;
//	uint T_A;
//	uint T_AInv;
//	uint *samplesA;
//	uint samplesASize;
//	uint *BA;
//	bitmap bBA;	
//	uint *samplesAInv;
//	uint samplesAInvSize;

//@@	uint displayCSAState;
//@@	long displayCSAPrevPosition;

	#ifdef PSI_HUFFMANRLE	
	HuffmanCompressedPsi hcPsi;
	#endif	
	#ifdef PSI_GONZALO
	GonzaloCompressedPsi gcPsi;
	#endif
	#ifdef PSI_DELTACODES
	DeltaCompressedPsi dcPsi;
	#endif
	
	//only needed during "parse_parameters".
	uint tempNSHUFF;
	uint psiSearchFactorJump;  //factor of the T_Psi value.
} ticsa;	

	
// FUNCTION PROTOTYPES: BUILDING THE INDEX

//Creates the ICSA	

	int buildIntIndex (uint *aintVector, uint n, uint sigma, uint n_traj, uint *l, uint *times, char *build_options, void **index ); 
	//ticsa *createIntegerCSA (uint **aintVector, uint SAsize, char *build_options);

//Returns number of elements in the indexed sequence of integers
	int sourceLenIntIndex(void *index, uint *numInts);

//Save the index to disk
	int saveIntIndex(void *index, char *pathname); //void storeStructsCSA(ticsa *myicsa, char *basename);

// Loads the index from disk.
	int loadIntIndex(char *pathname, void **index);  //ticsa *loadCSA(char *basename);

//  Frees memory	
	int freeIntIndex(void *index); //uint destroyStructsCSA(ticsa *myicsa);

//Returns the size (in bytes) of the index over the sequence of integers.
	int sizeIntIndex(void *index, size_t *numBytes); //uint CSA_size(ticsa *myicsa);	

	// Shows detailed summary info of the self-index (memory usage of each structure)
int printInfoIntIndex(void *index, const char tab[]);


	// returns the rank_1 (D,i)	
uint getRankicsa(void *index, uint i);

// returns the select_1 (D,i)	
uint getSelecticsa(void *index, uint i);

	// returns psi(i)	
uint getPsiicsa(void *index, uint i);
	// returns buffer[] <-- psi[ini..end]	
		
void getPsiBuffericsa(void *index, uint *buffer, size_t ini, size_t end);

	//recovers the sequence of integers kept by the ICSA.
void dumpICSASequence(void *index, uint **data, size_t *len);


//Number of occurrences of the pattern, and the interval [left,right] in the suffix array.
int countIntIndex(void *index, uint *pattern, uint length, ulong *numocc, ulong *left, ulong *right);
	//uint countCSA(ticsa *myicsa, uint *pattern, uint patternSize, uint *left, uint *right);		// Exponential search
	//uint countCSABin(ticsa *myicsa, uint *pattern, uint patternSize, uint *left, uint *right);	// Binary search

/* Private function prototypes ********************************************/
uint parametersCSA(ticsa *myicsa, char *build_options);

uint displayCSAFirst(ticsa *myicsa, uint position);
uint displayCSANext(ticsa *myicsa);
int  SadCSACompare(ticsa *myicsa, uint *pattern, uint patternSize, uint p);

void showStructsCSA(ticsa *myicsa);		// For Debugging


//@@ int locateIntIndex(void *index, uint *pattern, uint length, ulong **occ, ulong *numocc);
//@@ int displayIntIndex(void *index, ulong position, uint *value);
//@@ uint A(ticsa *myicsa, uint position);
//@@ uint inverseA(ticsa *myicsa, uint offset);

