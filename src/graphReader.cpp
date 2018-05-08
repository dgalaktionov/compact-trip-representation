#include "graphReader.h"

#define SAMPLES 6
#define PERIOD 28800
#define STOPS_LINE 100

uint GLOBAL_SORT_TIMES = 0;

// ***********************************************************************

int gr_readHeader (struct graphDB *graph, FILE *f) {
	uint i = 0, j=0, week;
	char separator = ' ';
	fscanf(f, "%u", &graph->maxtime);
	// graph->maxtime = PERIOD/SAMPLES;
	// graph->maxtime = graph->maxtime/SAMPLES;
	separator = fgetc(f);
	graph->nweeks = 10;
	graph->weeks = (uint *) malloc(graph->nweeks * sizeof(uint));

	while (separator != '\n') {
		fscanf(f, "%u", &week);
		// week = PERIOD/SAMPLES;
		// week = week/SAMPLES;
		separator = fgetc(f);
		graph->weeks[i++] = week;
		j+=week;

		if (i >= graph->nweeks) {
			graph->nweeks *= 2;
			graph->weeks = (uint *) realloc(graph->weeks, graph->nweeks * sizeof(uint));
		}
	}

	graph->nweeks = i;
	graph->weeks = (uint *) realloc(graph->weeks, graph->nweeks * sizeof(uint));
	assert(graph->maxtime == j);

	graph->lines = new std::map<std::string, uint16_t>();
	return 0;
}

int gr_readRecords(struct graphDB *graph, FILE *f ) {
	graph->nodes = 0;
	graph->maxtime = 0;
	graph->n = 256;
	graph->n_traj = 16;
	size_t nbits = 0;
	uint prev_time = -1;
	uint dummy;
	std::string line(128,0);

	uint *s = (uint *) my_malloc (graph->n * sizeof(uint));
	uint *times = (uint *) my_malloc (graph->n * sizeof(uint));
	uint *traj = (uint *) my_malloc(graph->n_traj * sizeof(uint));

	size_t to_read = 5000000;
	uint i=0, j=0;
	uint data, t;
	int read_result = 0;
	char separator;

	const uint no_val = graph->maxtime;

	s[i] = 0;
	times[0] = no_val;
	traj[j++] = i++;
	//traj[j++] = i;

	while(read_result != EOF) {
		memset(&line[0], 0, line.capacity());
		read_result = fscanf(f, "%100[^:]:%u:%u", &line[0], &data, &t);
		// t = (t % PERIOD)/SAMPLES;

		if (read_result == EOF){
			break;
		}

		if (graph->lines->count(line) == 0) {
			graph->lines->emplace(line, graph->lines->size());
		}

		data = data * STOPS_LINE + graph->lines->at(line);
		times[i] = t;
		s[i++]=data;

		if (t < prev_time)
			prev_time = t;

		// nbits += encodeGamma(&dummy, 0, t-prev_time+1);
		// nbits += riceSize(t-prev_time, 1);
		//prev_time = t;

		if (data >= graph->nodes)
			graph->nodes = data+1;

		if (t > graph->maxtime)
			graph->maxtime = t;

		separator = fgetc(f);

		if (separator == '\n') {
			s[i-1] = (s[i-1] - graph->lines->at(line))/STOPS_LINE;
			times[i] = no_val;
			s[i] = 0;
			traj[j++] = i++;
			prev_time = -1;
		}

		if(j%5000==0) fprintf(stderr, "Processing %.1f%%\r", (float)j/to_read*100);

		if (i >= graph->n - 2) {
			graph->n *= 2;
			s = (uint *) realloc(s, graph->n * sizeof(uint));
			times = (uint *) realloc(times, graph->n * sizeof(uint));
		}

		if (j >= graph->n_traj) {
			graph->n_traj *= 2;
			traj = (uint *) realloc(traj, graph->n_traj * sizeof(uint));
		}

		//if (i==to_read) break;
	}

	fprintf(stderr, "Processing %.1f%%", (float)i/to_read*100);
	// fprintf(stderr, "\n\n Variable codes use %zu bits!", nbits);

	graph->n = i;
	graph->n_traj = j;
	s = (uint *) realloc(s, (i) * sizeof(uint));
	times = (uint *) realloc(times, (i) * sizeof(uint));
	traj = (uint *) realloc(traj, graph->n_traj * sizeof(uint));

	//assert(no_val == graph->maxtime);
	graph->s = s;
	graph->times = times;
	graph->traj = traj;
	return 0;
}

