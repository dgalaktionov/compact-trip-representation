/* icsa.c
 * Copyright (C) 2011, Antonio Fariña and Eduardo Rodriguez, all rights reserved.
 *
 * icsa.c: Implementation of the interface "../intIndex/interfaceIntIndex.h"
 *   that permits to represent a sequence of uint32 integers with an iCSA:
 *   An integer-oriented Compressed Suffix Array.
 *   Such representation will be handled as a "ticsa" data structure, that
 *   the WCSA self-index will use (as an opaque type) to
 *   create/save/load/search/recover/getSize of the representation of the
 *   original sequence.
 *   Suffix sorting is done via q-sort()
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

#include "icsa.h"
#include "../../libsais/sais-2.4.1/include/sais.h"


int buildIntIndex (uint *aintVector, uint n, uint sigma, uint n_traj, uint *l, uint *times, char *build_options, void **index){
	uint textSize=n;
	uint *Psi, *SAI, *C;
	register uint i, j, k;
	uint nsHUFF;
	ticsa *myicsa;
	myicsa = (ticsa *) malloc (sizeof (ticsa));

	parametersCSA(myicsa, build_options);

	nsHUFF=myicsa->tempNSHUFF;

	// Almacenamos o valor dalguns parametros
	myicsa->sigma = sigma;
	myicsa->suffixArraySize = textSize;

	// Reservamos espacio para os vectores
	Psi = (uint *) malloc (sizeof(uint) * textSize);
	printf("\n\t *BUILDING THE SUFFIX ARRAY over %u integers... (with sais)\n", textSize);fflush(stdout);
	sais_i32((sa_int32_t *) aintVector, (sa_int32_t *) Psi, textSize, sigma+n_traj-1);
	printf("\n\t ...... ended.");
	printf("\n\t");

	// printf("\n\t");
	// for (i=503900; i<n; i++) printf("%u ", Psi[i]);
	// printf("\n\t");
	// for (i=503900; i<n; i++) printf("%u ", aintVector[Psi[i]]);
	// 	printf("\n\t");
	// for (i=3322; i<3422; i++) printf("%u ", Psi[i]);
	// printf("\n\t");
	// for (i=3322; i<3422; i++) printf("%u ", aintVector[Psi[i]]);
	// printf("\n");

	SAI = (uint *) calloc(textSize+1, sizeof(uint));	// +1 para repetir na ultima posición. Evitamos un if
	// BT: Reuse memory for times reordering
	for(i=0; i<textSize; i++) SAI[i] = l[i];
	for(i=0; i<textSize; i++) l[i] = SAI[Psi[i]];
	for(i=0; i<textSize; i++) SAI[i] = times[i];
	for(i=0; i<textSize; i++) times[i] = SAI[Psi[i]];

	// CONSTRUIMOS A INVERSA DO ARRAY DE SUFIXOS
	for(i=0;i<textSize;i++) SAI[Psi[i]] = i;
	SAI[textSize] = SAI[0];

	// CONSTRUIMOS E COMPRIMIMOS PSI
	printf("\n\t Creating compressed Psi...");
	for(i=0;i<textSize;i++) Psi[i] = SAI[Psi[i]+1];
	free(SAI);


	// ---------------------------------------------------------------------------//
	// ** aqui modificamos PSI, para hacerla cíclica en las tuplas ** //
	uint nDollars = n_traj;
	size_t z = 0;

	for (uint i = nDollars-1; i > 0; i--) {
		Psi[i] = Psi[i-1];
		l[i] = l[Psi[i]];
		times[i] = times[Psi[i]];
		/*z = i;

		while (Psi[z] >= nDollars) {
			z = Psi[z];
		}

		assert(Psi[z] == (i+1 < nDollars? i+1 : 0));
		Psi[z] = i;*/
	}

	Psi[0] = 0;	// respetar que nada sea apuntado dos veces

	// ** fin de la modificación de PSI para hacerla cíclica             ** //
	// ---------------------------------------------------------------------------//

	#ifdef PSI_HUFFMANRLE
	printf("\n\t Creating compressed Psi HUFFMAN RLE..."); fflush(stdout);fflush(stderr);
	myicsa->hcPsi = huffmanCompressPsi(Psi,textSize,myicsa->T_Psi,nsHUFF);
	#endif
	#ifdef PSI_GONZALO
	myicsa->gcPsi = gonzaloCompressPsi(Psi,textSize,myicsa->T_Psi,nsHUFF);
	#endif
	#ifdef PSI_DELTACODES
	printf("\n\t Creating compressed Psi DELTA CODES..."); fflush(stdout);fflush(stderr);
	myicsa->dcPsi = deltaCompressPsi(Psi,textSize,myicsa->T_Psi);
	#endif

	//free(Psi);


	// CONSTRUIMOS A FUNCION C
	C = (uint *) calloc((sigma), sizeof(uint));
	for(k=0, i=0;i<textSize;i++) {
		// Let's undo the joke with the zeros
		if (aintVector[i] >= n_traj)
			aintVector[i] -= n_traj-1;
		else
			aintVector[i] = 0;

		C[aintVector[i]]++;
	}

	assert(C[0] == n_traj);

	for (i=0,j=0;i<sigma;i++) {
		j = j + C[i];
		C[i] = j;
	}
	for(i=sigma-1;i>0;i--) C[i] = C[i-1];
	C[0] = 0;

	// Contruimos D
	myicsa->D = (uint*) malloc (sizeof(uint) * ((textSize+31)/32));
	for(i=0;i<((textSize+31)/32);i++) myicsa->D[i] = 0;
	for(i=0;i<sigma;i++) bitset(myicsa->D, C[i]);
	myicsa->bD = createBitmap(myicsa->D,textSize);
	free(C);

	// printf("\n");
	// for (i=3322; i<3422; i++) printf("%u ", Psi[i]);
	// printf("\n");
	// for (i=3322; i<3422; i++) printf("%u ", brank(myicsa->bD, Psi[i]));
	// printf("\n");

	// printf("\n\nOLD SIZE: %zu\n\n", sizebytesBitmap(myicsa->bD));
	// printf("SD SIZE: %zu\n\n", myicsa->bD->bs->getSize());
	free(Psi);

	*index = myicsa;
	return 0;
}


