/* buildAll.c
 * Copyright (C) 2011, Antonio Fariña, all rights reserved.
 *
 * BuildAll: Main program to check building and searching operations.
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


#include "graphReader.h"


/**------------------------------------------------------------------
  *  MAIN PROGRAM.
  *------------------------------------------------------------------ */

/* macro to detect and notify errors  */
#define IFERROR(error) {{if (error) { fprintf(stderr, "%s\n", error_index(error)); exit(1); }}}

void print_usage(char *);
double getTime(void);

	int main(int argc, char* argv[])
	{

	char *infile, *outfile;
    //uchar *text;
	char *params = NULL;
	//ulong text_len;
	void *Index;
	int error, i;
	double start, end;

    if (argc < 3) print_usage(argv[0]);
	if (argc > 3) {
		int nchars, len;
		nchars = argc-3;
		for(i=2;i<argc;i++)
			nchars += strlen(argv[i]);
		params = (char *) malloc((nchars+1)*sizeof(char));
		params[nchars] = '\0';
		nchars = 0;
		for(i=3;i<argc;i++) {
			len = strlen(argv[i]);
			strncpy(params+nchars,argv[i],len);
			params[nchars+len] = ' ';
			nchars += len+1;
		}
		params[nchars-1] = '\0';
	}

	outfile = argv[2];
// ---------- Set Graph properties and read input -----------------------

	printf("\n out file set to : %s",outfile); fflush(stdout);fflush(stderr);

	infile = argv[1];

	start = getTime();

	int shouldclosef=0;
	FILE *f = fopen(infile, "r");
	if (!f) f=stdin;
	else shouldclosef=1;
	//FILE *f = stdin;
	if(f == NULL) return 1;

	struct graphDB graph;

	graph.nEntries = 0; // number of gaps (TODO read dynamically)
	graph.gaps = NULL;

	readLines(&graph, fopen("texts/lineStops.txt", "r"));
	readStops(&graph, fopen("texts/stopLines.txt", "r"));
	readAvgTimes(&graph, fopen("texts/avgTimes.txt", "r"));
	readInitialTimes(&graph, fopen("texts/initialTimes.txt", "r"));

	error = gr_readHeader(&graph, f);
	IFERROR(error);
	fprintf(stderr, "\nRead weeks=%u\n", graph.nweeks);

	error = gr_readRecords(&graph,f);
	IFERROR(error);
	//fclose(f);

    if (gr_areSortedRecords(&graph))
		fprintf(stderr,"\n INPUT RECORDS ARE SORTED INCREASINGLY");
	else
		fprintf(stderr,"\n INPUT RECORDS ARE #not# SORTED INCREASINGLY");

	// gr_printRecords(&graph);
	gr_sortRecords(&graph);

    if (gr_areSortedRecords(&graph))
		fprintf(stderr,"\n SORTED RECORDS ARE SORTED INCREASINGLY");
	else
		fprintf(stderr,"\n SORTED RECORDS ARE #not# SORTED INCREASINGLY");

	// gr_printRecords(&graph);

	//uint gaps[graph.nEntries+1];
	if (graph.nEntries) {
		uint *gaps = (uint *) my_malloc (sizeof(uint) * (graph.nEntries));
		graph.gaps = gaps;
		uint gaps1stnode = graph.s[0];
		printf("\n 1st node = %u",gaps1stnode);
		// TODO make things with gaps
	}


	// ---------- End Set Graph properties and read input ---------------

	{
		//printf("\n parametros <<%s>>\n\n",stopwordsfile);
		//	build_WCSA (inputBuffer, finsize, stopwordsfile, NULL,outbasename);

		build_index (&graph, params,  &Index);  /** building the index */
		copy_commons(&graph, Index);
		end = getTime();
		free(graph.traj);


		/** saving the index to disk*/

		save_index (Index, outfile);
		fprintf(stderr,"Index saved !! ");

		/** tells the mem used by the index */
		ulong indexsize;
		index_size(Index, &indexsize);
		fprintf(stderr,"\nIndex occupied %lu bytes\n\n", indexsize);

		/** freeing the index */
		free_index(Index);
	}


	free(params);

	fprintf(stdout,"\n\n\t ## Building time (**parsing into integers + present_layer: %.3f secs\n", end-start );
	//fprintf(stdout,"\n\t ## Input: %lu bytes --> Output (pres_layer) %lu bytes.", text_len, index_len);
	//fprintf(stdout,"\n\t ## Overall compression --> %.2f%% (%.2f bits per char).\n\n",
    // 			(100.0*index_len)/text_len, (index_len*8.0)/text_len);

	if (shouldclosef)
		fclose(f);
	exit(0);

}



// --------------------------------------------------------------------
// --------------------------------------------------------------------

double
getTime (void)
{

	double usertime, systime;
	struct rusage usage;

	getrusage (RUSAGE_SELF, &usage);

	usertime = (double) usage.ru_utime.tv_sec +
		(double) usage.ru_utime.tv_usec / 1000000.0;

	systime = (double) usage.ru_stime.tv_sec +
		(double) usage.ru_stime.tv_usec / 1000000.0;

	return (usertime + systime);

}

void print_usage(char * progname) {
	fprintf(stderr, "Usage: %s <source file> <index file> [<parameters>]\n", progname);
	fprintf(stderr, "\nIt builds the index for the text in file <source file>,\n");
	fprintf(stderr, "storing it in <index file>. Any additional <parameters> \n");
	fprintf(stderr, "will be passed to the construction function.\n");
	fprintf(stderr, "At the end, the program sends to the standard error \n");
	fprintf(stderr, "performance measures on time to build the index.\n\n");
	exit(1);
}
