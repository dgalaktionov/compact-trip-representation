/*
 * This code was originally made by Guillermo de Bernardo <guillermo.debernardo@udc.es>
 *
 * Evaluates the performance of the datastructure.
 */
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <iostream>

#include "timing.h"
//#include "tgs.h"
#include "interface.h"
#define IFERROR(error) {{if (error) { fprintf(stderr, "%s\n", error_index(error)); exit(1); }}}

#include "debug.h"

 #include "arraysort.c"  //@@
 #include "graphReader.h"

/*
FIXME: Non soporta un numero de nodos maior que MAX_INT.

As consultas son nodos aleatorios (non e unha permutacion,
pode haber repetidos)
*/


QueryType queryTypes[16] = {
  {0, 2, false, get_starts_with_x},
  {1, 1, false, get_ends_with_x},
  {2, 1, false, get_x_in_the_middle},
  {3, 2, false, get_from_x_to_y},
  {4, 1, true, get_top_k},
  {5, 1, false, get_starts_or_ends_with_x},
  {6, 1, true, get_top_k_starts},
  {7, 1, false, get_uses_x},
  {8, 2, false, get_from_x_to_y_strong},
  {9, 2, false, get_from_x_to_y_weak},
  {10, 0, false, get_uses_x},
  {11, 1, true, get_top_k_times},
  {12, 1, true, get_top_k2},
  {13, 1, true, get_top_k_starts_seq},
  {14, 0, false, get_starts_with_x},
  {15, 4, false, get_from_x_to_y}
};


#define ISIZE 5


 int qsortcompareuintAsc (const void *x, const void *y) {
	 uint *a = (uint *) x;
	 uint *b = (uint *) y;
	 if ((*a) < (*b)) return -1;
	 else if ( (*a) == (*b) ) return 0;
	 return +1;
 }


int compareRes(unsigned int * l1, unsigned int * l2) {
        uint i;
        if (l1[0]!=l2[0]) return 1;
        for (i = 1; i <= l1[0]; i++) {
                if (l1[i] != l2[i])  {
                        printf("\ngot value %u at index %d, expected %u\n", l1[i], i, l2[i]);
                        fflush(stdout);fflush(stderr);
                        return 1;
                }
        }
        return 0;
}

void compareResShowPosFari(unsigned int * l1, unsigned int * l2) {
        uint i;
        uint minL;
        minL= (l1[0] < l2[0]) ? l1[0] : l2[0];


        for (i = 1; i <= minL; i++) {
                if (l1[i] != l2[i])  {
                        printf("\ngot value %u at index %d, expected %u\n", l1[i], i, l2[i]);
                        fflush(stdout);fflush(stderr);
                        return ;
                }
        }
        return ;
}


#define MAX_QUERIES 10000
TimeQuery * readQueries(void *index, char * filename, int * nqueries, int ignoreTimes) {
        twcsa *g = (twcsa *)index;
        TimeQuery * ret = (TimeQuery *) malloc(MAX_QUERIES*sizeof(TimeQuery));
        FILE * queryFile = fopen(filename, "r");
        int curn = 0;
        int start_hour = -1;
        char line[128];
        while(curn < MAX_QUERIES) {
                TimeQuery *query = &ret[curn];
                query->type = (QueryType *) malloc(sizeof(QueryType));
                int res = EOF;
                res = fscanf(queryFile, "%d ", &(query->type->type));
                if (res == EOF) break;
                query->type = &(queryTypes[query->type->type]);
                query->values = (uint *) malloc(sizeof(uint) * query->type->nValues);

                // if (query->type->type == 15) {
                        for (int i = 0; i < query->type->nValues; i+=2) {
                                res = fscanf(queryFile, "%100[^:]:", &line[0]);
                                query->values[i] = g->lines->at(line);
                                res = fscanf(queryFile, "%d ", &query->values[i+1]);
                        }
                // } else {
                //         for (int i = 0; i < query->type->nValues; i++) {
                //                 res = fscanf(queryFile, "%d ", &query->values[i]);
                //         }
                // }

                fscanf(queryFile, "%d", &start_hour);

                if (start_hour == -1) {
                  query->time = NULL;
                } else {
                  query->time = (TimeRange *) malloc(sizeof(TimeRange));
                  query->time->h_start = start_hour;
                  fscanf(queryFile, "%d", &query->time->h_end);
                }

                if (ignoreTimes) {
                  query->time = NULL;
                }
                start_hour = -1;

                res = fscanf(queryFile, "\n");


                if(query->type->resultIsArray) {
                      res = fscanf(queryFile, "%d", &query->expectednres);
                        uint i;
                        query->expectedres = (unsigned int *) malloc((1+query->expectednres)*sizeof(unsigned int));
                        query->expectedres[0] = query->expectednres;
                        for (i = 1; i <= query->expectednres; i++) {
                                fscanf(queryFile, "%u ", &query->expectedres[i]);
                        }
                } else {
                        res = fscanf(queryFile, "%d\n", &query->expectednres);
                }
                curn++;
        }
        *nqueries = curn;
        return ret;
}

void printQuery(TimeQuery q) {
        printf("%d", q.type->type);

        for (int i = 0; i < q.type->nValues; i++) {
          printf(" %d", q.values[i]);
        }

        printf(" %d\n", q.expectednres);
}