//Returns number of elements in the indexed sequence of integers
int sourceLenIntIndex(void *index, uint *numInts){
	ticsa *myicsa = (ticsa *) index;
	*numInts= myicsa->suffixArraySize;
	return 0; //no error;
}

int saveIntIndex(void *index, char *pathname) {
//void storeStructsCSA(ticsa *myicsa, char *basename) {

 	ticsa *myicsa = (ticsa *) index;
 	char *basename=pathname;

	char *filename;
	int file;
	int write_err;
	// Reservamos espacio para o nome do ficheiro
	filename = (char *)malloc(sizeof(char)*MAX_FILENAME_LENGTH);

	// Ficheiro co n�mero de elementos indexados (enteiros do texto orixinal)
	strcpy(filename, basename);
	strcat(filename, ".");
	strcat(filename, NUMBER_OF_ELEMENTS_FILE_EXT);
	unlink(filename);
	if( (file = open(filename, O_WRONLY|O_CREAT,S_IRWXG | S_IRWXU)) < 0) {
		printf("Cannot open file %s\n", filename);
		exit(0);
	}

	write_err=write(file, &(myicsa->sigma), sizeof(uint));
	write_err=write(file, &(myicsa->suffixArraySize), sizeof(int));
	close(file);

	strcpy(filename, basename);
	strcat(filename, ".");
	strcat(filename, PSI_COMPRESSED_FILE_EXT);

	#ifdef PSI_HUFFMANRLE
		storeHuffmanCompressedPsi(&(myicsa->hcPsi), filename);
	#endif
	#ifdef PSI_GONZALO
		storeGonzaloCompressedPsi(&(myicsa->gcPsi), filename);
	#endif
	#ifdef PSI_DELTACODES
		storeDeltaCompressedPsi(&(myicsa->dcPsi), filename);
	#endif

	// Ficheiro co vector de bits D
	strcpy(filename, basename);
	strcat(filename, ".");
	strcat(filename, D_FILE_EXT);
	unlink(filename);
	if( (file = open(filename, O_WRONLY|O_CREAT,S_IRWXG | S_IRWXU)) < 0) {
		printf("Cannot open file %s\n", filename);
		exit(0);
	}
	write_err=write(file, myicsa->D, sizeof(int)*((myicsa->suffixArraySize+31)/32));
	close(file);

	// Directorio de rank para D
	// Almacenamos o n�mero de superbloques seguido dos superbloques
	// E logo o n�mero de bloques seguido dos bloques
	strcpy(filename, basename);
	strcat(filename, ".");
	strcat(filename, D_RANK_DIRECTORY_FILE_EXT);
	saveBitmap(filename,myicsa->bD);

//@@	// Ficheiro coas mostras de A
//@@	strcpy(filename, basename);
//@@	strcat(filename, ".");
//@@	strcat(filename, SAMPLES_A_FILE_EXT);
//@@	unlink(filename);
//@@	if( (file = open(filename, O_WRONLY|O_CREAT,S_IRWXG | S_IRWXU)) < 0) {
//@@		printf("Cannot open file %s\n", filename);
//@@		exit(0);
//@@	}
//@@	write_err=write(file, myicsa->samplesA, sizeof(int) * (myicsa->samplesASize));
//@@	close(file);

//@@	// Ficheiro co vector BA (marca as posicions de A muestreadas)
//@@	strcpy(filename, basename);
//@@	strcat(filename, ".");
//@@	strcat(filename, BA_FILE_EXT);
//@@	unlink(filename);
//@@	if( (file = open(filename, O_WRONLY|O_CREAT,S_IRWXG | S_IRWXU)) < 0) {
//@@		printf("Cannot open file %s\n", filename);
//@@		exit(0);
//@@	}
//@@	write_err=write(file, myicsa->BA, sizeof(int)*((myicsa->suffixArraySize+31)/32));
//@@	close(file);

//@@	// Directorio de rank para BA
//@@	strcpy(filename, basename);
//@@	strcat(filename, ".");
//@@	strcat(filename, BA_RANK_DIRECTORY_FILE_EXT);
//@@	saveBitmap(filename, myicsa->bBA);

//@@	// Ficheiro coas mostras de A inversa
//@@	strcpy(filename, basename);
//@@	strcat(filename, ".");
//@@	strcat(filename, SAMPLES_A_INV_FILE_EXT);
//@@	unlink(filename);
//@@	if( (file = open(filename, O_WRONLY|O_CREAT,S_IRWXG | S_IRWXU)) < 0) {
//@@		printf("Cannot open file %s\n", filename);
//@@		exit(0);
//@@	}
//@@	write_err=write(file, myicsa->samplesAInv, sizeof(int) * (myicsa->samplesAInvSize));
//@@	close(file);

	// Ficheiro co periodo de muestreo de A e A inversa
	strcpy(filename, basename);
	strcat(filename, ".");
	//strcat(filename, SAMPLING_PERIOD_A_FILE_EXT);
	strcat(filename, SAMPLING_PERIOD_PSI_SEARCHFACTOR_FILE_EXT);

	unlink(filename);
	if( (file = open(filename, O_WRONLY|O_CREAT,S_IRWXG | S_IRWXU)) < 0) {
		printf("Cannot open file %s\n", filename);
		exit(0);
	}
//@@	write_err=write(file, &(myicsa->T_A), sizeof(int));
//@@	write_err=write(file, &(myicsa->T_AInv), sizeof(int));

	write_err=write(file, &(myicsa->psiSearchFactorJump), sizeof(uint));
	write_err=write(file, &(myicsa->T_Psi), sizeof(uint));

	close(file);
	free(filename);
	return write_err;
}

//Returns the size (in bytes) of the index over the sequence of integers.
//uint CSA_size(ticsa *myicsa) {
int sizeIntIndex(void *index, size_t *numBytes) {
	ticsa *myicsa = (ticsa *) index;
	size_t size = 0;
	size +=(sizeof (ticsa) * 1);
	size += sizeof(uint)*((myicsa->suffixArraySize+31)/32) ;  //D vector
	size += myicsa->bD->mem_usage;
//@@	size += sizeof(uint) * myicsa->samplesASize ;  // samples A
//@@	size += sizeof(uint) * myicsa->samplesAInvSize ;  // samples A^{-1}
//@@	size += sizeof(uint)*((myicsa->suffixArraySize+31)/32) ;  //BA vector
//@@	size += myicsa->bBA->mem_usage;

	#ifdef PSI_HUFFMANRLE
		size +=myicsa->hcPsi.totalMem;
	#endif
	#ifdef PSI_GONZALO
		size +=myicsa->gcPsi.totalMem;
	#endif
	#ifdef PSI_DELTACODES
		size +=myicsa->dcPsi.totalMem;
	#endif
	*numBytes = size;
	return 0; //no error.
}


