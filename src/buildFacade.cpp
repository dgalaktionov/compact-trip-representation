/* buildFacade.c
 * Copyright (C) 2011, Antonio Fariña, all rights reserved.
 *
 * BuildFacade.c: Implementation of the functions that the word-based self-index
 *   must implement following the "../pizza/interface.h" guidelines.
 *
 * See more details in:
 * Antonio Fariña, Nieves Brisaboa, Gonzalo Navarro, Francisco Claude, Ángeles Places,
 * and Eduardo Rodríguez. Word-based Self-Indexes for Natural Language Text. ACM
 * Transactions on Information Systems (TOIS), 2012.
 * http://vios.dc.fi.udc.es/indexing/wsi/publications.html
 * http://www.dcc.uchile.cl/~gnavarro/ps/tois11.pdf
 *
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

#include <sequence/Sequence.h>
#include <sequence/wt_coder.h>
#include "buildFacade.h"
#include "utils/errors.c"
#include "interface.h"
#include "../libcds/includes/sequence/WaveletMatrix.h"
#include "../libcds/includes/sequence/WaveletTree.h"
#include "../libcds/includes/sequence/wt_coder_hutucker.h"
#include "../libcds/includes/sequence/wt_coder_binary.h"
#include "../libcds/includes/sequence/wt_coder_huff.h"
#include "../libcds/includes/direct_access/DAC.h"



int createCompressedDictionary(void *index, uint samplingUnmap, uint nsHUFF) {
	twcsa *wcsa=(twcsa *) index;
	wcsa->cunmap = huffmanCompressPsi(wcsa->unmap,wcsa->nodes,samplingUnmap,nsHUFF);
	return 0;
}


int checkCompressedDictionary (void *index, uint samplingUnmap,uint nsHUFF) {
	twcsa *wcsa=(twcsa *) index;
	uint nodes = wcsa->nodes;
	//uint *gaps = wcsa->gaps;
	uint i;
	uint val1,val2;
//	printf("\n nodes = %u",nodes);
//	printf("\n gaps = [%u,%u,%u,%u,%u]", gaps[0], gaps[1], gaps[2], gaps[3], gaps[4]);
	fflush(stdout);

	uint count=0;
	for (i=0; i<nodes; i++) {
			val1= getHuffmanPsiValue(&wcsa->cunmap, i);
			val2=wcsa->unmap[i];
			if (val1 != val2) { count++;
		fprintf(stderr,"\n IDi=%u,UNMAP[i] vale (valHuff = %u) <> (val2= %u), ",i, val1,val2);
		if (count > 20) exit(0);;
			}
	}
	fprintf(stderr,"\n Test MAP/UNMAP (compressDictionary.HuffRLE) passed *OK*, ");
	return 0;
}

void createCompressedBitmapRRR (void *index){
	twcsa *wcsa=(twcsa *) index;
	size_t i;
	uint mapsize = wcsa->map_size;


	size_t numbits = mapsize;
	size_t numuints= (mapsize+W-1)/W;
	uint *data = (uint *) malloc (numuints * sizeof(uint));
	for (i=0;i<numuints;i++) data[i]=0;  //set zeroes;


//	for (i=0; i<5; i++)
//		printf("\n unmap[%u] = %u",i,wcsa->unmap[i]);

	for (i=0; i<wcsa->nodes; i++)
		bitset(data,wcsa->unmap[i]);

	BitSequenceBuilder *bs;
	//bs = new BitSequenceBuilderRRR(1024);  //DEFAULT_SAMPLING=32
	bs = new BitSequenceBuilderSDArray();

	wcsa->bmap =bs->build(data,numbits);

//	for (i=0; i<5; i++)
//		printf("\n select_1(%u) = %u",i,wcsa->bmap->select1(i));


	free(data);
	return;
}

int checkCompressedBitmapRRR (void *index) {
	twcsa *wcsa=(twcsa *) index;
	uint nodes = wcsa->nodes;
	//uint *gaps = wcsa->gaps;
	uint i;
	uint val1,val2;
	fflush(stdout);

	uint count=0;
	for (i=0; i<nodes; i++) {
			val1= wcsa->bmap->select1(i+1);
			val2=wcsa->unmap[i];
			if (val1 != val2) { count++;
		fprintf(stderr,"\n IDi=%u,UNMAP[i] vale (bmRRR.Select1 = %u) <> (val2= %u), ",i, val1,val2);
		if (count > 20) exit(0);;
			}
	}
	fprintf(stderr,"\n Test MAP/UNMAP (compressDictionary RRR) passed *OK*, ");
	return 0;
}

size_t calculateSizeTimes2(uint *freqs, size_t n, uint k) {
	size_t result = 0;
	uint *db = new uint[n/W+1]();

	for (uint i=0; i<n; i++) {
		if (freqs[i])
			bitset(db, i);
	}

	BitSequenceRRR bs = BitSequenceRRR(db, n, 32);
	result += bs.getSize();
	delete[] db;

	/*
	db = new uint[n/k+1]();
	uint sum = 0;

	for (uint i=0; i<n; i++) {
		if (i%k==0)
			db[i/k] = sum;

		sum+=freqs[i];
	}

	DAC d = DAC(db, n/k);
	result += d.getSize();
	delete[] db;
	*/

	return result;
}

size_t calculateSizeTimes3(uint *freqs, size_t n, uint k) {
		if (n <= 1)
			return 0;

		uint sum = 0;
		for (uint i = 0; i < n; i++)
			sum+=freqs[i];
		uint b = bits(sum);
		if (b == 0) return 0;

		uint node_size = (n+k-1)/k;
		uint nodes = (n+node_size-1)/node_size;
		size_t result = b*nodes;

		for (uint i = 0; i < n; i+=node_size) {
			if (i+node_size >= n) {
				result += calculateSizeTimes3(freqs+i, n-i, k);
			} else {
				result += calculateSizeTimes3(freqs+i, node_size, k);
			}
		}

		return result;
}

size_t calculateSizeTimes(uint *freqs, size_t n, uint k) {
	if (n <= 1)
		return 0;

	uint sum = 0;
	for (uint i = 0; i < n; i++)
		sum+=freqs[i];
	if (sum == 0) return 0;

	uint node_size = (n+k-1)/k;
	size_t result = k;

	for (uint i = 0; i < n; i+=node_size) {
		if (i+node_size >= n) {
			result += calculateSizeTimes(freqs+i, n-i, k);
		} else {
			result += calculateSizeTimes(freqs+i, node_size, k);
		}
	}

	return result;
}

void calculateOrder(uint *v, size_t n) {
	ulong changes = 0;
	ulong diff = 0;
	int last = 0;

	for (size_t i = 0; i < n; i++) {
		if (last != v[i]) {
			changes++;
			diff += abs(((int) v[i]) - last);
		}

		last = v[i];
	}

	std::cout << std::endl << changes << " changes, " << diff << " diff" << std::endl;

	return;
}

// Wavelet Matrix is built here
int buildTimesIndex(struct graphDB *graph, char *build_options, void **index) {
	twcsa *wcsa=(twcsa *) *index;
	uint maxtime = wcsa->maxtime+1;
    Mapper *mapper = new MapperNone();
    //Mapper *mapper = new MapperCont(wcsa->times, wcsa->n, BitSequenceBuilderRG(32), 0);
	//calculateOrder(wcsa->times, wcsa->n);


		fprintf(stderr,"\n Building Time Index...\n");
		// WaveletMatrix *timesWM = new WaveletMatrix(wcsa->times, 1, new BitSequenceBuilderRG(32), mapper,false);
		// WaveletMatrix *timesWM = new WaveletMatrix(wcsa->times, wcsa->n, new BitSequenceBuilderRG(32), mapper,false);
		WaveletMatrix *timesWM = new WaveletMatrix(wcsa->times, wcsa->n, new BitSequenceBuilderRRR(128), mapper,false);
		uint *freqs = new uint[maxtime](); // zero initialized!
		for (size_t i = 0; i < wcsa->n; i++) freqs[wcsa->times[i]]++;
		// WaveletTree *timesWM = new WaveletTree(wcsa->times, wcsa->n, new wt_coder_hutucker(freqs, maxtime), NULL, mapper);
		// WaveletTree *timesWM = new WaveletTree(wcsa->times, wcsa->n, new wt_coder_hutucker(freqs, maxtime), new BitSequenceBuilderRG(32), mapper);
		// WaveletTree *timesWM = new WaveletTree(wcsa->times, wcsa->n, new wt_coder_hutucker(freqs, maxtime), new BitSequenceBuilderRRR(128), mapper);
		//WaveletTree *timesWM = new WaveletTree(wcsa->times, wcsa->n, new wt_coder_hutucker(freqs, maxtime), new BitSequenceBuilderRPSN(new BitSequenceBuilderRG(32), 8, 4, 64u), mapper);
		//for (size_t i = 0; i < wcsa->n; i++) assert(timesWM->access(i) == wcsa->times[i]);
		delete[] freqs;
    //printf("\n\n\n WM range: %u\n\n\n", timesWM->rangeCount(500000, 600000, 2, 2));
		fprintf(stderr,"\n Done.\n");

    wcsa->myTimesIndex = (void *) timesWM;
    return 0;
}

