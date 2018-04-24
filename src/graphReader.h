#ifndef _GRAPHREADER
#define _GRAPHREADER

/*
struct graphDB {
	uint nEntries ;  // 3=RDF, 4=TemporalGraph
	uint *gaps;     //value to add to any entry in a RDF triplet, Temporal graph record, etc
		                //    gaps=[0, nodes, 2*nodes, 2*nodes+maxtime] for temporal graphs.

	uint nodes; //number of nodes from [0 ... nodes -1]
	uint maxtime; //maximum time in the dataset from [0 ... maxtime-1]
	uint edges; //unused.

	size_t n;   // number of contacts, or RDF triplets, or ...
	uint *s;  //including time and edges  (S[0 .. nEntries*n)
};
*/

#include "utils/basics.h"
#include "buildFacade.h"
#include "utils/delta.h"
int gr_readHeader (struct graphDB *graph, FILE *f);
int gr_readRecords(struct graphDB *graph, FILE *f);
void gr_printRecords(struct graphDB *graph);
int gr_areSortedRecords(struct graphDB *graph);
void gr_sortRecords(struct graphDB *graph) ;

#endif