//ticsa *loadCSA(char *basename) {
int loadIntIndex(char *pathname, void **index){

	char *basename=pathname;
	char *filename;
	int file;
	struct stat f_stat;
	uint suffixArraySize;


	ticsa *myicsa;
	myicsa = (ticsa *) malloc (sizeof (ticsa) * 1);
	int read_err;

	// VARIABLE GLOBAL QUE ALMACENA O ESTADO DOS DISPLAYS (IMPORTANTE PARA DISPLAY SECUENCIAL)
	// Almacena a �ltima posici�n do array de sufixos que mostramos con display ou displayNext
	// Se nos piden un displayNext, aplicamos PSI sobre esta posici�n e obtemos a seguinte,
	// coa que podemos obter o s�mbolo pedido, e actualizamos displayState
//@@	myicsa->displayCSAState = 0;
//@@	myicsa->displayCSAPrevPosition = -2;  //needed by DisplayCSA(position)

	// Reservamos espacio para o nome do ficheiro
	filename = (char *)malloc(sizeof(char)*MAX_FILENAME_LENGTH);

	// LEEMOS OS DATOS DO FICHEIRO QUE ALMACENA O NUMERO DE ELEMENTOS INDEXADOS
	strcpy(filename, basename);
	strcat(filename, ".");
	strcat(filename, NUMBER_OF_ELEMENTS_FILE_EXT);
	if( (file = open(filename, O_RDONLY)) < 0) {
		printf("Cannot read file %s\n", filename);exit(0);
	}
	read_err=read(file, &(myicsa->sigma), sizeof(uint));
	read_err=read(file, &suffixArraySize, sizeof(uint));
	myicsa->suffixArraySize = suffixArraySize;
	fprintf(stderr,"Number of indexed elements (suffix array size) = %u\n", suffixArraySize);

	// LEEMOS OS DATOS DO FICHEIRO QUE ALMACENA PSI COMPRIMIDA
	strcpy(filename, basename);
	strcat(filename, ".");
	strcat(filename, PSI_COMPRESSED_FILE_EXT);
	#ifdef PSI_HUFFMANRLE
		myicsa->hcPsi = loadHuffmanCompressedPsi(filename);
	#endif
	#ifdef PSI_GONZALO
		myicsa->gcPsi = loadGonzaloCompressedPsi(filename);
	#endif
	#ifdef PSI_DELTACODES
		myicsa->dcPsi = loadDeltaCompressedPsi(filename);
	#endif

	// LEEMOS OS DATOS DO FICHEIRO QUE ALMACENA D
	strcpy(filename, basename);
	strcat(filename, ".");
	strcat(filename, D_FILE_EXT);
	if( (file = open(filename, O_RDONLY)) < 0) {
		printf("Cannot read file %s\n", filename); exit(0);
	}
	myicsa->D = (uint *) malloc (sizeof(uint)*((suffixArraySize+31)/32));
	read_err=read(file, myicsa->D, sizeof(uint)*((suffixArraySize+31)/32));
	fprintf(stderr,"Bit vector D loaded (%u bits)\n", suffixArraySize);

	// LEEMOS OS DATOS DO FICHEIRO QUE ALMACENA O DIRECTORIO DE RANK1 PARA D
	strcpy(filename, basename);
	strcat(filename, ".");
	strcat(filename, D_RANK_DIRECTORY_FILE_EXT);
	myicsa->bD = loadBitmap(filename,myicsa->D,suffixArraySize);
	fprintf(stderr,"Rank1 Directory for D loaded (%d ones)\n", myicsa->bD->pop);
	/*
	{	uint ns, nb;
		ns = myicsa->bD->sSize;
		nb = myicsa->bD->bSize;
		myicsa->bD->data = myicsa->D;
		fprintf(stderr,"Rank1 Directory for D loaded (%d superblocks, %d blocks)\n", ns, nb);
	}
	*/

	// LEEMOS OS DATOS DO FICHEIRO QUE ALMACENA SAMPLES A
//@@	strcpy(filename, basename);
//@@	strcat(filename, ".");
//@@	strcat(filename, SAMPLES_A_FILE_EXT);
//@@	if( (file = open(filename, O_RDONLY)) < 0) {
//@@		printf("Cannot read file %s\n", filename); exit(0);
//@@	}
//@@	if( fstat(file, &f_stat) < 0) {
//@@		printf("Cannot read information from file %s\n", filename);	exit(0);
//@@	}
//@@	myicsa->samplesASize = (f_stat.st_size)/sizeof(uint);
//@@	myicsa->samplesA = (uint *)malloc(sizeof(uint) * myicsa->samplesASize);
//@@	read_err=read(file, myicsa->samplesA, sizeof(uint) * myicsa->samplesASize);
//@@	printf("Suffix array samples loaded (%d samples)\n", myicsa->samplesASize);

//@@	// LEEMOS OS DATOS DO FICHEIRO QUE ALMACENA BA
//@@	strcpy(filename, basename);
//@@	strcat(filename, ".");
//@@	strcat(filename, BA_FILE_EXT);
//@@	if( (file = open(filename, O_RDONLY)) < 0) {
		//@@printf("Cannot read file %s\n", filename); exit(0);
//@@	}
//@@	myicsa->BA = (uint *) malloc (sizeof(uint)*((suffixArraySize+31)/32));
//@@	read_err=read(file, myicsa->BA, sizeof(uint)*((suffixArraySize+31)/32));
//@@	printf("Bit vector BA loaded (%u bits)\n", suffixArraySize);

//@@	// LEEMOS OS DATOS DO FICHEIRO QUE ALMACENA O DIRECTORIO DE RANK1 PARA BA
//@@	strcpy(filename, basename);
//@@	strcat(filename, ".");
//@@	strcat(filename, BA_RANK_DIRECTORY_FILE_EXT);
//@@	myicsa->bBA = loadBitmap(filename,myicsa->BA,suffixArraySize);
//@@	{	uint ns, nb;
//@@		ns = myicsa->bBA->sSize;
//@@		nb = myicsa->bBA->bSize;
//@@		myicsa->bBA->data = myicsa->BA;
//@@		printf("Rank1 Directory for BA loaded (%d superblocks, %d blocks)\n", ns, nb);
//@@	}

//@@	// LEEMOS OS DATOS DO FICHEIRO QUE ALMACENA SAMPLES A INVERSA
//@@	strcpy(filename, basename);
//@@	strcat(filename, ".");
//@@	strcat(filename, SAMPLES_A_INV_FILE_EXT);
//@@	if( (file = open(filename, O_RDONLY)) < 0) {
//@@		printf("Cannot read file %s\n", filename); exit(0);
//@@	}
//@@	if( fstat(file, &f_stat) < 0) {
//@@		printf("Cannot read information from file %s\n", filename);	exit(0);
//@@	}
//@@	myicsa->samplesAInvSize = (f_stat.st_size)/(sizeof(uint));
//@@	myicsa->samplesAInv = (uint *)malloc(sizeof(uint) * myicsa->samplesAInvSize);
//@@	read_err=read(file, myicsa->samplesAInv, sizeof(uint) * myicsa->samplesAInvSize);
//@@	printf("Suffix array inverse samples loaded (%d samples)\n", myicsa->samplesAInvSize);

	// LEEMOS OS DATOS DO FICHEIRO QUE ALMACENA O PERIODO DE MUESTREO DO ARRAY DE SUFIXOS E DA INVERSA
	strcpy(filename, basename);
	strcat(filename, ".");
//	strcat(filename, SAMPLING_PERIOD_A_FILE_EXT);
	strcat(filename, SAMPLING_PERIOD_PSI_SEARCHFACTOR_FILE_EXT);
	if( (file = open(filename, O_RDONLY)) < 0) {
		printf("Cannot read file %s\n", filename); exit(0);
	}
//@@	read_err=read(file, &(myicsa->T_A), sizeof(uint));
//@@	read_err=read(file, &(myicsa->T_AInv), sizeof(uint));
//@@	printf("Sampling A Period T = %d, Sampling A inv Period TInv = %d\n", myicsa->T_A, myicsa->T_AInv);

	read_err=read(file, &(myicsa->psiSearchFactorJump), sizeof(uint));
	fprintf(stderr,"Psi Bin Search Factor-Jump is = %d\n", myicsa->psiSearchFactorJump);
	read_err=read(file, &(myicsa->T_Psi), sizeof(uint));
	fprintf(stderr,"Sampling Period PSI is = %d\n", myicsa->T_Psi);

	close(file);
	free(filename);




	//return myicsa;
	*index = myicsa;
	return read_err;
}