int build_baseline(void *index) {
	twcsa *wcsa = (twcsa *) index;
	const auto n_traj = wcsa->n_traj;
	const auto n = wcsa->n;
	const uint *times = wcsa->times;
	
	uint *s = wcsa->s;
	size_t i;
	uint a,z,ta,tz;

	a = 0;

	std::vector<std::map<uint,uint32_t>> *usesX = new std::vector<std::map<uint,uint32_t>>(wcsa->nodes, std::map<uint,uint32_t>());
	std::vector<std::map<uint,uint32_t>> *startsX = new std::vector<std::map<uint,uint32_t>>(wcsa->nodes, std::map<uint,uint32_t>());
	std::vector<std::map<uint,uint32_t>> *endsX = new std::vector<std::map<uint,uint32_t>>(wcsa->nodes, std::map<uint,uint32_t>());

	matrix_map *fromXtoY = new matrix_map();
	std::pair<uint, uint> pair = std::make_pair((uint) 0, (uint) 0);
	std::pair<uint, uint> tpair = std::make_pair((uint) 0, (uint) 0);
	const std::pair<uint, uint> spair = std::make_pair(wcsa->maxtime+1, wcsa->maxtime+1);

	for (i = 0; i < n; i++) {
		if (s[i] < n_traj) {
			s[i] = 0;

			if (a > 0) {
				pair.first = a;
				pair.second = z;
				tpair.first = ta;
				tpair.second = tz;

				if (fromXtoY->count(pair) == 0) {
					std::map<std::pair<uint, uint>, uint32_t> times;
					times[spair] = 0;
					fromXtoY->insert(std::make_pair(pair, times));
				}

				if (fromXtoY->at(pair).count(tpair)) {
					fromXtoY->at(pair)[tpair]++;
				} else {
					fromXtoY->at(pair)[tpair] = 1;
				}

				fromXtoY->at(pair)[spair]++;

				if (startsX->at(a).count(ta))
					startsX->at(a)[ta]++;
				else
					startsX->at(a)[ta] = 1;
				
				if (endsX->at(a).count(ta))
					endsX->at(a)[ta]++;
				else
					endsX->at(a)[ta] = 1;
			}

			a = 0;
		} else {
			s[i] -= n_traj-1;

			z = s[i];
			tz = times[i];

			if (a == 0) {
				a = z;
				ta = tz;
			}

			if (usesX->at(a).count(ta))
				usesX->at(a)[ta]++;
			else
				usesX->at(a)[ta] = 1;
		}
	}

	wcsa->baseline = new tbaseline{usesX, startsX, endsX, fromXtoY};

	return 0;
}

/** Building the index */

    /* Creates index from text[0..length-1]. Note that the index is an
      opaque data type. Any build option must be passed in string
      build_options, whose syntax depends on the index. The index must
      always work with some default parameters if build_options is NULL.
      The returned index is ready to be queried. */
int build_index (struct graphDB *graph, char *build_options, void **index) {
	int returnvalue;

     printf("\n parameters: \"%s\"\n",build_options); fflush(stderr);fflush(stdout);

    returnvalue = build_WCSA (graph, build_options, index);

	// if (!returnvalue) {
	// 	returnvalue = build_baseline(*index);
	// }

    if (!returnvalue)
    	returnvalue = build_iCSA (build_options,*index);

    buildTimesIndex(graph, build_options, index);



#ifdef DICTIONARY_HUFFRLE
	uint samplingUnmap=32;
	uint nsHUFF= 16384;
	createCompressedDictionary(*index,  samplingUnmap, nsHUFF);
	checkCompressedDictionary(*index,  samplingUnmap, nsHUFF);
#else
	createCompressedBitmapRRR(*index);
	checkCompressedBitmapRRR(*index);
#endif

    return returnvalue;
}

/**  Saves index on disk by using single or multiple files, having
	proper extensions. */
int save_index (void *index, char *filename) {
	char *basename = filename;
	twcsa *wcsa=(twcsa *) index;
	char *outfilename;
	int file;

	printf("\n Saving structures to disk: %s.*",filename);		fflush(stderr);fflush(stdout);
	outfilename = (char *)my_malloc(sizeof(char)*(strlen(basename)+10));

	/**dummy file for dcaro-script **/
	{
		strcpy(outfilename, basename);
		unlink(outfilename);
		if( (file = open(outfilename, O_WRONLY|O_CREAT,S_IRWXG | S_IRWXU)) < 0) {
			printf("Cannot open file %s\n", outfilename);
			exit(0);
		}
		char msg[256];
		sprintf(msg, "G-icsa structure :: %s\n",filename);
		write(file,msg,strlen(msg));
		close(file);
	}


	/**File with some constants  */
	{
		strcpy(outfilename, basename);
		strcat(outfilename, ".");
		strcat(outfilename, CONSTANTS_FILE_EXT);
		unlink(outfilename);
		if( (file = open(outfilename, O_WRONLY|O_CREAT,S_IRWXG | S_IRWXU)) < 0) {
			printf("Cannot open file %s\n", outfilename);
			exit(0);
		}
		ssize_t write_err=0;
		write_err=write(file, &(wcsa->nEntries), sizeof(uint));
		write_err=write(file, wcsa->gaps, sizeof(uint) * (wcsa->nEntries));
		write_err=write(file, &(wcsa->nodes), sizeof(uint));
		write_err=write(file, &(wcsa->maxtime), sizeof(uint));
		write_err=write(file, &(wcsa->n_traj), sizeof(uint));
		write_err=write(file, &(wcsa->n), sizeof(size_t));

		close(file);
	}

	/**File with weeks  */
	{
		strcpy(outfilename, basename);
		strcat(outfilename, ".");
		strcat(outfilename, WEEKS_FILE_EXT);
		unlink(outfilename);
		if( (file = open(outfilename, O_WRONLY|O_CREAT,S_IRWXG | S_IRWXU)) < 0) {
			printf("Cannot open file %s\n", outfilename);
			exit(0);
		}
		ssize_t write_err=0;
		for (int i=0; i<wcsa->nweeks; i++) {
			write_err=write(file, &(wcsa->weeks[i]), sizeof(uint));
		}

		close(file);
	}

	/** File with the words from the vocabulary (sorted alphabetically) */
	{
		strcpy(outfilename, basename);
		strcat(outfilename, ".");
		strcat(outfilename, VOCABULARY_WORDS_FILE_EXT);
		unlink(outfilename);

		if( (file = open(outfilename, O_WRONLY|O_CREAT,S_IRWXG | S_IRWXU)) < 0) {
			printf("Cannot open file %s\n", outfilename);
			exit(0);
		}

		//the number of canonical words
		ssize_t write_err=0;
		write_err = write(file, &wcsa->map_size, sizeof(uint));
		write_err = write(file, wcsa->map, sizeof(uint) * wcsa->map_size);
		write_err = write(file, wcsa->unmap, sizeof(uint) * wcsa->nodes);

		close(file);
	}

	if (wcsa->baseline && 0) {
		strcpy(outfilename, basename);
		strcat(outfilename, ".");
		strcat(outfilename, "baseline");
		unlink(outfilename);

		if( (file = open(outfilename, O_WRONLY|O_CREAT,S_IRWXG | S_IRWXU)) < 0) {
			printf("Cannot open file %s\n", outfilename);
			exit(0);
		}

		for (auto const &times : *(wcsa->baseline->usesX)) {
			const size_t n = times.size();
			write(file, &n, sizeof(n));

			for (const auto &t : times) {
				write(file, &t, sizeof(t));
			}
		}

		for (auto const &times : *(wcsa->baseline->startsX)) {
			const size_t n = times.size();
			write(file, &n, sizeof(n));

			for (const auto &t : times) {
				write(file, &t, sizeof(t));
			}
		}

		for (auto const &times : *(wcsa->baseline->endsX)) {
			const size_t n = times.size();
			write(file, &n, sizeof(n));

			for (const auto &t : times) {
				write(file, &t, sizeof(t));
			}
		}

		for(const auto &p : *(wcsa->baseline->fromXtoY)) {
			const size_t n = p.second.size();

			write(file, &p.first.first, sizeof(uint));
			write(file, &p.first.second, sizeof(uint));
			write(file, &n, sizeof(size_t));

			for (const auto &t : p.second) {
				write(file, &t.first.first, sizeof(uint));
				write(file, &t.first.second, sizeof(uint));
				write(file, &t.second, sizeof(uint32_t));
			}
		}

		close(file);
	}

	if (wcsa->myTimesIndex) {
		strcpy(outfilename, basename);
		strcat(outfilename, ".");
		strcat(outfilename, TIMES_FILE_EXT);
		unlink(outfilename);
		std::ofstream ofs(outfilename, std::ofstream::out);
		((Sequence *)wcsa->myTimesIndex)->save(ofs);
		ofs.close();
	}

	if (wcsa->lines) {
		strcpy(outfilename, basename);
		strcat(outfilename, ".");
		strcat(outfilename, "lines");
		unlink(outfilename);
		std::ofstream ofs(outfilename, std::ofstream::out);
	
		for (const auto &line : *wcsa->lines) {
			ofs << line.first << std::endl;
		}

		ofs.close();
	}

	my_free_array(outfilename);

	if (wcsa->myicsa) {
		/******** saves index on integers (bottom) ******/
		saveIntIndex((void *) wcsa->myicsa, basename);
	}

	if (wcsa->s) {
		saveSEfile(basename, wcsa->s, (ulong) wcsa->n);
		//free(wcsa->se);
	}


	return 0;
}

    /**  Loads index from one or more file(s) named filename, possibly
      adding the proper extensions. */