#define CHECK_RESULTS 1
#define LOOPS 1000000




int main(int argc, char ** argv) {

  //FILE * flog = fopen("deactivewcsa.log","w");

        char * fileName;
        char * timesFile;

        //@@ struct tgs index;
        void *index;
        int totalres = 0;
        unsigned int * gotreslist;
        uint gotres = 0;

        if (argc < 4) {
                printf("Usage: %s <infexfile> <timesfile> <queryfile> [ignore_times] [loops]\n", argv[0]);
                exit(1);
        }

        fileName = argv[1];
        timesFile = argv[2];


        //@@ f.open(fileName, ios::binary);
        //@@ tgs_load(&index, f);
        //@@ f.close();

        ulong Index_size, Text_length;
        int error= load_index (fileName, timesFile, &index);
        IFERROR (error);
        //printInfo(index);

        struct graphDB graph;
        readLines(&graph, fopen("texts/lineStops.txt", "r"));
	readStops(&graph, fopen("texts/stopLines.txt", "r"));
	readAvgTimes(&graph, fopen("texts/avgTimes.txt", "r"));
	readInitialTimes(&graph, fopen("texts/initialTimes.txt", "r"));
        copy_commons(&graph, index);


    error = index_size(index, &Index_size);
    IFERROR (error);
    error = get_length(index, &Text_length);
    Text_length *= sizeof(uint);
    IFERROR (error);
    fprintf(stderr, "\nIndex size = %lu b\n", Index_size);
      //@@

        gotreslist = (uint*)malloc(sizeof(unsigned int)*BUFFER);
        int ignore_times = 0;

        if (argc > 4) {
          ignore_times = atoi(argv[4]);
        }

        int nqueries = 0;
        TimeQuery * queries = readQueries(index, argv[3], &nqueries, ignore_times);
        int executed_queries = LOOPS;

        if (argc > 5) {
          executed_queries = atoi(argv[5]);
        }

        int i,j;

#ifndef EXPERIMENTS
  printf("We are checking the results... Experiments mode off.\n");
#endif

        //for (j = 0; j < (queries[0].type->type == 15 ? XY_TOTAL : 1); j++) {
        for (j = 0; j < XY_TOTAL; j++) {
        //for (j = 0; j < 1; j++) {
                totalres = 0;
                startClockTime();
                        for (i = 0; i < executed_queries; i++) {
                        // printf("%i\n", i);
                                TimeQuery query = queries[i%nqueries];
                                // printQuery(query);

                                if (query.type->resultIsArray)
                                query.res = gotreslist;
                                query.subtype = j;
                                //query.subtype = XY_LINE_START;
                                gotres = query.type->callback(index, &query);
                                totalres += gotres;
                                //printf("%i %u %u\n%u\n", query.type->type, query.values[0], query.values[1], gotres);

                #ifndef EXPERIMENTS

                //                //Comentar para medir tiempos:
                                if (CHECK_RESULTS) {

                                int failcompare = 0;
                                if (!query.type->resultIsArray) {
                                failcompare = (gotres != query.expectednres);
                                } else {

                        //@@ fari ... sort reversed neighbors
                                //qsort(&gotreslist[1],gotreslist[0], sizeof(uint) ,qsortcompareuintAsc );
                                //@@

                                failcompare = compareRes(gotreslist, query.expectedres);
                                gotres = gotreslist[0];
                                }

                                if (failcompare) {
                                printf("\n --------------\n");
                                printf("query :"); printQuery(query);
                                printf("count: got %d expected %d\n", gotres, query.expectednres);

                //      compareResShowPosFari(gotreslist, query.expectedres);

                        if (query.type->resultIsArray) {
                                printf("expected: "); print_arraysort(query.expectedres);
                                printf("got     : "); print_arraysort(gotreslist);
                        }
                        //exit(1);
                                }
                                //totalres += gotres;
                                }

                #else
                                // totalres += *gotreslist;
                #endif




                        }

                        ulong microsecs = endClockTime()/1000; //to microsecs

                //  printf("time = (%lf), %d queries, %lf micros/query, %lf micros/arista\n",
                //                 difftime, nqueries,
                //                 difftime/nqueries, difftime/totalres);

                        //printf("time = %lf (%ld) (%lf), %d queries, %lf micros/query, %lf micros/arista\n",
                        //      timeFromBegin(), realTimeFromBegin(), difftime, nqueries,
                        //     difftime/nqueries, difftime/totalres);


                        // spatial_index temporal_index query_input num_queries totaloutput timeperquery timeperoutput
                        // printf("%s\t%s\t%s\t%ld\t%d\t%d\t%lf\t%lf\n", argv[1], argv[2], argv[3],
                        //                microsecs, executed_queries, totalres, (double)microsecs/executed_queries, (double)microsecs/totalres);
                        // query_input num_queries totaloutput timeperquery timeperoutput
                        printf("%s\t%ld\t%d\t%d\t%lf\t%lf\n", argv[3],
                                microsecs, executed_queries, totalres, (double)microsecs/executed_queries, (double)microsecs/totalres);

                        //destroyK2Tree(tree);
        }


//fclose(flog );
        exit(0);

}