//uint destroyStructsCSA(ticsa *myicsa) {
int freeIntIndex(void *index) {
	ticsa *myicsa = (ticsa *) index;
		// Liberamos o espacio reservado

	if (!myicsa) return 0;

	size_t total=0, totaltmp=0;

	size_t nbytes;sizeIntIndex(index, &nbytes);

	total +=(sizeof (ticsa) * 1);;

	#ifdef PSI_HUFFMANRLE
		total+= totaltmp = myicsa->hcPsi.totalMem;
		destroyHuffmanCompressedPsi(&(myicsa->hcPsi));
	#endif
	#ifdef PSI_GONZALO
		total+= totaltmp = myicsa->gcPsi.totalMem;
		destroyGonzaloCompressedPsi(&(myicsa->gcPsi));
	#endif
	#ifdef PSI_DELTACODES
		total+= totaltmp = myicsa->dcPsi.totalMem;
		destroyDeltaCodesCompressedPsi(&(myicsa->dcPsi));
	#endif
	printf("\n\t[destroying  iCSA: compressed PSI structure] ...Freed %zu bytes... RAM", totaltmp);

	free(myicsa->D);			total+= totaltmp =  (sizeof(uint)*((myicsa->suffixArraySize+31)/32));
							printf("\n\t[destroying  iCSA: D vector] ...Freed %zu bytes... RAM",totaltmp);
								total += myicsa->bD->mem_usage;
	destroyBitmap(myicsa->bD);

//@@	free(myicsa->samplesA);		total+= totaltmp = (sizeof(uint) * myicsa->samplesASize);
//@@							printf("\n\t[destroying  Samples A: A   ] ...Freed %zu bytes... RAM",totaltmp);
//@@	free(myicsa->samplesAInv);	total+= totaltmp =  (sizeof(uint) * myicsa->samplesAInvSize);
//@@							printf("\n\t[destroying  Samples AInv: A   ] ...Freed %zubytes... RAM",totaltmp);
//@@						printf("\n\t[destroying  rank bit D   ] ...Freed %zu bytes... RAM", (size_t)myicsa->bD->mem_usage);
//@@	free(myicsa->BA);			total+= totaltmp =  (sizeof(uint)*((myicsa->suffixArraySize+31)/32));
//@@							printf("\n\t[destroying  SA: BA vector] ...Freed %zu bytes... RAM",totaltmp);

//@@								total += myicsa->bBA->mem_usage;
//@@	destroyBitmap(myicsa->bBA);

	printf("\n\t**** [the whole iCSA ocuppied ... %zu bytes... RAM",total);
	printf("\n\t**** iCSA size = %zu bytes ", (size_t) nbytes);
	printf("\n");

	free(myicsa);

	return 0; //no error.
}

	// Shows detailed summary info of the self-index (memory usage of each structure)