int load_index(char *filename, char *timesFile, void **index){
	twcsa *wcsa;
	wcsa = loadWCSA (filename, timesFile);
	(*index) = (void *) wcsa;

#ifdef DICTIONARY_HUFFRLE
	uint samplingUnmap=32;
	uint nsHUFF= 16384;
	createCompressedDictionary(*index,  samplingUnmap, nsHUFF);
	checkCompressedDictionary(*index,  samplingUnmap, nsHUFF);
#else
	createCompressedBitmapRRR(*index);
	checkCompressedBitmapRRR(*index);
#endif

	//cout << "Loaded WM count: " << ((WaveletMatrix *) wcsa->myTimesIndex)->rangeCount(400000, 600000, 2, 2) << endl;

	return 0;
}

int loadTimeIndex(twcsa *wcsa, char *basename) {
		char filename[1024];
		strcpy(filename, basename);
		strcat(filename, ".");
		strcat(filename, TIMES_FILE_EXT);
		std::ifstream ifs(filename, std::ifstream::in);
		wcsa->myTimesIndex = (void *) Sequence::load(ifs);
		ifs.close();
}

	/** Frees the memory occupied by index. */
int free_index(void *index){
	twcsa *wcsa=(twcsa *) index;
	ulong size;
	index_size(index,&size);
	fflush(stderr);
	fflush(stdout);
	printf("\n[destroying index] ...Freed %lu bytes... RAM", size);

	//frees gaps array
	my_free_array(wcsa->gaps);

	//frees the array SE.
	if (wcsa->s)
		my_free_array (wcsa->s);

	if (wcsa->times)
		my_free_array (wcsa->times);

	if (wcsa->map)
		my_free_array (wcsa->map);

	if (wcsa->unmap)
		my_free_array (wcsa->unmap);

	if (wcsa->myTimesIndex)
		delete ((Sequence *) wcsa->myTimesIndex);

	if (wcsa->baseline && 0) {
		delete ((std::vector<std::map<uint,uint32_t>> *) wcsa->baseline->usesX);
		delete ((std::vector<std::map<uint,uint32_t>> *) wcsa->baseline->startsX);
		delete ((std::vector<std::map<uint,uint32_t>> *) wcsa->baseline->endsX);
		delete ((matrix_map *) wcsa->baseline->fromXtoY);
		delete ((tbaseline *) wcsa->baseline);
	}

	if (wcsa->lineStops) {
		delete wcsa->lineStops;
	}

	if (wcsa->stopLines) {
		delete wcsa->stopLines;
	}

	if (wcsa->avgTimes) {
		delete wcsa->avgTimes;
	}

	if (wcsa->initialTimes) {
		delete wcsa->initialTimes;
	}

#ifdef DICTIONARY_HUFFRLE
	destroyHuffmanCompressedPsi(&(wcsa->cunmap));
#else
	if (wcsa->bmap != NULL)
		delete wcsa->bmap;
#endif


	//the iCSA.
	if (wcsa->myicsa) {
		freeIntIndex((void *) wcsa->myicsa);
	}



	//the pointer to wcsa.
	my_free(wcsa);
	return 0;
}

	/** Gives the memory occupied by index in bytes. */
int index_size(void *index, ulong *size) {
	twcsa *wcsa=(twcsa *)index;
	*size=0;
	*size += sizeof(twcsa);

//**uncompressed
	//INTEGERS in map
	//*size += sizeof(uint) * (wcsa->map_size);  //all values


	#ifdef DICTIONARY_HUFFRLE
		//** vocabulary AS bitmap for "map", plus compressed representacion (huffman+rle+---) for cunmap
		//COMPRESSED UNMAP
		*size += wcsa->cunmap.totalMem;
		//BITMAP  n + o(n)
		*size += wcsa->map_size/8 + (wcsa->map_size/256 * sizeof(uint))   +   ((wcsa->map_size/32) * sizeof(char));
	#else
		//** vocabulary as a bitmapRRR.
		if (wcsa->bmap)
			*size +=wcsa->bmap->getSize();
	#endif


	if (wcsa->myicsa) {
		size_t nbytes;
		sizeIntIndex((void *) wcsa->myicsa, &nbytes);
		*size += nbytes;
	}

	fprintf(stderr,"\nSize of int index: %zu bytes (%.2f%% compression)\n",
		*size, 100*(*size)*8/(float)(bits(wcsa->nodes)*wcsa->n));

	if (wcsa->myTimesIndex) {
		size_t timesBytes = ((Sequence *) wcsa->myTimesIndex)->getSize();
		fprintf(stderr,"\nSize of times index: %zu bytes (%.2f%% compression)\n",
			timesBytes, 100*timesBytes*8/(float)(bits(wcsa->maxtime)*wcsa->n));
		*size += timesBytes;
	}

	size_t commonBytes = 0;

	if (wcsa->lineStops) {
		commonBytes = sizeof(size_t) * wcsa->lineStops->size();

		for (const auto &vec : *wcsa->lineStops) {
			commonBytes += sizeof(uint32_t) * vec.size();
		}
		
		fprintf(stderr,"\nSize of lineStops: %zu bytes", commonBytes);
		*size += commonBytes;
	}

	if (wcsa->stopLines) {
		commonBytes = sizeof(size_t) * wcsa->stopLines->size();

		for (const auto &vec : *wcsa->stopLines) {
			commonBytes += sizeof(uint16_t) * vec.size();
		}
		
		fprintf(stderr,"\nSize of stopLines: %zu bytes", commonBytes);
		*size += commonBytes;
	}

	if (wcsa->avgTimes) {
		commonBytes = sizeof(size_t) * wcsa->avgTimes->size();

		for (const auto &vec : *wcsa->avgTimes) {
			commonBytes += sizeof(uint16_t) * vec.size();
		}
		
		fprintf(stderr,"\nSize of avgTimes: %zu bytes", commonBytes);
		*size += commonBytes;
	}

	if (wcsa->initialTimes) {
		commonBytes = sizeof(size_t) * wcsa->initialTimes->size();

		for (const auto &vec : *wcsa->initialTimes) {
			commonBytes += sizeof(uint32_t) * vec.size();
		}
		
		fprintf(stderr,"\nSize of initialTimes: %zu bytes", commonBytes);
		*size += commonBytes;
	}

	if (wcsa->baseline && 0) {
		const uint bits_node = bits(wcsa->nodes);
		const uint bits_time = bits(wcsa->maxtime);
		uint max_count = 0;
		uint spatial_max_count = 0;
		uint spatial_count = 0;

		fprintf(stderr,"\nPURE SPATIAL STATS:");
		fprintf(stderr,"\n%zu different X to Y", wcsa->baseline->fromXtoY->size());
		

		for (const auto &times : *(wcsa->baseline->fromXtoY)) {
			spatial_count = 0;

			for (const auto &t : times.second) {
				max_count = max(max_count, t.second);
				spatial_count += t.second;
			}

			spatial_max_count = max(spatial_max_count, spatial_count);
		}

		fprintf(stderr,"\nMax count (from X to Y, no times): %zu (%zu bits)", spatial_max_count, bits(spatial_max_count));

		for (const auto &times : *(wcsa->baseline->usesX)) {
			spatial_count = 0;
			for (const auto &t : times) {
				spatial_count += t.second;
			}
			spatial_max_count = max(spatial_max_count, spatial_count);
		}

		fprintf(stderr,"\nMax count (uses X, no times): %zu (%zu bits)", spatial_max_count, bits(spatial_max_count));


		fprintf(stderr,"\n\n\nSPATIOTEMPORAL STATS:");

		fprintf(stderr,"\nMax count (from X to Y, with times): %zu (%zu bits)", max_count, bits(max_count));

		for (const auto &times : *(wcsa->baseline->usesX)) {
			for (const auto &t : times) {
				max_count = max(max_count, t.second);
			}
		}

		fprintf(stderr,"\nMax count (uses X, with times): %zu (%zu bits)", max_count, bits(max_count));

		const uint bits_count = bits(max_count);
		const uint bits_map_overhead = 32*8; // WHY???
		
		//MemTrack::TrackDumpBlocks();
		//MemTrack::TrackListMemoryUsage();
		size_t bytes = sizeof(*(wcsa->baseline->usesX));

		for (const auto &times : *(wcsa->baseline->usesX)) {
			bytes += sizeof(times) + (bits_time + bits_count + bits_map_overhead) * times.size() / 8;
		}
		
		//bytes += (wcsa->nodes) * (bytes + (wcsa->maxtime + 2) * sizeof(uint32_t));
		
		fprintf(stderr,"\nSize of baseline (uses X): %zu bytes\n", bytes);
		
		*size += bytes;
		bytes = sizeof(*(wcsa->baseline->startsX));

		for (const auto &times : *(wcsa->baseline->startsX)) {
			bytes += sizeof(times) + (bits_time + bits_count + bits_map_overhead) * times.size() / 8;
		}

		fprintf(stderr,"\nSize of baseline (starts with X): %zu bytes\n", bytes);

		*size += bytes;
		bytes = sizeof(*(wcsa->baseline->endsX));

		for (const auto &times : *(wcsa->baseline->endsX)) {
			bytes += sizeof(times) + (bits_time + bits_count + bits_map_overhead) * times.size() / 8;
		}

		fprintf(stderr,"\nSize of baseline (ends with X): %zu bytes\n", bytes);

		*size += bytes;
		bytes = sizeof(*(wcsa->baseline->fromXtoY));
		bytes += (sizeof(size_t) + sizeof(void *)) * wcsa->baseline->fromXtoY->bucket_count();

		fprintf(stderr,"\nSize of baseline (SPATIAL from X to Y): %zu bytes\n", bytes 
			+ wcsa->baseline->fromXtoY->size() * (bits_node*2 + sizeof(void *)*8 + bits_count)/8);

		for (const auto &p : *wcsa->baseline->fromXtoY) {
			bytes += (bits_node*2)/8 + sizeof(void *) + sizeof(p.second);
			bytes += ((bits_time*2 + bits_count + bits_map_overhead) * p.second.size())/8;
		}

		fprintf(stderr,"\nSize of baseline (from X to Y): %zu bytes\n", bytes);
		*size += bytes;

		size_t se_size = wcsa->n * (bits_node + bits_time) / 8;
		fprintf(stderr,"\nSize of baseline (total): %zu bytes (%.2f%% compression)\n", *size, 
			100*(*size)/((float) se_size));
	}

	return 0;
}

   /** Gives the number of integers in the indexed sequence */
