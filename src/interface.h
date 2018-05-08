#ifndef _INTERFACE_GRAPH
#define _INTERFACE_GRAPH


/* interface.h
 * Copyright (C) 2011, Antonio Fariña, all rights reserved.
 *
 * interface.h: Interface including all the functions temporal graph needs.
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

#include <map>
#include <utility>
#include <string>

/* General interface for using the compressed temporal graph */

#ifndef uchar
#define uchar unsigned char
#endif
#ifndef uint
#define uint  unsigned int
#endif
#ifndef ulong
#define ulong unsigned long
#endif


 struct graphDB {
  uint nEntries;  // number of gaps
	uint *gaps;     //value to add to any entry in a RDF triplet, Temporal graph record, etc
		                //    gaps=[0, nodes, 2*nodes, 2*nodes+maxtime] for temporal graphs.

	uint nodes; //number of nodes from [0 ... nodes -1]
	uint maxtime; //maximum time in the dataset from [0 ... maxtime-1]

	size_t n;   // number of contacts, or RDF triplets, or ...
	uint *s;  //including time and edges  (S[0 .. nEntries*n)
	uint *traj;	// trajectories index on s
	uint n_traj;	// number of trajectories

  uint *times; // times for s
  uint *weeks;  // time cycles
  uint nweeks;  // number of cycles
  //uint n_times; // should be the same as n
  std::map<std::string, uint16_t> *lines; // lines in our structure
};


typedef struct squery TimeQuery;

// TODO change for a C++ class
typedef struct {
  int type;
  int nValues;
  bool resultIsArray;
  int (*callback)(void *, TimeQuery*);
} QueryType;

typedef struct {
  uint h_start;
  uint h_end;
} TimeRange;

struct squery {
        QueryType *type;
        uint *values;
        uint *res;
        unsigned int expectednres;
        unsigned int * expectedres;
        TimeRange *time;
};

typedef struct {
  uint node;
  uint freq;
} TopK_Item;

typedef struct {
  uint start_v;
  uint end_v;
  uint start_i;
  uint end_i;
  uint freq;
} SpatialBinTopK_Item;

#define BUFFER 1024*1024*10


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

        /*  Saves index on disk by using single or multiple files, having
          proper extensions. */
int save_index (void *index, char *filename);
	//@@void tgs_save(struct tgs *a, ofstream & f);

        /*  Loads index from one or more file(s) named filename, possibly
          adding the proper extensions. */
int load_index (char *filename, char *timesFile, void **index);
	//@@void tgs_load(struct tgs *a, ifstream & f);

        /* Frees the memory occupied by index. */
int free_index (void *index);
	//@@void tgs_free(struct tgs *a);

        /* Gives the memory occupied by index in bytes. */
int index_size(void *index, ulong *size);
	//@@size_t tgs_size(struct tgs *a);


        /* Gives the length of the text indexed */

int get_length(void *index, ulong *length);

        /*  Obtains the length of the text indexed by index. */
int length (void *index, ulong *length);

/******** TRAJECTORIES FROM DAGAL ***************************/
int get_starts_with_x(void *index, TimeQuery *query);
int get_ends_with_x(void *index, TimeQuery *query);
int get_x_in_the_middle(void *index, TimeQuery *query);
int get_from_x_to_y(void *index, TimeQuery *query);
int get_from_x_to_y_strong(void *index, TimeQuery *query);
int get_from_x_to_y_weak(void *index, TimeQuery *query);
int get_top_k(void *index, TimeQuery *query); // binary
int get_top_k2(void *index, TimeQuery *query); // sequential
int get_starts_or_ends_with_x(void *index, TimeQuery *query);
int get_top_k_starts(void *index, TimeQuery *query);
int get_uses_x(void *index, TimeQuery *query);
int get_top_k_times(void *index, TimeQuery *query);
int get_top_k_starts_seq(void *index, TimeQuery *query);


#endif