int printInfoIntIndex(void *index, const char tab[]) {
	ticsa *myicsa = (ticsa *) index;
	if (!myicsa) return 0;

	size_t structure, totalpsi, totalD, totalBD;

	structure=sizeof(ticsa);

	#ifdef PSI_HUFFMANRLE
		totalpsi = myicsa->hcPsi.totalMem;

		unsigned long  getHuffmanPsiSizeBitsUptoPosition(HuffmanCompressedPsi *cPsi, size_t position);
		unsigned long sizesPSIQuartil[4];
		unsigned long sizesPSIAcummQuartil[4];
		size_t n = myicsa->suffixArraySize;

		sizesPSIAcummQuartil[0] = getHuffmanPsiSizeBitsUptoPosition(&myicsa->hcPsi, n/4);
		sizesPSIAcummQuartil[1] = getHuffmanPsiSizeBitsUptoPosition(&myicsa->hcPsi, n/2);
		sizesPSIAcummQuartil[2] = getHuffmanPsiSizeBitsUptoPosition(&myicsa->hcPsi, 3*n/4);
		sizesPSIAcummQuartil[3] = getHuffmanPsiSizeBitsUptoPosition(&myicsa->hcPsi, n-1);

		sizesPSIQuartil[0] = getHuffmanPsiSizeBitsUptoPosition(&myicsa->hcPsi, n/4);
		sizesPSIQuartil[1] = getHuffmanPsiSizeBitsUptoPosition(&myicsa->hcPsi, n/2) - sizesPSIAcummQuartil[0];
		sizesPSIQuartil[2] = getHuffmanPsiSizeBitsUptoPosition(&myicsa->hcPsi, 3*n/4) - sizesPSIAcummQuartil[1];
		sizesPSIQuartil[3] = getHuffmanPsiSizeBitsUptoPosition(&myicsa->hcPsi, n-1) - sizesPSIAcummQuartil[2];

		sizesPSIQuartil[0] /=8;  //(in bytes);
		sizesPSIQuartil[1] /=8;  //(in bytes);
		sizesPSIQuartil[2] /=8;  //(in bytes);
		sizesPSIQuartil[3] /=8;  //(in bytes);

		sizesPSIAcummQuartil[0] /=8;  //(in bytes);
		sizesPSIAcummQuartil[1] /=8;  //(in bytes);
		sizesPSIAcummQuartil[2] /=8;  //(in bytes);
		sizesPSIAcummQuartil[3] /=8;  //(in bytes);


	#endif
	#ifdef PSI_GONZALO
		totalpsi = myicsa->gcPsi.totalMem;
	#endif
	#ifdef PSI_DELTACODES
		totalpsi = myicsa->dcPsi.totalMem;
	#endif

	totalD   = (sizeof(uint)*((myicsa->suffixArraySize+31)/32));
	totalBD  = myicsa->bD->mem_usage;

//@@	totalSA  = (sizeof(uint) * myicsa->samplesASize);
//@@	totalSAinv = (sizeof(uint) * myicsa->samplesAInvSize);
//@@	totalBA  = (sizeof(uint)*((myicsa->suffixArraySize+31)/32));
//@@	totalBBA = myicsa->bBA->mem_usage;

	size_t nbytes; sizeIntIndex(index, &nbytes); //whole self-index

	printf("\n ===================================================:");
	printf("\n%sSummary Self-index on integers (icsa) layer:",tab);
	printf("\n%s   icsa structure = %zu bytes",tab, structure);
	printf("\n%s   psi         = %10zu bytes",tab, totalpsi);
	#ifdef PSI_HUFFMANRLE
	printf("\n%s  psi ... Size at pos psi[Q1  (00-25%%)]  = %10zu bytes (accum = %10zu bytes)",tab, (size_t) sizesPSIQuartil[0], sizesPSIAcummQuartil[0]);
	printf("\n%s  psi ... Size at pos psi[Q2  (25-50%%)]  = %10zu bytes (accum = %10zu bytes)",tab, (size_t) sizesPSIQuartil[1], sizesPSIAcummQuartil[1]);
	printf("\n%s  psi ... Size at pos psi[Q3  (50-75%%)]  = %10zu bytes (accum = %10zu bytes)",tab, (size_t) sizesPSIQuartil[2], sizesPSIAcummQuartil[2]);
	printf("\n%s  psi ... Size at pos psi[Q4 (75-100%%)]  = %10zu bytes (accum = %10zu bytes)",tab, (size_t) sizesPSIQuartil[3], sizesPSIAcummQuartil[3]);

	#endif

	printf("\n%s   D (bitmap)  = %10zu bytes",tab, totalD);
	printf("\n%s   rank for D  = %10zu bytes",tab, totalBD);
//@@	printf("\n%s   SA(sampled) = %10u bytes",tab, totalSA);
//@@	printf("\n%s   SAinv(samp) = %10u bytes",tab, totalSAinv);
	//@@printf("\n%s   BA (bitmap) = %10u bytes",tab, totalBA);
//@@	printf("\n%s   rank for BA = %10u bytes",tab, totalBBA);
	printf("\n%sTotal = ** %zu bytes (in RAM) ** ",tab, nbytes);
	printf("\n");

	return 0; //no error.
}












uint inline locateSymbol(const ticsa *myicsa, const uint i) {
	return i+1 > myicsa->bD->pop ? myicsa->bD->n-1 : bselect(myicsa->bD, i+1);
}

uint locateCSASymbol(const void *myicsa, const uint i) {
	return locateSymbol(((ticsa *) myicsa), i);
}

uint inline getPsiValue(ticsa *myicsa, const uint i) {
	#ifdef PSI_HUFFMANRLE
		return getHuffmanPsiValue(&(myicsa->hcPsi),i);
	#endif
	#ifdef PSI_GONZALO
		return getGonzaloPsiValue(&(myicsa->gcPsi),i);
	#endif
	#ifdef PSI_DELTACODES
		return getDeltaPsiValue(&(myicsa->dcPsi),i);
	#endif
}