int get_length(void *index, ulong *length) {
	twcsa *wcsa=(twcsa *) index;
	*length = wcsa->n;
	return 0;
}

   /** Gives the number of integers in the indexed sequence */
int length (void *index, ulong *length) {
	return (get_length(index,length));
}
/** *******************************************************************************
  * Showing some statistics and info of the index
  * *******************************************************************************/

		/* Shows summary info of the index */
		/* Shows summary info of the index */
int printInfo(void *index) {
	twcsa *wcsa = (twcsa *) index;

	unsigned long indexSize;
	size_t intIndexSize;
	int err;

	err = index_size(index, &indexSize);
	if (err!=0) return err;
	err = sizeIntIndex(wcsa->myicsa, &intIndexSize);
	if (err!=0) return err;

	printf("\n   Weeks (weeks) = ");
	for (int i=0; i<wcsa->nweeks; i++) {printf("%zu ",(size_t) wcsa->weeks[i]);}
	printf("\n   ");

#ifdef DICTIONARY_HUFFRLE
		printf("\n ===================================================:");
		printf("\n Summary of indexed Graph-DB-iCSA:");
		printf("\n   Number of entries (nEntries) per record = %zu",(size_t) wcsa->nEntries);
		printf("\n   Number of records/contacts (n) = %zu",(size_t) wcsa->n);
		printf("\n   Number of trajectories (n_traj) = %zu",(size_t) wcsa->n_traj);
		printf("\n   ");

		printf("\n   Vocabulary info -----");
		printf("\n      Num entries in map[]   (map_size)  = %zu",(size_t) wcsa->map_size);
		printf("\n        vocabulary unmap size (nodes*4)  =   %zu bytes",(size_t) wcsa->nodes * sizeof(uint));
		printf("\n      ->vocabulary unmap size (compressed HuffRLE)  = **%zu bytes**",(size_t) wcsa->cunmap.totalMem);

		printf("\n        vocabulary map[] size <as uints>    =   %zu bytes",(size_t) (wcsa->nodes + wcsa->map_size)*sizeof(uint) );
		printf("\n      ->vocabulary map[] size <with bitmap> = **%zu bytes**",(size_t) (wcsa->map_size/8 + (wcsa->map_size/256 * sizeof(uint))   +   ((wcsa->map_size/32) * sizeof(char)))  );
		printf("\n   Total vocabulary size = %zu bytes",(size_t) wcsa->cunmap.totalMem +
		                                                    (wcsa->map_size/8 + (wcsa->map_size/256 * sizeof(uint))   +   ((wcsa->map_size/32) * sizeof(char)))  );
		printf("\n   -----");
#else

		printf("\n   Vocabulary info -----");
		printf("\n      Num entries in map[]   (map_size)  = %zu",(size_t) wcsa->map_size);
		printf("\n      Num entries in unmap[] (nodes)  = %zu",(size_t) wcsa->nodes);

		printf("\n        vocabulary unmap size (nodes*4)  =   %zu bytes",(size_t) wcsa->nodes * sizeof(uint));
		//printf("\n      ->vocabulary unmap size (compressed)  = **%zu bytes**",(size_t) wcsa->cunmap.totalMem);
		//printf("\n      ->vocabulary map[] size <with bitmap> = **%zu bytes**",(size_t) (wcsa->map_size/8 + (wcsa->map_size/256 * sizeof(uint))   +   ((wcsa->map_size/32) * sizeof(char)))  );
		printf("\n        vocabulary map[] size <as uints>    =   %zu bytes",(size_t) (wcsa->nodes + wcsa->map_size)*sizeof(uint) );

		printf("\n Total vocabulary size as bitmapRRR (map=rank, unmap=Select) = %zu bytes**\n", wcsa->bmap->getSize());
		/*
		size_t v1,v2,v3,v4,vtot;
		v1=wcsa->bmap->rank1(wcsa->gaps[1]-1);
		v2=wcsa->bmap->rank1(wcsa->gaps[2]-1) - wcsa->bmap->rank1(wcsa->gaps[1]-1);
		v3=wcsa->bmap->rank1(wcsa->gaps[3]-1) - wcsa->bmap->rank1(wcsa->gaps[2]-1);
		v4=wcsa->bmap->rank1(wcsa->gaps[4]-1) - wcsa->bmap->rank1(wcsa->gaps[3]-1);
		vtot=wcsa->bmap->rank1(wcsa->gaps[4]-1);

		printf("\n Entries en vocabulary-range = [%zu][%zu][%zu][%zu], total **\n", v1,v2,v3,v4,vtot );
		*/

		printf("\n   -----");

		printf("\n   ");

#endif

		printf("\n   wcsa structure = %zu bytes", (size_t) sizeof(twcsa));

		printf("\n\n @@ Summary of self-index on Integers:");
		err = printInfoIntIndex(wcsa->myicsa, " ");
		if (err!=0) return err;

		printf("\n ========================================================:");
		printf("\n ** TOTAL SIZE IN MEMORY WHOLE STRUCTURE= %zu bytes **", (size_t) indexSize);
		printf("\n ========================================================:");
		printf("\n\n");
		return 0;
	}




/** ***********************************************************************************
	 CONSTRUCTION OF THE INDEX WCSA
    ***********************************************************************************/

/**
  * BUILDS THE WCSA INDEX
  */

int build_WCSA (struct graphDB *graph, char *build_options, void **index) {
	twcsa *wcsa;
	wcsa = (twcsa *) my_malloc (sizeof (twcsa) * 1);

	size_t i;

	wcsa->nEntries = graph->nEntries;
	wcsa->gaps     = graph->gaps;
	wcsa->nodes    = graph->nodes;
	wcsa->maxtime  = graph->maxtime;
	wcsa->n_traj  = graph->n_traj;

	wcsa->n        = graph->n;
	wcsa->s 	   = graph->s;
	wcsa->times	   = graph->times;
	wcsa->nweeks   = graph->nweeks;
	wcsa->weeks   = graph->weeks;

	wcsa->lines = graph->lines;

	wcsa->baseline = nullptr;
	wcsa->lineStops = nullptr;
	wcsa->stopLines = nullptr;
	wcsa->avgTimes = nullptr;
	wcsa->initialTimes = nullptr;

	const auto n_lines = wcsa->lines->size();

	printf("\n Number of nodes = %u", wcsa->nodes);
	printf("\n Number of times = %u", wcsa->maxtime+1);
	printf("\n Number of lines = %zu", n_lines);


	/** creates the vocabulary of "used values" **/
	// map, unmap, map_size, zeronode are set, and entries in s[] are remapped.
	//uint map_size = wcsa->gaps[wcsa->nEntries];
	uint map_size = wcsa->nodes;
	uint *wordscount = (uint *) my_malloc(sizeof(uint) * map_size);

	for (i=0; i< map_size; i++) wordscount[i]=0;

	for (i=0; i< wcsa->n ; i++)
		wordscount[wcsa->s[i]]++;


	wcsa->nodes=0;
	wcsa->n_stops = 0;
	uint32_t max_stop = 0;

	for (i=0; i< map_size; i++) {
		if (wordscount[i]!=0) wcsa->nodes++;
	}

	for (i=0; i < STOPS; i++) {
		if (wordscount[i]!=0) max_stop = i;
	}

	wcsa->n_stops = max_stop+1;

	fprintf(stderr,"\n n_stops %u ",wcsa->n_stops);
	fprintf(stderr,"\n real nodes %u ",wcsa->nodes);

	// for (i=0; i< map_size; i++) wordscount[i]=0;
	// map_size = 0;

	// for (i=0; i< wcsa->n ; i++) {
	// 	const auto stop = wcsa->s[i];

	// 	if (stop >= STOPS) {
	// 		assert(stop >= STOPS + STOPS_LINE);
	// 		wcsa->s[i] = wcsa->n_stops + ((stop-STOPS)/STOPS_LINE - 1) * n_lines + (stop % STOPS_LINE);
	// 	}

	// 	wordscount[wcsa->s[i]]++;

	// 	if (wcsa->s[i] >= map_size) {
	// 		map_size = wcsa->s[i]+1;
	// 	}
	// }

	fprintf(stderr,"\n map_size %u ",map_size);
	uint *map        = (uint *) my_malloc(sizeof(uint) * map_size);
	uint *unmap = (uint *) my_malloc(sizeof(uint) * wcsa->nodes);
	uint mapcount=0;


	map[0] = 0;

	for (i=0; i< map_size; i++) {
		if (wordscount[i]!=0) {
			//printf("%u, %u\n", i, wordscount[i]);
			unmap[mapcount]=i;
			map[i]=mapcount;
			mapcount++;
		}
	}

	//fprintf(stderr,"\n zeronode %u (%u)",zeronode, unmap[zeronode]);

	if (unmap[0] != 0) {
		printf("\n posible problema MAP/UNMAP al no existir nodo directo = 0\n");
		printf("\n unmap[0] = %u\n", unmap[0]);
	}
	fflush(stdout);fflush(stderr);

	for (i=1; i< map_size; i++) {
		if (wordscount[i]==0) {
			// fprintf(stderr,"\n Warning: unused word %zu", i);
			map[i]=map[i-1];
		}
	}

    my_free_array(wordscount);

	assert(mapcount == wcsa->nodes);

	//remap values in s[]
	for (i=0, mapcount=0; i< wcsa->n; i++) {
		//printf("\n s[i]=%u", wcsa->s[i]);
		//printf("\n mu[i]=%u\n", unmap[map[wcsa->s[i]]]);
		assert (wcsa->s[i] == unmap[map[wcsa->s[i]]]);

		if (i < 100) {
			// std::cout << wcsa->s[i] << ' ';
		}

		wcsa->s[i] = map[wcsa->s[i]];

		// Assign a unique ID to each 0
		if (wcsa->s[i] > 0) {
			wcsa->s[i] += wcsa->n_traj-1;
		} else {
			wcsa->s[i] = mapcount++;
		}
	}

	// printf("\n\n%u, %u\n\n", wcsa->n_traj,wcsa->s[wcsa->n-1]);

	assert(mapcount == wcsa->n_traj);
	//wcsa->s[wcsa->n-1]=0; // the last one is a real 0

	fprintf(stderr,"\n map and unmap vocabulary arrays created sucessfully");

	wcsa->map=map;
	wcsa->map_size = map_size;
	wcsa->unmap = unmap;
	/**/
	/*
	printf("\n");
	for (i=0; i < wcsa->ssize+1; i++) {
		printf("%u, ", wcsa->s[i]);
	}
	*/
	wcsa->myicsa = NULL;
	wcsa->bmap = NULL;
	wcsa->myTimesIndex = NULL;

	*index = wcsa;
	return 0;
}