void gr_printRecords(struct graphDB *graph) {
	fprintf(stderr,"\n %u trajectories read, max nodes = %u, max-time = %u", graph->n_traj, graph->nodes, graph->maxtime);
	uint i,j,z;
	z=0;
	for (j=0;j<10 && z < graph->n;j++) {
		fprintf(stderr,"\n\t[");
		do {
			fprintf(stderr,"%u ",graph->s[z]);
		} while(graph->s[++z] > 0);
		fprintf(stderr,"]");
	}
	fprintf(stderr,"\n...\n...\n");

	if (j >= 10) {
		z= graph->traj[(graph->n_traj-10)/2];

		for (j=0;j<10 && z < graph->n;j++) {
			fprintf(stderr,"\n\t[");
			do {
				fprintf(stderr,"%u ",graph->s[z]);
			} while(graph->s[++z] > 0);
			fprintf(stderr,"]");
		}
		fprintf(stderr,"\n...\n...\n");

		z= graph->traj[(graph->n_traj-10)];

		for (j=0;j<10 && z < graph->n;j++) {
			fprintf(stderr,"\n\t[");
			do {
				fprintf(stderr,"%u ",graph->s[z]);
			} while(graph->s[++z] > 0);
			fprintf(stderr,"]");
		}

		fprintf(stderr,"\n");
	}

}

int gr_areSortedRecords(struct graphDB *graph) {
	uint i = 0, j = 1, k = 0;

	while (j < graph->n) {
		do {
			if (graph->s[j] > graph->s[i]) {break;}
			else if (graph->s[j] < graph->s[i]) {fprintf(stderr,"\n UNSORTED: %u > %u  (record j=%u)\n", graph->s[j] , graph->s[i] , k); return 0;}
			else {i++; j++;}
		} while (graph->s[i] > 0 || graph->s[j] > 0);

		while (graph->s[i] > 0) i++;
		i++;
		while (graph->s[j] > 0) j++;
		j++;
		k++;
	}

	return 1;
}


struct graphDB *gr_graph; //  global variable for gr_graphsuffixCmp

size_t getTrajectoryStart(size_t i) {
	do {i--;} while (gr_graph->s[i] != 0);
	return i;
}

size_t getTrajectoryEnd(size_t i) {
	while (gr_graph->s[i+1] != 0) {i++;}
	return i;
}

int dollarCmp(const size_t a_start, const size_t b_start) {
	register size_t a,b;
	a=a_start;
	b=b_start;

	if (gr_graph->s[a] < gr_graph->s[b]) {
		return -1;
	} else if (gr_graph->s[a] > gr_graph->s[b]) {
		return +1;
	} else {
		a = getTrajectoryEnd(a);
		b = getTrajectoryEnd(b);

		if (gr_graph->s[a] < gr_graph->s[b]) {
			return -1;
		} else if (gr_graph->s[a] > gr_graph->s[b]) {
			return +1;
		} else {
			if (gr_graph->times[a_start] < gr_graph->times[b_start]) {
				return -1;
			} else {
				GLOBAL_SORT_TIMES++;
				 if (gr_graph->times[a_start] > gr_graph->times[b_start])
					return +1;
				else
					return 0;
			}
		}
	}
}

int gr_graphsuffixCmp(const void *arg1, const void *arg2) {

	size_t a_start = *((uint *) arg1);
	size_t b_start = *((uint *) arg2);

	register size_t a,b;
	a=a_start;
	b=b_start;

	while(gr_graph->s[a] == gr_graph->s[b]) {
		if (gr_graph->s[a] == 0) {
			if (a != a_start) {
				a = getTrajectoryStart(a);
				b = getTrajectoryStart(b);
			}

			return dollarCmp(a+1,b+1);
		}

		a++; b++;
	}

	if (gr_graph->s[a] < gr_graph->s[b])
		return -1;
	return +1; // If they were equal, 0 is returned by dollarCmp
}

void gr_sortRecords(struct graphDB *graph) {
	gr_graph = graph;
	// Don't touch the last trajectory
	qsort(graph->traj, graph->n_traj-1, sizeof(uint), gr_graphsuffixCmp);
	fprintf(stderr,"\n\n %u TIMES THAT START TIME AFFECTED THE SORTING\n\n", GLOBAL_SORT_TIMES);

	uint i = 0, j = 0, z = 0;

	/*
	printf("\n");
	for (i = 0; i < graph->n_traj; i++) {
		printf("%u ", graph->traj[i]);
	}
	printf("\n");
	*/

	uint *s = (uint *) my_malloc (graph->n * sizeof(uint));
	uint *times = (uint *) my_malloc (graph->n * sizeof(uint));

	for (i=0, j=0, z=0; i < graph->n_traj; i++) {
		j = graph->traj[i];
		graph->traj[i] = z;

		do {
			times[z] = graph->times[j];
			s[z++] = graph->s[j];
		} while (graph->s[++j] > 0);
	}

	free(graph->times);
	free(graph->s);
	graph->s = s;
	graph->times = times;

	/*
	printf("\n");
	for (i = 0; i < graph->n_traj; i++) {
		printf("%u ", graph->traj[i]);
	}
	printf("\n");
	*/
}

void gr_setGaps(struct graphDB *graph, uint *gaps) {
	uint i;
	graph->gaps = (uint *) my_malloc(sizeof(uint) * (graph->nEntries+1));
	for (i=0; i<=graph->nEntries;i++) graph->gaps[i] = gaps[i];
}

// ---------------------------------------------------------------------