int countIntIndex(void *index, uint *pattern, uint length, ulong *numocc, ulong *left, ulong *right) {
	*numocc = 0;

	if (length == 0)
		return 0;

	ticsa *myicsa = (ticsa *) index;
	uint skip = myicsa->T_Psi;
	uint buffer[skip+1];
	register unsigned long l, r, i, lv, rv, k;
	//unsigned long sl, sr;

	if (*left == 0) {
		*left = locateSymbol(myicsa, pattern[length-1]);
	}

	if (*right == 0) {
		*right = locateSymbol(myicsa, pattern[length-1]+1) - 1;
	}

	for (i = length-1; i > 0; i--) {
		l = locateSymbol(myicsa, pattern[i-1]);
		r = locateSymbol(myicsa, pattern[i-1]+1) - 1;
		lv = r;
		rv = l;

		// printf("%lu %lu %lu\n", pattern[i], *left, *right);

		int j=0;
		if (l > 10) {
			j = -10;
		}

		// LEFT BINARY SEARCH
		while (l < lv && l <= (k = ((l+lv)/2)/skip*skip)) {
			// printf("\t\t%lu %lu %lu\n", l, k, lv);

			if (getPsiValue(myicsa, k) < *left) {
				l = k+skip;
			} else {
				lv = k;
			}
		}

		if (l >= skip && l >= lv)
			l -= skip;

		if (l < rv)
			l += skip;

		// printf("\tLEFT %lu %lu %lu %lu\n", l, lv, getPsiValue(myicsa, l), getPsiValue(myicsa, lv));

		if (l <= r) {
			// LEFT BINARY REFINE
			lv = lv-l > skip ? skip : lv-l+1; // right limit
			getHuffmanPsiValueBuffer(&(myicsa->hcPsi), buffer, l, l+lv);
			k = 0; // left limit

			/*
			for (int j=250; j<lv; j++) {
				printf("%u ", buffer[j]);
			}
			printf("\n");
			*/

			while (k < lv) {
				if (buffer[(k+lv)/2] < *left) {
					k = (k+lv)/2 + 1;
				} else {
					lv = (k+lv)/2;
				}
			}

			l = l + k;
			lv = buffer[k];

			if (lv < *left) {
				lv = *right+1; // NOT FOUND
			}

			// printf("\tLEFT %lu:%lu\n", l, lv);

			if (lv <= *right) {
				// RIGHT BINARY SEARCH
				lv = rv;
				rv = r;
				r = l;
				k = ((rv+r)/2)/skip*skip;

				if (r > k && k+skip < rv) {
					k += skip;
				}

				while (r < rv && r <= k) {
					// printf("\t\t%lu %lu %lu\n", r, k, rv);

					if (getPsiValue(myicsa, k) > *right) {
						rv = k;
					} else {
						r = k + skip;
					}

					k = ((rv+r)/2)/skip*skip;
				}

				if (r >= skip && r >= rv && r-skip >= l)
					r -= skip;

				// printf("\tRIGHT %lu %lu\n", r, rv);

				if (l <= rv) {
					// RIGHT BINARY REFINE
					rv = rv-r >= skip ? skip-1 : rv-r; // right limit
					getHuffmanPsiValueBuffer(&(myicsa->hcPsi), buffer, r, r+rv);
					k = 0;
					// printf("\t\t%u %u %u\n", r, k, rv);

					while (k < rv) {
						if (buffer[(k+rv+1)/2] > *right) {
							rv = (k+rv+1)/2 - 1;
						} else {
							k = (k+rv+1)/2;
						}
					}

					r = r + k;
					rv = buffer[k];
					// printf("\tRIGHT %lu:%lu\n", r, rv);
				}
			} else {
				*left = l;
				*right = l;
				return 0;
			}
		}

		*left = l;
		*right = r;

		if (l > r) {
			return 0;
		}
	}

	// printf("%lu %lu %lu\n", pattern[0], *left, *right);
	*numocc = *right-*left+1;
	return 0;
}







// OPERACIONS DO CSA

// BUSCA BINARIA SOBRE MOSTRAS + 2 BUSCAS EXPONENCIAIS + 2 BUSCAS BINARIAS
// 1º Busca binaria sobre o array de sufixos, elexindo como pivote un múltiplo de bin_search_psi_skip_interval (que orixinalmente foi pensado para igualar co valor de Psi).
// 2º Esta busca pode deterse por:
//	a) O pivote repítese entre dúas iteracións -> As ocorrencias están entre o pivote e a seguinte mostra (pivote + bin_search_psi_skip_interval) -> facemos dúas buscas binarias
//	b) O pivote é unha ocorrencia do patrón -> Faise unha busca exponencial sobre mostras hacia a esquerda e outra hacia a dereita, ata atopar a unha mostra á esquerda e outra
//	á dereita do intervalo de ocorrencias. Entre cada unha destas e a inmediatamente anterior da busca exponencial, faise unha busca binaria para atopar os extremos do intervalo.

int countIntIndex2(void *index, uint *pattern, uint length, ulong *numocc, ulong *left, ulong *right){
	//unsigned int countCSA(ticsa *myicsa, uint *pattern, uint patternSize, uint *left, uint *right) {

	uint patternSize = length;
	ticsa *myicsa = (ticsa *) index;

	register unsigned long l, r, i;
	register long comp, p, previousP;
	//register unsigned int l, r, i;
	//register int comp, p, previousP;
	register uint bin_search_psi_skip_interval = myicsa->psiSearchFactorJump;

	//fprintf(stderr,"\n psiSearchFactor = %d",myicsa->psiSearchFactorJump);

	l = 0;
	r = (myicsa->suffixArraySize+bin_search_psi_skip_interval-2)/bin_search_psi_skip_interval*bin_search_psi_skip_interval;
	p = ((l+r)/2)/bin_search_psi_skip_interval * bin_search_psi_skip_interval;
	previousP = 0;

	// BUSCA BINARIA SOBRE MOSTRAS
	while( (p != previousP) && (comp = SadCSACompare(myicsa, pattern, patternSize, p)) ) {
		if(comp > 0) l = p;
		else r = p;
		previousP = p;
		p = ((l+r)/2)/bin_search_psi_skip_interval*bin_search_psi_skip_interval;
	}

	if(p==previousP) {

		// BUSCA BINARIA ENTRE O PIVOTE E A SEGUINTE MOSTRA
		l = previousP;
		r = previousP+bin_search_psi_skip_interval;
		if(r > myicsa->suffixArraySize) r = myicsa->suffixArraySize - 1;
		while(l < r) {
			p = (l+r)/2;
			if(SadCSACompare(myicsa, pattern, patternSize, p) <= 0) r = p;
			else l = p+1;
		}

		if(SadCSACompare(myicsa, pattern, patternSize, r)) {
			*left = l;
			*right = r;
			//return 0;
			*numocc = 0;
			return 0; //no error.
		}
		*left = r;

		l = previousP;
		r = previousP+bin_search_psi_skip_interval;
		if(r > myicsa->suffixArraySize) r = myicsa->suffixArraySize - 1;
		while(l < r) {
			p = (l+r+1)/2;
			if(SadCSACompare(myicsa, pattern, patternSize, p) >= 0) l = p;
			else r = p-1;
		}
		*right = l;

	} else {

		previousP = p;	// En previousP poñemos o p atopado na busca sobre as mostras

		// BUSCA EXPONENCIAL HACIA ATRÁS
		i = 1;
		p -= bin_search_psi_skip_interval;
		while(p>0 && !SadCSACompare(myicsa, pattern, patternSize, p)) {
			i<<=1;
			p = previousP-(i*bin_search_psi_skip_interval);
		}
		// Busca binaria entre as duas ultimas mostras da exponencial
		if(previousP > i*bin_search_psi_skip_interval) l = previousP-(i*bin_search_psi_skip_interval);
		else l=0;
		i>>=1;
		r = previousP-(i*bin_search_psi_skip_interval);
		while(l < r) {
			p = (l+r)/2;
			if(SadCSACompare(myicsa, pattern, patternSize, p) <= 0) r = p;
			else l = p+1;
		}
		*left = r;

		// BUSCA EXPONENCIAL HACIA ADIANTE
		i = 1;
		p = previousP+bin_search_psi_skip_interval;
		while(p<myicsa->suffixArraySize && !SadCSACompare(myicsa, pattern, patternSize, p)) {
			i<<=1;
			p = previousP+(i*bin_search_psi_skip_interval);
		}
		// Busca binaria entre as duas ultimas mostras da exponencial
		if(p < myicsa->suffixArraySize) r = previousP+(i*bin_search_psi_skip_interval);
		else r = myicsa->suffixArraySize-1;
		i>>=1;
		l = previousP+(i*bin_search_psi_skip_interval);
		while(l < r) {
			p = (l+r+1)/2;
			if(SadCSACompare(myicsa, pattern, patternSize, p) >= 0) l = p;
			else r = p-1;
		}
		*right = l;
	}

	//return *right-*left+1;
	*numocc = (uint) *right-*left+1;
	return 0; //no error
}