int build_iCSA (char *build_options, void *index)
{
	twcsa *wcsa = (twcsa *) index;
	/********* creates the self-index on ints (bottom layer) *********/
	//creating CSA from Edu's code...
	{
		size_t total;
		fprintf(stderr,"\n **** CREATING CSA-bottom-layer *****");
		void *bottomIntIndex;
		buildIntIndex(wcsa->s, wcsa->n, wcsa->nodes, wcsa->n_traj, wcsa->times, build_options,(void **)&bottomIntIndex);
		wcsa->myicsa = bottomIntIndex;
		sizeIntIndex((void *) wcsa->myicsa, &total);

		printf("\n\t**** [iCSA built on %zu integers. Size = %zu bytes... RAM",(size_t) wcsa->n, total);
	}
	return 0;
}

void copy_commons (struct graphDB *graph, void *index) {
	twcsa *wcsa = (twcsa *) index;
	wcsa->lines = graph->lines;
	wcsa->lineStops = graph->lineStops;
	wcsa->stopLines = graph->stopLines;
	wcsa->avgTimes = graph->avgTimes;
	wcsa->initialTimes = graph->initialTimes;
}


	/** saves the content of the file SE (ids of the source words) **/
int saveSEfile (char *basename, uint *v, ulong n) {
	char outfilename[255];
	FILE *file;
	sprintf(outfilename,"%s.%s",basename,SE_FILE_EXT);
	unlink(outfilename);
	if( (file = fopen(outfilename, "w")) ==NULL) {
		printf("Cannot open file %s\n", outfilename);
		exit(0);
	}

	//write(file, v, sizeof(uint) * n );
	int err= fwrite(v,sizeof(uint),n,file);
	fclose(file);
	return err;
}


/** ********************************************************************
  * Loading from disk
  **********************************************************************/

/**-----------------------------------------------------------------
 * LoadWCSA.
 * Loads all the data structures of WCSA (included the icsa)
 ----------------------------------------------------------------- */

int loadBaseline(twcsa *wcsa, char *basename) {
	int file;
	char filename[1024];

	strcpy(filename, basename);
	strcat(filename, ".");
	strcat(filename, "baseline");
	
	if( (file = open(filename, O_RDONLY)) < 0) {
		printf("Cannot open file %s\n", filename);
		exit(0);
	}

	std::vector<std::map<uint,uint32_t>> *usesX = new std::vector<std::map<uint,uint32_t>>(wcsa->nodes, std::map<uint,uint32_t>());
	std::vector<std::map<uint,uint32_t>> *startsX = new std::vector<std::map<uint,uint32_t>>(wcsa->nodes, std::map<uint,uint32_t>());
	std::vector<std::map<uint,uint32_t>> *endsX = new std::vector<std::map<uint,uint32_t>>(wcsa->nodes, std::map<uint,uint32_t>());

	matrix_map *fromXtoY = new matrix_map();
	std::pair<uint, uint> pair = std::make_pair((uint) 0, (uint) 0);
	std::pair<uint, uint> tpair = std::make_pair((uint) 0, (uint) 0);

	size_t n;
	std::pair<uint,uint32_t> tpair2;

	for (auto &times : *(usesX)) {
		read(file, &n, sizeof(n));

		for (;n > 0;n--) {
			read(file, &tpair2, sizeof(tpair2));
			times[tpair2.first] = tpair2.second;
		}
	}

	for (auto &times : *(startsX)) {
		read(file, &n, sizeof(n));

		for (;n > 0;n--) {
			read(file, &tpair2, sizeof(tpair2));
			times[tpair2.first] = tpair2.second;
		}
	}

	for (auto &times : *(endsX)) {
		read(file, &n, sizeof(n));

		for (;n > 0;n--) {
			read(file, &tpair2, sizeof(tpair2));
			times[tpair2.first] = tpair2.second;
		}
	}

	uint a,z,ta,tz;
	uint32_t c;
	size_t i;
	

	while (read(file, &a, sizeof(uint))) {
		read(file, &z, sizeof(uint));
		read(file, &n, sizeof(size_t));

		pair.first = a;
		pair.second = z;
		std::map<std::pair<uint, uint>, uint32_t> times;

		for (i = 0; i < n; i++) {
			read(file, &ta, sizeof(uint));
			read(file, &tz, sizeof(uint));
			read(file, &c, sizeof(uint32_t));

			tpair.first = ta;
			tpair.second = tz;
			times[tpair] = c;
		}

		fromXtoY->insert(std::make_pair(pair, times));
	}

	fromXtoY->reserve(fromXtoY->size());

	close(file);

	wcsa->baseline = new tbaseline{usesX, startsX, endsX, fromXtoY};
	//wcsa->baseline = new tbaseline{NULL, NULL, NULL, fromXtoY};
}

twcsa *loadWCSA(char *filename, char *timesFile) {
	twcsa *wcsa;

	wcsa = (twcsa *) my_malloc (sizeof (twcsa) * 1);
	wcsa->n=0;

	loadIntIndex(filename, (void **)&wcsa->myicsa);
	loadStructs(wcsa,filename);
	loadTimeIndex(wcsa,timesFile);
	// loadBaseline(wcsa, filename);


	return wcsa;
}

/** ------------------------------------------------------------------
 * LoadStructs.
 *	Reads files and loads all the data needed for searcherFacade
 ----------------------------------------------------------------- */
 void loadStructs(twcsa *wcsa, char *basename) {
	char *filename;
	int file;

	filename = (char *)my_malloc(sizeof(char)*(strlen(basename)+10));
	fprintf(stderr,"Loading Index from file %s.*\n", basename);

	/**File with some constants  */
	{	strcpy(filename, basename);
		strcat(filename, ".");
		strcat(filename, CONSTANTS_FILE_EXT);

		if( (file = open(filename, O_RDONLY)) < 0) {
			printf("Cannot open file %s\n", filename);
			exit(0);
		}

	ssize_t read_err=0;
		read_err= read(file, &(wcsa->nEntries), sizeof(uint));
		wcsa->gaps = (uint *) my_malloc(sizeof(uint) * (wcsa->nEntries));
		read_err= read(file, wcsa->gaps, sizeof(uint) * (wcsa->nEntries));
		read_err= read(file, &(wcsa->nodes), sizeof(uint));
		read_err= read(file, &(wcsa->maxtime), sizeof(uint));
		read_err= read(file, &(wcsa->n_traj), sizeof(uint));
		read_err= read(file, &(wcsa->n), sizeof(size_t));

		close(file);
	}

	/**File with the weeks (time cycles) */
	{	strcpy(filename, basename);
		strcat(filename, ".");
		strcat(filename, WEEKS_FILE_EXT);

		if( (file = open(filename, O_RDONLY)) < 0) {
			printf("Cannot open file %s\n", filename);
			exit(0);
		}

		ssize_t read_err=0;
		uint i=0;
		wcsa->nweeks = 10;
		wcsa->weeks = (uint *) malloc(wcsa->nweeks * sizeof(uint));

		while (0 < (read_err = read(file, &wcsa->weeks[i], sizeof(uint)))) {
			i++;

			if (i >= wcsa->nweeks) {
				wcsa->nweeks *= 2;
				wcsa->weeks = (uint *) realloc(wcsa->weeks, wcsa->nweeks * sizeof(uint));
			}
		}

		wcsa->nweeks = i;
		wcsa->weeks = (uint *) realloc(wcsa->weeks, wcsa->nweeks * sizeof(uint));

		close(file);
	}

	/** File with the words from the vocabulary (sorted alphabetically) */
	{
		strcpy(filename, basename);
		strcat(filename, ".");
		strcat(filename, VOCABULARY_WORDS_FILE_EXT);
		//sizeFile= fileSize(filename)-sizeof(uint);

		if( (file = open(filename, O_RDONLY)) < 0) {
			printf("Cannot open file %s\n", filename);
			exit(0);
		}

		//the number of canonical words
		ssize_t read_err=0;
		read_err = read(file, &wcsa->map_size, sizeof(uint));
		wcsa->map = (uint *) my_malloc(sizeof(uint) * wcsa->map_size);
		wcsa->unmap = (uint *) my_malloc(sizeof(uint) * wcsa->nodes);
		read_err = read(file, wcsa->map, sizeof(uint) * wcsa->map_size);
		read_err = read(file, wcsa->unmap, sizeof(uint) * wcsa->nodes);

		close(file);
	}

	{
		strcpy(filename, basename);
		strcat(filename, ".");
		strcat(filename, "lines");
		std::ifstream ifs(filename, std::ifstream::in);
		std::string line;

		wcsa->lines = new std::map<std::string, uint16_t>();

		while (std::getline(ifs, line)) {
			wcsa->lines->emplace(line, wcsa->lines->size());
		}

		ifs.close();
	}

	wcsa->s= NULL;
	my_free_array(filename);
}




