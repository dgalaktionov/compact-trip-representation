/* buildFacade.h
 * Copyright (C) 2011, Antonio Fariña, all rights reserved.
 *
 * BuildFacade.h: Definition of the functions that the word-based self-index
 *   must implement following the "../pizza/interface.h" guidelines. We also
 *   define here the data types required by the WCSA self-index, as well as the
 *   twcsa type (that will be referred to as an opaque type by "../pizza" programs.
 *
 * See more details in:
 * Antonio Fariña, Nieves Brisaboa, Gonzalo Navarro, Francisco Claude, Ángeles Places,
 * and Eduardo Rodríguez. Word-based Self-Indexes for Natural Language Text. ACM
 * Transactions on Information Systems (TOIS), 2012.
 * http://vios.dc.fi.udc.es/indexing/wsi/publications.html
 * http://www.dcc.uchile.cl/~gnavarro/ps/tois11.pdf

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

#ifndef BUILDFACADE_H
#define BUILDFACADE_H



/* only for getTime() */
#include <sys/time.h>
#include <sys/resource.h>
#include <assert.h>

#include <algorithm>
#include <queue>
#include <vector>
#include <map>
#include <utility>
#include <unordered_map>
#include <string>
#include <exception>

#include "utils/defValues.h"
#include "utils/fileInfo.h"


#include "utils/huff.h"
#include "utils/parameters.h"
#include "utils/bitmap.h"
#include "utils/zstdArray.h"


#include "intIndex_qsort/interfaceIntIndex.h"
#include "intIndexUtils/psiHuffmanRLE.h"

#include <utils/libcdsBasics.h>
#include <bitsequence/BitSequence.h>
#include <bitsequence/BitSequenceBuilder.h>

#include "interface.h"
#include "utils/delta.h"

// #include "MemTrack.h"

using namespace std;
using namespace cds_static;
using namespace cds_utils;

/*
#ifndef uchar
#define uchar unsigned char
#endif
#ifndef uint
#define uint  unsigned int
#endif
#ifndef ulong
#define ulong unsigned long
#endif
*/

#ifndef my_malloc
	#define my_malloc(u) malloc(u)
	//#define my_malloc(u) new uchar[u]
#endif

#ifndef my_free
	#define my_free(u) free(u)
#endif

#ifndef my_free_array
	#define my_free_array(u) free(u)
#endif


//Permits to show the results of the queries.
//#define DEBUG_FACADE_ON

//Allows using getPsiBuffericsa to sequentially uncompress consecutive
//psi values instead of obtaining (not sequentially) 1 value at a time

#define BUFFER_PSI_ON
//#define BUFFER_PSI_EDGE_ON


//default behavior... line uncommented!!
//      set for snapshot to use t1 <=t < T2
//comment next line for snapshot to use t1 <=t <=T2
#define SNAPSHOT_DOES_NOT_INCLUDE_RIGHT




//TYPE for use in mapID(id,type) and unmapID(id,type)
#define NODE     (0)
#define NODE_REV (1)
#define TIME_INI (2)
#define TIME_END (3)


#ifndef BUILD_FACADE_TYPES
#define BUILD_FACADE_TYPES
#define MAX_ENTRIES 10

struct pair_hash {
	template <class T1, class T2>
	std::size_t operator () (const std::pair<T1,T2> &p) const {
		return p.first*10000 + p.second;
	}
};

typedef std::unordered_map< std::pair<uint, uint>, std::map<std::pair<uint, uint>, uint32_t>, pair_hash > matrix_map;