// Version inicial de busca binaria
unsigned int countCSABin(ticsa *myicsa, uint *pattern, uint patternSize, uint *left, uint *right) {
	register ulong l, r, p;

	l = 0;
	r = myicsa->suffixArraySize-1;

	while(l < r) {
		p = (l+r)/2;
		if(SadCSACompare(myicsa, pattern, patternSize, p) <= 0) r = p;
		else l = p+1;
	}

	// SE SON DISTINTOS O PATRON NON APARECE NO TEXTO E DEVOLVEMOS 0
	if(SadCSACompare(myicsa, pattern, patternSize, r)) {
		*left = l;
		*right = r;
		return 0;
	}

	// Almacenamos o limite esquerdo
	*left = r;

	// SE SON IGUALES (O PATRON APARECE NO TEXTO), BUSCAMOS AGORA O LIMITE DEREITO, QUE ALMACENAREMOS EN right
	// NOTA: INICIAMOS A BUSQUEDA A PARTIR DO ESQUERDO...
	l = r;
	r = myicsa->suffixArraySize-1;

	while(l < r) {
		p = (l+r+1)/2;
		if(SadCSACompare(myicsa, pattern, patternSize, p) >= 0) l = p;
		else r = p-1;
	}

	// Gardamos o limite dereito
	*right = l;

	return (uint) *right-*left+1;
}



	// returns the rank_1 (D,i)
uint getRankicsa(void *index, uint i) {
	ticsa *myicsa = (ticsa *) index;
	return brank(myicsa->bD, i);// - 1;
}

	// returns the select_1 (D,i)
uint getSelecticsa(void *index, uint i) {
	ticsa *myicsa = (ticsa *) index;
	return bselect(myicsa->bD, i);// - 1;
}


	// returns psi(i)
uint getPsiicsa(void *index, uint i) {
	ticsa *myicsa = (ticsa *) index;
	register uint positionAux;
		#ifdef PSI_HUFFMANRLE
			positionAux=getHuffmanPsiValue(&(myicsa->hcPsi), i );
		#endif
		#ifdef PSI_GONZALO
			positionAux=getGonzaloPsiValue(&(myicsa->gcPsi), i);
		#endif
		#ifdef PSI_DELTACODES
			positionAux=getDeltaPsiValue(&(myicsa->dcPsi), i);
		#endif
		return	positionAux;
}

	// returns buffer[] <-- psi[ini..end]
void getPsiBuffericsa(void *index, uint *buffer, size_t ini, size_t end) {
	ticsa *myicsa = (ticsa *) index;
		#ifdef PSI_HUFFMANRLE
			getHuffmanPsiValueBuffer(&(myicsa->hcPsi), buffer,ini,end);
		#endif
		#ifdef PSI_GONZALO
			getGonzaloPsiValueBuffer(&(myicsa->gcPsi), buffer,ini,end);
		#endif
		#ifdef PSI_DELTACODES
			getDeltaPsiValueBuffer(&(myicsa->dcPsi), buffer,ini,end);
		#endif
	return;
}



	//recovers the sequence of integers kept by the ICSA. (including the terminator)
void dumpICSASequence(void *index, uint **data, size_t *len) {
	ticsa *icsa = (ticsa *) index;
	size_t n = (icsa->suffixArraySize-1);

	uint * buffer = (uint *) malloc (sizeof(uint) * (icsa->suffixArraySize ) );
	uint * psibuffer = (uint *) malloc (sizeof(uint) * (icsa->suffixArraySize ) );

	//psibuffer[0]=getPsiicsa(icsa,0);
	//getHuffmanPsiValueBuffer(&(icsa->hcPsi),&psibuffer[1],1,icsa->suffixArraySize-1);
	//getHuffmanPsiValueBuffer(&(icsa->hcPsi),psibuffer,0,icsa->suffixArraySize-1);
	 getPsiBuffericsa(index, psibuffer,0,icsa->suffixArraySize-1);

	size_t i;
	size_t z=0;
	uint p;
	for (i=0;i<n;i++) {
		p=i;
		buffer[z++] = getRankicsa(icsa,p) -1;  // !! entrada en el vocab
		//p=getPsiicsa(icsa,p);
		p=psibuffer[p];
	}

	uint value = getRankicsa(icsa,icsa->suffixArraySize-1);
	buffer[z++] = value-1;   // !! entrada en el vocab

	free(psibuffer);
	*data = buffer;
	*len = z;
}

	//recovers the sequence of integers kept by the ICSA. (including the terminator)