/** ****************************************************************
  * Querying the index WCSA
  * ***************************************************************/
///////////////////////////////////////////////////////////////////////////////////////
//					   FUNCTIONS NEEDED FOR SEARCHING A PATTERN    					 //
///////////////////////////////////////////////////////////////////////////////////////


//private function to use in "mapID"
uint getmap(twcsa *g, uint value) {
#ifdef DICTIONARY_HUFFRLE
	return g->map[value];  // NOT STILL CHANGED BY A rank1(Bitmap_for_map, value) ; we compute space for a bitmap with rank (37.5) but still have an array of uints
#else
	return g->bmap->rank1(value)-1;
#endif
}

//private function to use in "unmapID"
uint getunmap(twcsa *g, uint value) {
	return g->unmap[value];   	//COULD BE REPLACED BY A select (Bitmap_for_map, value);
	//return g->bmap->select1(value+1);
}

uint mapID (twcsa *g, uint value, uint type) {
	return getmap(g, value);
}

uint unmapID (twcsa *g, uint value, uint type) {
#ifdef DICTIONARY_HUFFRLE
	return getHuffmanPsiValue(&g->cunmap,value);
#else
	return (g->bmap->select1(value+1));
	//return getunmap(g, value)      - g->gaps[type] -1;
#endif
}


#define SAMPLES 1

// Wrapper for a WM count, designed to work well when querying overflowing time ranges (such as 23:30 to 0:40).
// When isContinuous, it returns in res the starting and ending indexes of the resulting range, as well as their values.
size_t getRange(twcsa *index, size_t lu, size_t ru, int t_start, int t_end, bool isContinuous, vector<pair<int, int>> * &res) {
	uint i,j;
	uint TIMES_WEEK = 0, TIMES_TOTAL = index->maxtime/SAMPLES;
	t_start = t_start/SAMPLES;
	t_end = t_end/SAMPLES;

	for (i=0,j=0; i<index->nweeks; i++){
		j+= index->weeks[i]/SAMPLES;

		if (t_start < j) {
			TIMES_WEEK = index->weeks[i]/SAMPLES;
		}
	}

	/*
	if (t_end >= t_start + TIMES_WEEK - (t_start % TIMES_WEEK)) {
		t_end = t_start + TIMES_WEEK - (t_start % TIMES_WEEK) - 1;
	}
	*/

	int t_start2 = TIMES_TOTAL;
	int t_end2 = TIMES_TOTAL;
	size_t numocc = 0;
	Sequence *wm = (Sequence *)index->myTimesIndex;

	// check if there's a time overflow
	if (t_start > t_end) {
		t_start2 = t_end - (t_end % TIMES_WEEK);
		t_end2 = t_end;
		t_end = t_start2 + TIMES_WEEK - 1;
	}

	// printf("%i %i %i %i\n", t_start, t_end, t_start2, t_end2);

	if (isContinuous) {
		res = new std::vector<pair<int,int>>(2);

		wm->range(lu, ru, t_start, t_end, res);
		
		if (res->back().first)
			numocc += res->back().first - res->front().first + 1;

		if (t_start2 != TIMES_TOTAL) {
			std::vector<pair<int,int>> *res2 = new std::vector<pair<int,int>>(2);
			wm->range(lu, ru, t_start2, t_end2, res2);

			if (res2->back().first)
				numocc += res2->back().first - res2->front().first + 1;

			res->insert(res->end(), res2->begin(), res2->end());
			delete res2;
		}
	} else {
		// printf("Q1 for %lu %lu: %i %i\n", lu, ru, t_start, t_end);
		numocc += wm->rangeCount(lu, ru, t_start, t_end);

		if (t_start2 != TIMES_TOTAL) {
			// printf("Q1 for %i %i\n", t_start2, t_end2);
			numocc += wm->rangeCount(lu, ru, t_start2, t_end2);
		}
	}

	return numocc;
}

// add or substract a time delta
int addTime(int t, int delta, uint TIMES_WEEK) {
	delta = delta % TIMES_WEEK;

	if (t >= TIMES_WEEK) {
		return TIMES_WEEK + ((t + delta) % TIMES_WEEK);
	} else {
		return (TIMES_WEEK + t + delta) % TIMES_WEEK;
	}
}

size_t getRange(twcsa *index, size_t lu, size_t ru, int t_start, int t_end) {
	 vector<pair<int, int>> *res = NULL;

	 return getRange(index, lu, ru, t_start, t_end, false, res);
}

int get_starts_with_x(void *index, TimeQuery *query) {
	twcsa *g = (twcsa *)index;
	ulong numocc, lu, ru;

	if (g->baseline) {
		const auto u = mapID(g, query->values[0], NODE);
		const auto &times = g->baseline->startsX->at(u);

		if (query->time) {
			numocc = 0;
			
			if (query->time->h_end < query->time->h_start) {
				for (auto i = times.begin(); i != times.upper_bound(query->time->h_end); i++) {
					numocc += i->second;
				}

				for (auto i = times.lower_bound(query->time->h_start); i != times.end(); i++) {
					numocc += i->second;
				}
			} else {
				for (auto i = times.lower_bound(query->time->h_start); i != times.upper_bound(query->time->h_end); i++) {
					numocc += i->second;
				}
			}
		} else {
			numocc = times.size() > 0 ? times.begin()->second : 0;
		}
		
		return numocc;
	}

	if (query->type->nValues) {
		uint u = mapID(g, query->values[0], NODE);
		uint pattern[2] = {0, u};
		// printf("%lu\n", u);

		countIntIndex(g->myicsa, pattern, 2, &numocc, &lu, &ru);
		// printf("%lu %lu %lu %lu\n", g->map_size, u, lu, ru);
	} else {
		lu = 1;
		ru = getSelecticsa(g->myicsa, 2)-1;
	}

	numocc = ru - lu + 1;

	// printf("%lu %lu %lu\n", numocc, lu, ru);

	if (query->time) {
		numocc = getRange(g, lu, ru,
						  query->time->h_start, query->time->h_end);
	}

	return numocc;
}

int get_ends_with_x(void *index, TimeQuery *query) {
	twcsa *g = (twcsa *)index;
	uint u = mapID(g, query->values[0], NODE);
	uint pattern[2] = {u, 0};
	ulong numocc, lu, ru;

	if (g->baseline) {
		const auto u = mapID(g, query->values[0], NODE);
		const auto &times = g->baseline->endsX->at(u);

		if (query->time) {
			numocc = 0;
			
			if (query->time->h_end < query->time->h_start) {
				for (auto i = times.begin(); i != times.upper_bound(query->time->h_end); i++) {
					numocc += i->second;
				}

				for (auto i = times.lower_bound(query->time->h_start); i != times.end(); i++) {
					numocc += i->second;
				}
			} else {
				for (auto i = times.lower_bound(query->time->h_start); i != times.upper_bound(query->time->h_end); i++) {
					numocc += i->second;
				}
			}
		} else {
			numocc = times.size() > 0 ? times.begin()->second : 0;
		}

		return numocc;
	}

	countIntIndex(g->myicsa, pattern, 2, &numocc, &lu, &ru);

	// printf("%lu %lu %lu\n", numocc, lu, ru);

	if (query->time && numocc) {
		numocc = getRange(g, lu, ru,
						  query->time->h_start, query->time->h_end);
	}

	return numocc;
}

int get_x_in_the_middle(void *index, TimeQuery *query) {
	return get_uses_x(index, query) - get_starts_with_x(index, query) - get_ends_with_x(index, query);
}