typedef struct {
	std::vector<std::map<uint,uint32_t>> *usesX;
	std::vector<std::map<uint,uint32_t>> *startsX;
	std::vector<std::map<uint,uint32_t>> *endsX;
	matrix_map *fromXtoY;
} tbaseline;

	/**the WCSA index structures... */
	typedef struct {
		uint nEntries;  // number of gaps
		uint *gaps;     //value to add to any entry in a RDF triplet, Temporal graph record, etc
							//    gaps=[0, nodes, 2*nodes, 2*nodes+maxtime] for temporal graphs.

		uint nodes;     //number of nodes from [0 ... nodes]
		uint maxtime;   //maximum time in the dataset from [0 ... maxtime-1]
		uint n_traj;	// number of trajectories (zeros)

		size_t n;       //number of integers in s
		uint *s;        //including time and edges  (S[0 .. nEntries*n)

		uint *l;		// lines for each stop in s
		uint *times;	// times for each stop in s
		uint *weeks;	// time cycles
		uint nweeks;	// number of time cycles

		uint *map;   //the vocabulary mapping,
		uint map_size;
		uint *unmap;
		HuffmanCompressedPsi cunmap;

		BitSequence *bmap;
		void *myicsa; //the CSA built on SE

		void *linesIndex; // WM for lines
		void *myTimesIndex; // usually a WM for times

		tbaseline *baseline;

		std::map<std::string, uint16_t> *lines; // lines in our structure
		uint32_t n_stops; // max id for a stop (before lines)
		std::vector< std::vector<uint32_t> > *lineStops;
		std::vector< std::vector<uint16_t> > *stopLines;
		std::vector< std::vector<uint16_t> > *avgTimes;
		std::vector< std::vector<uint32_t> > *initialTimes;

		ZSTDArray *cInitialTimes;
	} twcsa;


#endif


enum CTRBitmap {RG, RRR, TOTAL};


typedef struct {
	CTRBitmap bitmap_type = CTRBitmap::RG;
	int32_t param = 32;
} tctrbitmap;

/** ******************************************************************************
    * Interface (from pizza chili) for using the WCSA index
*********************************************************************************/

/* Error management */

        /* Returns a string describing the error associated with error number
          e. The string must not be freed, and it will be overwritten with
          subsequent calls. */

char *error_index (int e);

/* Building the index */

        /* Creates index from text[0..length-1]. Note that the index is an
          opaque data type. Any build option must be passed in string
          build_options, whose syntax depends on the index. The index must
          always work with some default parameters if build_options is NULL.
          The returned index is ready to be queried. */

int build_index (struct graphDB *graph, char *build_options, void **index) ;
void copy_commons (struct graphDB *graph, void *index);

        /*  Saves index on disk by using single or multiple files, having
          proper extensions. */

int save_index (void *index, char *filename);

        /*  Loads index from one or more file(s) named filename, possibly
          adding the proper extensions. */

int load_index (char *filename, void **index);

        /* Frees the memory occupied by index. */

int free_index (void *index);

        /* Gives the memory occupied by index in bytes. */


int index_size(void *index, ulong *size);


        /* Gives the length of the text indexed */

int get_length(void *index, ulong *length);

        /*  Obtains the length of the text indexed by index. */
int length (void *index, ulong *length);


/** *******************************************************************************************/
/** Building part of the index ****************************************************************/

int build_WCSA (struct graphDB *graph,char *build_options, void **index);
int build_iCSA (char  *build_options, void *index);


/** *******************************************************************************
  * Showing some statistics and info of the index
  * *******************************************************************************/

		/* Shows summary info of the index */
int printInfo(void *index);


/** *******************************************************************************************/
/** Search part of the index ******************************************************************/
// Definitions of some PUBLIC function prototipes.

		//loading/freeing the data structures into memory.
    void loadStructs(twcsa *wcsa, char *basename);
	twcsa *loadWCSA(char *filename, char *timesFile);

// Definitions of PRIVATE functions
	//Auxiliary functions
	int saveSEfile (char *basename, uint *v, ulong n);
	double getTime2 (void);

//private function to use in "mapID"
uint getmap(twcsa *g, uint value);

//private function to use in "unmapID"
uint getunmap(twcsa *g, uint value);

uint mapID (twcsa *g, uint value, uint type);


uint unmapID (twcsa *g, uint value, uint type);

#endif