void dumpICSASequence_slow_lessmem(void *index, uint **data, size_t *len) {
	ticsa *icsa = (ticsa *) index;
	size_t n = (icsa->suffixArraySize-1);

	uint * buffer = (uint *) malloc (sizeof(uint) * (icsa->suffixArraySize ) );

	size_t i;
	size_t z=0;
	uint p;
	for (i=0;i<n;i++) {
		p=i;
		buffer[z++] = getRankicsa(icsa,p) -1;  // !! entrada en el vocab
		p=getPsiicsa(icsa,p);
	}

	uint value = getRankicsa(icsa,icsa->suffixArraySize-1);
	buffer[z++] = value-1;   // !! entrada en el vocab

	*data = buffer;
	*len = z;
}











// Mostra as estructuras creadas
void showStructsCSA(ticsa *myicsa) {

	unsigned int index;

	// ESTRUCTURAS PARA CSA
	printf("Basic CSA structures:\n\n");

	// VALORES DA FUNCI�N PSI (decodificando)
	printf("\tPSI: (Sampling period = %d)\n", myicsa->T_Psi);
	for(index=0; index < myicsa->suffixArraySize; index++)
		#ifdef PSI_HUFFMANRLE
			printf("\tPsi[%d] = %d\n", index, getHuffmanPsiValue(&(myicsa->hcPsi),index));
		#endif
		#ifdef PSI_GONZALO
			printf("\tPsi[%d] = %d\n", index, getGonzaloPsiValue(&(myicsa->gcPsi),index));
		#endif
		#ifdef PSI_DELTACODES
			printf("\tPsi[%d] = %d\n", index, getDeltaPsiValue(&(myicsa->dcPsi),index));
		#endif
	printf("\n");

	// VECTOR D DE SADAKANE CO DIRECTORIO DE RANK ASOCIADO
	printf("\tD = ");
	showBitVector(myicsa->D, myicsa->suffixArraySize);
	printf("\n\nSuperbloques de D:\n");
	{	uint ns;
		uint nb;
		ns = myicsa->bD->sSize;
		nb= myicsa->bD->bSize;
		for(index=0; index<ns; index++) {
			//printf("\tDs[%d] = %d\n", index, Dir.Ds[index]);
			printf("\tDs[%d] = %d\n", index, myicsa->bD->sdata[index]);
		}
		printf("\nBloques de D:\n");

		for(index=0; index<nb; index++) {
			//printf("\tDb[%d] = %d\n", index, Dir.Db[index]);
			printf("\tDb[%d] = %d\n", index, myicsa->bD->bdata[index]);
		}
		printf("\n\n");
	}
	// ESTRUCTURAS PARA ACCEDER O ARRAY DE SUFIXOS E A SUA INVERSA
//@@	printf("Suffix Array Sampling Structures: (Sampling period = %d)\n", myicsa->T_A);
//@@	printf("\tSuffix Array Samples:\n");
//@@	for(index=0; index < myicsa->samplesASize; index++)
//@@		printf("\tSamplesA[%d] = %d\n", index, myicsa->samplesA[index]);
//@@	printf("\n");
//@@	printf("\tInverse Suffix Array Samples:\n");
//@@	for(index=0; index < myicsa->samplesASize; index++)
//@@		printf("\tSamplesAInv[%d] = %d\n", index, myicsa->samplesAInv[index]);
//@@	printf("\n");

}


// Comparacion de Sadakane entre un patron (pattern) y el sufijo en la posicion p del array de sufijos
// IMPORTANTE EVITAR ULTIMA CHAMADA A PSI
int SadCSACompare(ticsa *myicsa, uint *pattern, uint patternSize, uint p) {

	register unsigned int j, i, currentInteger, diff;

	i = p;
	j = 0;

	while(1) {
		currentInteger = brank(myicsa->bD, i) - 1;
		diff = pattern[j++] - currentInteger;
		if(diff) return diff;
		if(j == patternSize) return 0;
		else
			#ifdef PSI_HUFFMANRLE
				i=getHuffmanPsiValue(&(myicsa->hcPsi),i);
			#endif
			#ifdef PSI_GONZALO
				i=getGonzaloPsiValue(&(myicsa->gcPsi),i);
			#endif
			#ifdef PSI_DELTACODES
				i=getDeltaPsiValue(&(myicsa->dcPsi),i);
			#endif
	}

}




// Initializes the parameters of the index.
uint parametersCSA(ticsa *myicsa, char *build_options){
	char delimiters[] = " =;";
	int j,num_parameters;
	char ** parameters;
	int ssPsi,nsHuff,psiSearchFactor;

	ssPsi  = DEFAULT_PSI_SAMPLING_PERIOD;
	nsHuff = DEFAULT_nsHUFF;
	psiSearchFactor = DEFAULT_PSI_BINARY_SEARCH_FACTOR;

	if (build_options != NULL) {
	parse_parameters(build_options,&num_parameters, &parameters, delimiters);
	for (j=0; j<num_parameters;j++) {

		if ((strcmp(parameters[j], "sPsi") == 0 ) && (j < num_parameters-1) ) {
			ssPsi=atoi(parameters[j+1]);
		}
		if ((strcmp(parameters[j], "nsHuff") == 0 ) && (j < num_parameters-1) ) {
			nsHuff=atoi(parameters[j+1]);
			nsHuff *=1024;
		}
		if ((strcmp(parameters[j], "psiSF") == 0 ) && (j < num_parameters-1) ) {
			psiSearchFactor=atoi(parameters[j+1]);
		}
		j++;
	}
	free_parameters(num_parameters, &parameters);
	}

	myicsa->T_Psi = ssPsi;
	myicsa->tempNSHUFF = nsHuff;
	myicsa->psiSearchFactorJump = psiSearchFactor * ssPsi;

	printf("\n\t parameters for iCSA: samplePsi=%d", ssPsi);
	printf("\n\t              : nsHuff=%d, psiSearchFactor = %d --> jump = %d", nsHuff,psiSearchFactor, myicsa->psiSearchFactorJump);
	return num_parameters;
}