// Purely spatial from x to y
int get_from_x_to_y(void *index, TimeQuery *query) {
	twcsa *g = (twcsa *)index;
	const auto n_lines = g->lines->size();
	uint u = mapID(g, STOPS + query->values[1] * STOPS_LINE + query->values[0], NODE);
	uint v = mapID(g, query->values[3], NODE);
	//std::cout << u << ' ' << STOPS + query->values[1] * STOPS_LINE + query->values[0] << ' ' << unmapID(g,u,NODE_REV) << std::endl;
	//std::cout << unmapID(g,u+1,NODE_REV) << std::endl;
	assert(unmapID(g, v, NODE_REV) == query->values[3]);
	assert(unmapID(g, u, NODE_REV) == STOPS + query->values[1] * STOPS_LINE + query->values[0]);

	uint pattern[3] = {v, 0, u};

	ulong numocc, lu, ru;
	countIntIndex(g->myicsa, pattern, 3, &numocc, &lu, &ru);
	// printf("%lu %lu %lu\n", numocc, lu, ru);
/*
	for (int i = lu; i <= ru; i++) {
		printf("%u:%u ", getPsiicsa(g->myicsa, i), ((WaveletMatrix *) g->myTimesIndex)->access(getPsiicsa(g->myicsa, i)));
	}
	printf("\n");
*/

	return numocc;
}

// strong semantics
int get_from_x_to_y_strong(void *index, TimeQuery *query) {
	if (query->time == NULL) {
		return get_from_x_to_y(index, query);
	}

	twcsa *g = (twcsa *)index;
	uint u = mapID(g, query->values[0], NODE);
	uint v = mapID(g, query->values[1], NODE);
	uint pattern[3] = {v, 0, u};
	int start_time = query->time->h_start;
	int end_time = query->time->h_end;
	ulong numocc, lu, ru, lu0, ru0;

	if (g->baseline) {
		const std::map<std::pair<uint, uint>, uint32_t> &times = g->baseline->fromXtoY->at(std::make_pair(u,v));
		auto ta = times.lower_bound(std::make_pair(start_time, start_time));
		const auto &tz = times.upper_bound(std::make_pair(end_time, end_time));
		numocc = 0;

		if (end_time < start_time) {
			for (;ta!=times.cend();ta++) {
				numocc += ta->second;
			}

			for (ta=times.begin();ta!=tz;ta++) {
				numocc += ta->second;
			}
		} else {
			for (;ta!=tz;ta++) {
				numocc += ta->second;
			}
		}

		return numocc;
	}

	// First we get the Y$X range
	countIntIndex(g->myicsa, pattern, 3, &numocc, &lu, &ru);
	//printf("%lu %lu\n", u, v);
	//printf("%lu %lu\n", start_time, end_time);
	//printf("%lu %lu %lu\n", numocc, lu, ru);

	if (numocc == 0)
		return 0;

	// We calculate the matching XY$ range
	lu0 = getPsiicsa(g->myicsa, lu);
	ru0 = getPsiicsa(g->myicsa, ru);

	numocc = 0;
	std::vector<pair<int, int>> *res;

	// We get the subrange of XY$ that start within the desired time range
	if (getRange(g, lu0, ru0, start_time, end_time,
			true, res)) {

		// Translate that subrange to the original Y$X range
		// ...and see how many of them are also within the time range.
		numocc =  res->at(1).first ? getRange(g,
			lu + res->at(0).first - lu0,
			lu + res->at(1).first - lu0,
			start_time, end_time) : 0;

		// Handle the special case of time overflow (i.e. from 23:30 to 0:40)
		if (res->size() > 3 && res->at(3).first) {
			numocc += getRange(g,
				lu + res->at(2).first - lu0,
				lu + res->at(3).first - lu0,
				start_time, end_time);
		}
	}

	// printf("%lu %lu %lu\n", numocc, lu, ru);
	delete res;
	return numocc;
}

// weak semantics
int get_from_x_to_y_weak(void *index, TimeQuery *query) {
	if (query->time == NULL) {
		return get_from_x_to_y(index, query);
	}

	twcsa *g = (twcsa *)index;
	uint u = mapID(g, query->values[0], NODE);
	uint v = mapID(g, query->values[1], NODE);
	uint pattern[3] = {v, 0, u};
	int start_time = query->time->h_start;
	int end_time = query->time->h_end;
	int diff = 32;	// FIXME I'm abusing of the fact that no trajectory is longer than 31

	uint i,j;
	uint TIMES_WEEK = 0, TIMES_TOTAL = g->maxtime;

	for (i=0,j=0; i<g->nweeks; i++){
		j+= g->weeks[i];

		if (start_time < j) {
			TIMES_WEEK = g->weeks[i];
		}
	}

	// Handle cases when the queries time range is too fucking big
	if (end_time >= start_time) {
		diff = min(diff, ((int) TIMES_WEEK) - end_time + start_time - 1);
	} else {
		diff = min(diff, start_time - end_time - 1);
	}

	ulong numocc, lu, ru, lu0, ru0;

	if (g->baseline) {
		const std::map<std::pair<uint, uint>, uint32_t> &times = g->baseline->fromXtoY->at(std::make_pair(u,v));
		auto ta = times.lower_bound(std::make_pair(start_time, start_time));
		const auto &tz = times.upper_bound(std::make_pair(end_time, end_time));
		numocc = 0;

		if (end_time < start_time) {
			for (ta=times.begin();ta!=times.cend();ta++) {
				if (ta->first.second >= start_time)
					numocc += ta->second;
			}

			for (ta=times.begin();ta!=tz;ta++) {
				numocc += ta->second;
			}
		} else {
			for (ta=times.begin();ta!=tz;ta++) {
				if (ta->first.second >= start_time)
					numocc += ta->second;
			}
		}

		return numocc;
	}

	// First we get the Y$X range
	countIntIndex(g->myicsa, pattern, 3, &numocc, &lu, &ru);
	// printf("%lu %lu\n", u, v);
	// printf("%lu %lu\n", start_time, end_time);
	// printf("%lu %lu %lu\n", numocc, lu, ru);

	if (numocc == 0)
		return 0;

	// We calculate the matching XY$ range
	lu0 = getPsiicsa(g->myicsa, lu);
	ru0 = getPsiicsa(g->myicsa, ru);
	assert(ru-lu == ru0-lu0);

	std::vector<pair<int, int>> *res;

	// Count all the occurences within the desired time in XY$, they're all good for the result...
	numocc = getRange(g, lu0, ru0, start_time, end_time);

	if (diff) {
		//...but we also need to add the ones that start BEFORE the start time...
		if (getRange(g, lu0, ru0,
			/*addTime(start_time, -diff, TIMES_WEEK),*/
			(start_time/TIMES_WEEK)*TIMES_WEEK,
			addTime(start_time, -1, TIMES_WEEK),
			true, res)) {

			//... while ending AFTER the start time
			numocc += res->at(1).first ? getRange(g,
				lu + res->at(0).first - lu0,
				lu + res->at(1).first - lu0,
				start_time, (start_time/TIMES_WEEK+1)*TIMES_WEEK-1) : 0;

			// Handle the special case of time overflow (i.e. from 23:30 to 0:40)
			if (res->size() > 2 && res->at(3).first) {
				numocc += getRange(g,
					lu + res->at(2).first - lu0,
					lu + res->at(3).first - lu0,
					start_time, (start_time/TIMES_WEEK+1)*TIMES_WEEK-1);
			}
		}

		delete res;
	}

	// printf("%lu %lu %lu\n", numocc, lu, ru);
	return numocc;
}

bool topK_cmp(const TopK_Item a, const TopK_Item b) {
    return a.freq>b.freq;
}

// Repair the heap whose root element is at index 'start', assuming the heaps rooted at its children are valid
// https://en.wikipedia.org/wiki/Heapsort#Pseudocode
void sift_down(std::vector<TopK_Item> &topK, uint position) {
	uint left, right;

	left = position*2 + 1;
	right = position*2 + 2;

	while (right < topK.size()) {
		if (topK[position].freq > topK[left].freq || topK[position].freq > topK[right].freq) {
			if (topK[left].freq < topK[right].freq) {
				std::swap(topK[position], topK[left]);
				position = left;
			} else {
				std::swap(topK[position], topK[right]);
				position = right;
			}
		} else {
			// We're done. Don't use break to enable more compiler optimizations
			position = topK.size();
		}

		left = position*2 + 1;
		right = position*2 + 2;
	}

	if (left < topK.size() && topK[position].freq > topK[left].freq) {
		std::swap(topK[position], topK[left]);
	}
}

// sequential approach
int get_top_k2(void *index, TimeQuery *query) {
	twcsa *g = (twcsa *)index;
	uint k = query->values[0];
	uint n = g->map_size;	// we don't have the number of nodes :(
	uint select_before = getSelecticsa(g->myicsa, 2);
	uint current_select = 0, i = 1, freq = 0;
	std::vector<TopK_Item> topK(k, {0,0});
	TopK_Item item = {i, current_select - select_before};

	for (i = 3; i < k+3; i++) {
		current_select = getSelecticsa(g->myicsa, i);
		item = {i-2, current_select - select_before};

		if (query->time) {
			item.freq = getRange(g, select_before, current_select, query->time->h_start, query->time->h_end);
		}

		topK[i-3] = item;
		select_before = current_select;
	}

	std::make_heap(topK.begin(), topK.end(), topK_cmp);

	for (; i <= n; i++) {
		current_select = getSelecticsa(g->myicsa, i);

		// The root of the heap has the MINIMUM value of the Top K
		// It's a min-heap. It's not a mistake.
		freq = current_select - select_before;

		if (query->time) {
			freq = getRange(g, select_before, current_select, query->time->h_start, query->time->h_end);
		}

		if (freq > topK.front().freq) {
			item = {i-2, freq};
			topK[0] = item;
			sift_down(topK, 0);
			//std::make_heap(topK.begin(), topK.end(), topK_cmp);
		}

		select_before = current_select;
	}

	// Last iteration:
	current_select = g->n;
	freq = current_select - select_before;

	if (query->time) {
		freq = getRange(g, select_before, current_select, query->time->h_start, query->time->h_end);
	}

	if (freq > topK.front().freq) {
		item = {i-2, freq};
		topK[0] = item;
		sift_down(topK, 0);
		//std::make_heap(topK.begin(), topK.end(), topK_cmp);
	}

	std::sort_heap (topK.begin(), topK.end(), topK_cmp);

	//
	// std::cout << "final sorted range :\n";
	//   for (i=0; i<topK.size(); i++)
	//     std::cout << ' ' << topK[i].node << ' ' << topK[i].freq << '\n';
	//
	//
	// std::cout << '\n';

	query->res[0] = k;

	for (i = 0; i < k; i++) {
		query->res[i+1] = unmapID(g, topK[i].node, NODE_REV);
	}

	return k;
}

class Spatial_TopK_Cmp {
public:
    bool operator() (SpatialBinTopK_Item a, SpatialBinTopK_Item b) {
        return b.freq > a.freq;
    }
};

// binary partitions approach
int get_top_k(void *index, TimeQuery *query) {
	twcsa *g = (twcsa *)index;
	uint k = query->values[0];
	uint current_k = 0;
	uint n = g->map_size;

	std::priority_queue<SpatialBinTopK_Item, std::vector<SpatialBinTopK_Item>, Spatial_TopK_Cmp> topKQueue;
	uint middle = 1;
	uint middle_i = getSelecticsa(g->myicsa, 2);
	SpatialBinTopK_Item item = {1, n-1, middle_i, g->n - 1, 0};
	SpatialBinTopK_Item item_left;
	SpatialBinTopK_Item item_right;
	topKQueue.push(item);

	while (current_k < k) {
		item = topKQueue.top();
		topKQueue.pop();

		if (item.start_v == item.end_v) {
			// std::cout << ' ' << item.start_v << ' ' << item.freq << '\n';
			query->res[++current_k] = item.start_v;
		} else {
			middle = item.start_v + (item.end_v - item.start_v + 1)/2;
			middle_i = getSelecticsa(g->myicsa, middle + 1);
			item_left = {item.start_v, middle - 1, item.start_i, middle_i - 1, middle_i - item.start_i - 1};
			item_right = {middle, item.end_v, middle_i, item.end_i, item.end_i - middle_i};

			if (query->time) {
				item_left.freq = getRange(g, item_left.start_i, item_left.end_i,
					query->time->h_start, query->time->h_end);

				item_right.freq = getRange(g, item_right.start_i, item_right.end_i,
					query->time->h_start, query->time->h_end);
			}

			topKQueue.push(item_left);
			topKQueue.push(item_right);
		}
	}

	query->res[0] = k;

	return k;
}


int get_top_k_starts_seq(void *index, TimeQuery *query) {
	twcsa *g = (twcsa *)index;
	ulong lu, ru, numocc;
	uint k = query->values[0];
	uint n = g->map_size;	// we don't have the number of nodes :(
	uint i = 1;
	uint pattern[2] = {0, i};
	std::vector<TopK_Item> topK(k, {0,0});
	TopK_Item item = {i,0};

	for (i = 1; i <= k; i++) {
		pattern[1] = i;
		countIntIndex(g->myicsa, pattern, 2, &numocc, &lu, &ru);

		if (query->time) {
			numocc = getRange(g, lu, ru,	query->time->h_start, query->time->h_end);
		}

		item = {i, numocc};
		topK[i-1] = item;
	}

	std::make_heap(topK.begin(), topK.end(), topK_cmp);

	for (; i < n; i++) {
		pattern[1] = i;
		countIntIndex(g->myicsa, pattern, 2, &numocc, &lu, &ru);

		if (query->time) {
			numocc = getRange(g, lu, ru,	query->time->h_start, query->time->h_end);
		}

		// The root of the heap has the MINIMUM value of the Top K
		// It's a min-heap. It's not a mistake.
		if (numocc > topK.front().freq) {
			item = {i, numocc};
			topK[0] = item;
			sift_down(topK, 0);
			//std::make_heap(topK.begin(), topK.end(), topK_cmp);
		}
	}

	std::sort_heap (topK.begin(), topK.end(), topK_cmp);

	// std::cout << "final sorted range :\n";
	//   for (i=0; i<topK.size(); i++)
	//     std::cout << ' ' << topK[i].node << ' ' << topK[i].freq << '\n';

	//std::cout << '\n';

	query->res[0] = k;

	for (i = 0; i < k; i++) {
		query->res[i+1] = unmapID(g, topK[i].node, NODE_REV);
	}

	return k;
}

int get_top_k_starts(void *index, TimeQuery *query) {
	twcsa *g = (twcsa *)index;
	uint k = query->values[0];
	uint current_k = 0;
	uint n = g->map_size;
	uint pattern[2] = {0,0};
	uint skip = 0;
	ulong numocc, lu, ru;

	std::priority_queue<SpatialBinTopK_Item, std::vector<SpatialBinTopK_Item>, Spatial_TopK_Cmp> topKQueue;
	uint middle = 1;
	uint middle_i = 1;
	SpatialBinTopK_Item item = {1, n-1, middle_i, getSelecticsa(g->myicsa, 2) - 1, 0};
	SpatialBinTopK_Item item_left;
	SpatialBinTopK_Item item_right;
	topKQueue.push(item);

	while (current_k < k) {
		item = topKQueue.top();
		topKQueue.pop();

		if (item.start_v == item.end_v) {
			// std::cout << ' ' << item.start_v << ' ' << item.freq << '\n';
			query->res[++current_k] = item.start_v;
		} else {
			middle = item.start_v + (item.end_v - item.start_v + 1)/2;
			skip = 0;
			numocc = 0;

			while (numocc == 0 && middle+skip <= item.end_v) {
				pattern[1] = middle+skip;
				countIntIndex(g->myicsa, pattern, 2, &numocc, &lu, &ru);
				skip++;
			}

			if (numocc) {
				middle_i = lu;
				item_right = {middle+skip-1, item.end_v, middle_i, item.end_i, numocc};

				if (query->time) {
					item_right.freq = getRange(g, item_right.start_i, item_right.end_i,
						query->time->h_start, query->time->h_end);
				}

				topKQueue.push(item_right);
			} else {
				middle_i = item.end_i+1;
			}

			item_left = {item.start_v, middle-1, item.start_i, middle_i - 1, middle_i - item.start_i - 1};

			if (query->time) {
				item_left.freq = getRange(g, item_left.start_i, item_left.end_i,
					query->time->h_start, query->time->h_end);
			}

			topKQueue.push(item_left);
		}
	}

	query->res[0] = k;

	return k;
}

// Se topK in the Wavelet Matrix implementation
int get_top_k_times(void *index, TimeQuery *query) {
	twcsa *g = (twcsa *)index;
	query->res[0] = ((Sequence *) g->myTimesIndex)->topK(query->values[0], getSelecticsa(g->myicsa, 2), g->n-1, query->res+1);

	for (int i = 1; i <= query->res[0]; i++) {
		// cout << query->res[i] << endl;
	}

	return query->res[0];
}


int get_starts_or_ends_with_x(void *index, TimeQuery *query) {
	return get_starts_with_x(index, query) + get_ends_with_x(index, query);
}


int get_uses_x(void *index, TimeQuery *query) {
	twcsa *g = (twcsa *)index;
	ulong numocc = 0, lu = 0, ru = 0;

	if (g->baseline) {
		const auto u = mapID(g, query->values[0], NODE);
		const auto &times = g->baseline->usesX->at(u);

		if (query->time) {
			numocc = 0;
			
			if (query->time->h_end < query->time->h_start) {
				for (auto i = times.begin(); i != times.upper_bound(query->time->h_end); i++) {
					numocc += i->second;
				}

				for (auto i = times.lower_bound(query->time->h_start); i != times.end(); i++) {
					numocc += i->second;
				}
			} else {
				for (auto i = times.lower_bound(query->time->h_start); i != times.upper_bound(query->time->h_end); i++) {
					numocc += i->second;
				}
			}
		} else {
			numocc = times.size() > 0 ? times.begin()->second : 0;
		}

		return numocc;
	}

	//notice that:
	//select(0) is 0
	//select(1) is 1
	//select(2) is the first stop after the $
	//select(x+1) is where the stops x start

	if (query->type->nValues) {
		uint u = mapID(g, query->values[0], NODE);

		if (u+1 >= g->map_size) {
			lu = getSelecticsa(g->myicsa, u+1);
			ru = g->n - 1;
		} else {
			lu = getSelecticsa(g->myicsa, u+1);
			ru = getSelecticsa(g->myicsa, u+2)-1;
		}

		// printf("%lu %lu %lu %lu\n", g->map_size, u, lu, ru);
	} else {
		lu = getSelecticsa(g->myicsa, 2);
		ru = g->n-1;
	}

	numocc = ru - lu + 1;

	if (query->time) {
		Sequence *wm = (Sequence *)g->myTimesIndex;
		numocc = getRange(g, lu, ru, query->time->h_start, query->time->h_end);
		// numocc = getRange(g, lu, ru, query->time->h_start, query->time->h_start);
		// numocc = wm->rank(query->time->h_start, ru) - wm->rank(query->time->h_start, lu);
	}

	// printf("%lu %lu %lu\n", numocc, lu, ru);

	return numocc;
}


/**************************/
//** debugging only

uint get_num_nodes(void *gindex) {
	twcsa *g = (twcsa*) gindex;
	return g->nodes;
}


double getTime2 (void)
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
