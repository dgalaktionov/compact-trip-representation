/* psiHuffmanRLE.c
 * Copyright (C) 2011, Antonio Fariña and Eduardo Rodriguez, all rights reserved.
 * Improved representation based on a previous version from Gonzalo Navarro's.
 *
 * psiHuffmanRLE.c: Compressed Representation for Psi in CSA.
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

#include "psiHuffmanRLE.h"

// IMPLEMENTACION DAS FUNCIONS

void destroyHuffmanCompressedPsi(HuffmanCompressedPsi *compressedPsi) {
	freeHuff(compressedPsi->diffsHT);
	free(compressedPsi->samples);
	free(compressedPsi->samplePointers);
	free (compressedPsi->stream);
}

HuffmanCompressedPsi huffmanCompressPsi(unsigned int *Psi, size_t psiSize, unsigned int T, unsigned int nS) {
	
	HuffmanCompressedPsi cPsi;
	
	uint absolute_value;
	register size_t index;
	register size_t ptr, samplesPtr, samplePointersPtr;
	unsigned int runLenght, binaryLenght;
	
	ssize_t *diffs;	
	unsigned int *huffmanDst;
	
	// Estructuras da funcion comprimida (para logo asignar)
	// Tam�n se podian almacenar directamente
	THuff diffsHT;
	size_t numberOfSamples;
	unsigned int *samples;

	unsigned int sampleSize;
	size_t *samplePointers;
	
	unsigned int pointerSize;
	unsigned int *stream;
	size_t streamSize;
	
	// Variables que marcan os intervalos dentro do vector de frecuencias
	unsigned int runLenghtStart = nS - 64 - T; 	// Inicio das Runs
	unsigned int negStart = nS - 64;			// Inicio dos Negativos
	unsigned int bigStart = nS - 32;			// Inicio dos Grandes (>runLenghtStart)
	
	// Para estadistica
	size_t totalSize;
	
	// Reservamos espacio para a distribuci�n de valores de Psi
	printf("\nMALLOC FOR %u\n", nS);
	huffmanDst = (unsigned int *)malloc(sizeof(int)*nS);
	for(index=0;index<nS;index++) huffmanDst[index]=0;

	
	// Inicializamos diferencias	
	printf("\nMALLOC FOR %u\n", psiSize);
	diffs = (ssize_t *)malloc(sizeof(ssize_t)*psiSize);	
	if (!diffs) {
		printf("\n unable to allocate %zu bytes\n",sizeof(ssize_t)*psiSize);
		exit(0);
	}

	
	diffs[0] = 0;
	for(index=1; index<psiSize; index++) 
		diffs[index] = ((ssize_t)Psi[index]) - ((ssize_t)Psi[index-1]);
		
	// Calculamos a distribucion de frecuencias
	runLenght = 0;
	for(index=0; index<psiSize; index++) {

		if(index%T) {
			
			if(diffs[index]== ((ssize_t) 1) ) {
				runLenght++;
			} else {	// Non estamos nun run
				if(runLenght) {
					huffmanDst[runLenght+runLenghtStart]++;
					runLenght = 0;
				}
				if(diffs[index]>((ssize_t)1) && diffs[index]<runLenghtStart) 
					huffmanDst[diffs[index]]++;
				else
					if(diffs[index]< ((ssize_t) 0) ) {	// Valor negativo
						absolute_value = (uint) (-diffs[index]);
						binaryLenght = bits(absolute_value);
						
						if (binaryLenght > 32)
							printf("\n OJO, que absolute_value vale %u",absolute_value);
							
						huffmanDst[binaryLenght+negStart-1]++;
					} else {				// Valor grande >= 128
						absolute_value = (uint)(diffs[index]);
						binaryLenght = bits(absolute_value);

						if (binaryLenght > 32)
							printf("\n OJO, que absolute_value vale %u",absolute_value);

						huffmanDst[binaryLenght+bigStart-1]++;
					}
			}
			
		} else { // Rompemos o run porque atopamos unha mostra
			if(runLenght) {
				huffmanDst[runLenght+runLenghtStart]++;
				runLenght = 0;
			}
		}
		
	}
		
	if(runLenght) huffmanDst[runLenght+runLenghtStart]++;
	
	// Creamos o arbol de Huffman
	diffsHT = createHuff(huffmanDst,nS-1,UNSORTED);
	
	// Calculamos o espacio total ocupado pola secuencia Huffman + RLE
	streamSize = (size_t) diffsHT.total;
	fprintf(stderr,"\n\t\t [3] diffsHT.total = %zu bits ", streamSize);

/******2014.03.12... bug del optimizador de gcc ?? ****/
// ok con gcc -O0
// falla con gcc -O3

/*
//Si quito if-printf, el optimizador (-O3) hace que este código falle xD!

size_t prev_streamsize = streamSize;
	for(index=negStart;index<bigStart;index++) {
		if (!(index %19)) 
			printf("\n \t\t  %ld , %ld,  %ld , resta= %ld", negStart, index, huffmanDst[index], (index-negStart+1));
		streamSize += ((size_t)huffmanDst[index])*(index-negStart+1);	// Negativos
	}
	printf("\n\t\t [2]streamSize = %ld , index = %ld, incremento = %ld",streamSize, index, streamSize - prev_streamsize);

	for(index=bigStart;index<nS;index++) {
		if (!(index %21)) 
			printf("\n \t\t  %ld , %ld,  %ld , resta= %ld", negStart, index, huffmanDst[index], (index-bigStart+1));
		streamSize += ((size_t)huffmanDst[index])*(index-bigStart+1);		// Grandes	
	}
	printf("\n\t\t [3]streamSize = %ld , index = %ld",streamSize, index);
	fflush(stdout);
	
*/

//versión que funciona OK		
	for(index=negStart;index<nS;index++)  { 
		if (index < bigStart)
			streamSize += ((size_t)huffmanDst[index])*(index-negStart+1);	// Negativos
		else 
			streamSize += ((size_t)huffmanDst[index])*(index-bigStart+1);   // Grandes	
	}	
	fprintf(stderr,"\n\t\t [3]streamSize = %ld , index = %ld",streamSize, index);

/**********/
	
	// Calculamos o numero de mostras e o espacio ocupado por cada mostra e por cada punteiro
	numberOfSamples = (psiSize+T-1)/T;	
	sampleSize = bits(psiSize);



// ********************************************************************************************
//@@fari!!!!xD!!!  Necesario sólo porque comprimimos el vocabulario del graph-index con esto: ver UNMAP[]
	uint sampleSize2 = bits(Psi[psiSize-1]);	
	if (sampleSize2>sampleSize) sampleSize=sampleSize2;
// ********************************************************************************************

	
	pointerSize = bits(streamSize);	
	fprintf(stderr,"\n psi: pointersize = %u bits, sampleSize=%u bits", pointerSize,sampleSize);

fprintf(stderr,"\n espacio para Sample-values-psi = %lu bytes", sizeof(uint)*(((size_t)numberOfSamples*sampleSize+W-1)/W));
fprintf(stderr,"\n espacio para Sample-values-psi** = %lu bytes", sizeof(uint)*(((size_t)numberOfSamples*sampleSize+W-1)/W));
	// Reservamos espacio para a secuencia e para as mostras e punteiros
	samples = (unsigned int *)malloc(sizeof(uint)*(((size_t)numberOfSamples*sampleSize+W-1)/W));	
		samples[(((size_t)numberOfSamples*sampleSize+W-1)/W)-1] =0000; //initialized only to avoid valgrind warnings
		

fprintf(stderr,"\n espacio para Sample-pointers-psi = %lu bytes", sizeof(size_t)* (ulong_len(pointerSize,numberOfSamples)) );
		
	samplePointers = (size_t *)malloc(sizeof(size_t)* (ulong_len(pointerSize,numberOfSamples)) );
		samplePointers[ (ulong_len(pointerSize,numberOfSamples)) -1] = 00000000;  //initialized only to avoid valgrind warnings

//printf("\n\n**** TEST_ULONLEN = %lu, normal = %lu", sizeof(size_t)* (ulong_len(pointerSize,numberOfSamples)) ,
//											(((size_t)numberOfSamples*pointerSize+WW-1)/WW)*sizeof(size_t));
		
	stream = (unsigned int *)malloc(sizeof(int)*((streamSize+W-1)/W));
		stream[((streamSize+W-1)/W)-1]=0000;//initialized only to avoid valgrind warnings
	
fprintf(stderr,"\n espacio para stream-psi = %lu bytes", sizeof(int)*((streamSize+W-1)/W) );
fflush(stderr);fflush(stdout);
	
	// Comprimimos secuencialmente (haber� que levar un punteiro desde o inicio)
	ptr = 0;
	samplesPtr = 0;
	samplePointersPtr = 0;
	runLenght = 0;
	for(index=0; index<psiSize; index++) {
		
		if(index%T) {
			
			if(diffs[index]==((ssize_t)1)) {
				runLenght++;
			} else {	// Non estamos nun run
				if(runLenght) {
					ptr = encodeHuff(diffsHT,runLenght+runLenghtStart,stream,ptr);
					runLenght = 0;
				}
				if(diffs[index]>((ssize_t)1) && diffs[index]<runLenghtStart) {				
					ptr = encodeHuff(diffsHT,(uint)diffs[index],stream,ptr);	
				}	
				else
					if(diffs[index]< ((ssize_t)0) ) {	// Valor negativo
						absolute_value = (uint) (-diffs[index]);
						binaryLenght = bits(absolute_value);
						ptr = encodeHuff(diffsHT,binaryLenght+negStart-1,stream,ptr);
						bitwrite(stream,ptr,binaryLenght,absolute_value);
						
					/**/	uint tmp = bitread(stream,ptr,binaryLenght);			
					/**/	assert (tmp == absolute_value);	
						
						ptr += binaryLenght;						
					} else {				// Valor grande >= 128
						absolute_value = (uint) diffs[index];
						binaryLenght = bits(absolute_value);					
						ptr = encodeHuff(diffsHT,binaryLenght+bigStart-1,stream,ptr);
						bitwrite(stream,ptr,binaryLenght,absolute_value);
					/**/	uint tmp = bitread(stream,ptr,binaryLenght);			
					/**/	assert (tmp == absolute_value);							
						ptr += binaryLenght;
					}
			}
			
		} else { // Rompemos o run porque atopamos unha mostra
			if(runLenght) {				
				ptr = encodeHuff(diffsHT,runLenght+runLenghtStart,stream,ptr);
				runLenght = 0;
			}
			bitwrite(samples,samplesPtr,sampleSize, Psi[index]);
			/**/uint tmp = bitread(samples,samplesPtr,sampleSize);			
			/**/assert (tmp == Psi[index]);		
			
			samplesPtr += sampleSize;
			
			bitwrite64(samplePointers,samplePointersPtr,pointerSize,ptr);
			/**/size_t tmp2 = bitread64(samplePointers,samplePointersPtr,pointerSize);
			/**/assert (tmp2 == ptr);					
			samplePointersPtr += pointerSize;

			



					/*  //Chequear que bitwrite/bitread funcionan OK
						{{
									
									uint tmp = bitread(samples,samplesPtr,sampleSize);
									
									if (tmp != Psi[index]) {
										printf("\n bitwrite32/bitread32 failed: value src= %u value recovered = %u, k-bits=%u, offset = %u/%u",				          
																							   Psi[index],tmp, sampleSize,samplesPtr,(numberOfSamples*sampleSize+W-1));
										//checkBitwriteBitRead(samples, samplesPtr, 70000000);
										
										checkBitwriteBitReadValue(samples, samplesPtr,67120000);
										checkBitwriteBitReadValue(samples, 0,67120000);
																							   
										bitwrite(samples,samplesPtr,sampleSize, 67120000);
										uint tmp = bitread(samples,samplesPtr,sampleSize);
										printf("\n bitwrite32/bitread32 failed: value src= %u value recovered = %u, k-bits=%u, offset = %u/%u",				          
																							   67120000,tmp, sampleSize,samplesPtr,(numberOfSamples*sampleSize+W-1));





										exit(0);
										counterrors ++;                                                       
										if (counterrors > 20) exit(0);
									}
									//assert(tmp == Psi[index]);
						}}
						*/
			
		}
		
	}
	
	if(runLenght) {	
		ptr = encodeHuff(diffsHT,runLenght+runLenghtStart,stream,ptr);
	}
	
	// Amosamos o espacio ocupado
	totalSize = ((size_t) sizeof(HuffmanCompressedPsi)) + 
		sizeof(int)*((size_t)(numberOfSamples*sampleSize+W-1)/W) + 
		sizeof(size_t)*((size_t)(numberOfSamples*pointerSize+WW-1)/WW) +
		sizeof(int)*((size_t)(streamSize+W-1)/W) + 
		sizeHuff(diffsHT);
		
		fprintf(stderr,"\n@@@@@@@@@ psi samaplePeriod= %u, ns=%u ", T,nS);
		fprintf(stderr,"\n@@@@@@@@@ psi size= [samples = %lu] bytes ", sizeof(int)*((size_t)(numberOfSamples*sampleSize+W-1)/W));
		fprintf(stderr,"\n@@@@@@@@@ psi size= [pointers = %lu] bytes ",sizeof(size_t)*((size_t)(numberOfSamples*pointerSize+WW-1)/WW));
		fprintf(stderr,"\n@@@@@@@@@ psi size= [totalsize diffsHt.total  = %lu] bits \n",diffsHT.total);		
		fprintf(stderr,"\n@@@@@@@@@ psi size= [streamsize+largevalues =%lu] bytes ", sizeof(int)*((size_t)(streamSize+W-1)/W));
		fprintf(stderr,"\n@@@@@@@@@ psi size= [sizeHuff tree = %lu] bytes ", (ulong)sizeHuff(diffsHT));		
		
		fflush(stdout);fflush(stderr);

	//@@printf("\n\t Compressed Psi size = %zu bytes, with %d different symbols.", totalSize, nS);
	
	// Necesario antes de decodificar
	prepareToDecode(&diffsHT);
	
	// Asignamos os valores a cPsi e devolvemolo
	cPsi.T = T;
	cPsi.diffsHT = diffsHT;
	cPsi.nS = nS;
	cPsi.numberOfSamples = numberOfSamples;
	cPsi.samples = samples;
	cPsi.sampleSize = sampleSize;
	cPsi.samplePointers = samplePointers;
	cPsi.pointerSize = pointerSize;
	cPsi.stream = stream;
	cPsi.streamSize = streamSize;
	cPsi.totalMem = totalSize;
	
/*
// *
{ ////////////////////////////// CHECKS IT WORKED ///////////////////////////////////////////////////////
	fprintf(stderr,"\n Test compress/uncompress PSI is starting for all i in Psi[0..psiSize-1], "); fflush(stdout); fflush(stderr);
	size_t i;
	uint val1,val2;	
	fflush(stdout);
	
	uint count=0;printf("\n");
	for (i=0; i<psiSize; i++) {
			val1= getHuffmanPsiValue(&cPsi, i);
			val2=Psi[i];
			if (val1 != val2) { count++;
					fprintf(stderr,"\n i=%zu,psi[i] vale (compressed = %u) <> (original= %u), ",i, val1,val2);
					fprintf(stderr,"\n i=%zu,diffs[i] = %ld ",i, (long)diffs[i]); fflush(stdout);fflush(stderr);
					if (count > 100) {i= i/0; break;}
			}
			
		if(i%500000==0) fprintf(stderr, "\t Testing compress/uncompress psi: %.1f%%\r", (float)i/psiSize*100);	
	}
		fprintf(stderr, "\t Testing compress/uncompress psi: %.1f%%", (float)i/psiSize*100);
		fprintf(stderr,"\n Test compress/uncompress PSI passed *OK*, "); fflush(stdout); fflush(stderr);
} /////////////////////////////////////////////////////////////////////////////////////
// *
*/
	
	//frees resources not needed in advance
	free(diffs);
	free(huffmanDst);
	
	//returns the data structure that holds the compressed psi.
	return cPsi;	
}



unsigned long  getHuffmanPsiSizeBitsUptoPosition(HuffmanCompressedPsi *cPsi, size_t position) {
	
	register size_t index;
	size_t sampleIndex, positionsSinceSample, ptr;

	unsigned int psiValue, absolute_value, huffmanCode; 
	unsigned int binaryLenght, runLenght;
	
	unsigned int runLenghtStart = cPsi->nS - 64 - cPsi->T;
	unsigned int negStart = cPsi->nS - 64;
	unsigned int bigStart = cPsi->nS - 32;	
	
	sampleIndex = position / cPsi->T;
	psiValue = bitread(cPsi->samples,sampleIndex*cPsi->sampleSize,cPsi->sampleSize);
	ptr = bitread64(cPsi->samplePointers,sampleIndex*cPsi->pointerSize,cPsi->pointerSize);
	
	positionsSinceSample = position%cPsi->T;
	
	for(index=0;index<positionsSinceSample;index++) {
		
		ptr = decodeHuff(&cPsi->diffsHT,&huffmanCode,cPsi->stream,ptr);
		
		if(huffmanCode < runLenghtStart) { 	// Incremento directo
			psiValue += huffmanCode;
		}	
		else 
			if(huffmanCode < negStart) {	// Estamos nun run
				runLenght = huffmanCode - runLenghtStart;
				if(index+runLenght>=positionsSinceSample)
					return psiValue+positionsSinceSample-index;
				else {
					psiValue += runLenght;
					index += runLenght-1;
				}
			}
			else
				if(huffmanCode < bigStart) {	// Negativo
					binaryLenght = huffmanCode-negStart+1;
					absolute_value = bitread(cPsi->stream,ptr,binaryLenght);
					ptr += binaryLenght;
					psiValue -= absolute_value;	
				}
				else {	// Grande
					binaryLenght = huffmanCode-bigStart+1;
					absolute_value = bitread(cPsi->stream,ptr,binaryLenght);
					ptr += binaryLenght;
					psiValue += absolute_value;				 
				}				
	}
	
	 
	
	return (unsigned long) (ptr + sampleIndex * (cPsi->sampleSize + cPsi->pointerSize) );

}


unsigned int getHuffmanPsiValue(HuffmanCompressedPsi *cPsi, size_t position) {
	
	register size_t index;
	size_t sampleIndex, positionsSinceSample, ptr;

	unsigned int psiValue, absolute_value, huffmanCode; 
	unsigned int binaryLenght, runLenght;
	
	unsigned int runLenghtStart = cPsi->nS - 64 - cPsi->T;
	unsigned int negStart = cPsi->nS - 64;
	unsigned int bigStart = cPsi->nS - 32;	
	
	sampleIndex = position / cPsi->T;
	psiValue = bitread(cPsi->samples,sampleIndex*cPsi->sampleSize,cPsi->sampleSize);
	ptr = bitread64(cPsi->samplePointers,sampleIndex*cPsi->pointerSize,cPsi->pointerSize);
	
	positionsSinceSample = position%cPsi->T;
	
	for(index=0;index<positionsSinceSample;index++) {
		
		ptr = decodeHuff(&cPsi->diffsHT,&huffmanCode,cPsi->stream,ptr);
		
		if(huffmanCode < runLenghtStart) { 	// Incremento directo
			psiValue += huffmanCode;
		}	
		else 
			if(huffmanCode < negStart) {	// Estamos nun run
				runLenght = huffmanCode - runLenghtStart;
				if(index+runLenght>=positionsSinceSample)
					return psiValue+positionsSinceSample-index;
				else {
					psiValue += runLenght;
					index += runLenght-1;
				}
			}
			else
				if(huffmanCode < bigStart) {	// Negativo
					binaryLenght = huffmanCode-negStart+1;
					absolute_value = bitread(cPsi->stream,ptr,binaryLenght);
					ptr += binaryLenght;
					psiValue -= absolute_value;	
				}
				else {	// Grande
					binaryLenght = huffmanCode-bigStart+1;
					absolute_value = bitread(cPsi->stream,ptr,binaryLenght);
					ptr += binaryLenght;
					psiValue += absolute_value;				 
				}				
	}
	
	return psiValue;

}

void getHuffmanPsiValueBuffer_1(HuffmanCompressedPsi *cPsi, uint *buffer, size_t ini, size_t end) {
	size_t i;
	for (i=ini; i<=end;i++)
		*buffer++ = getHuffmanPsiValue(cPsi,i);
	return;
}
	

/*   //VERSION SPIRE 2014. WITH PROBLEMS WITH RUNS
void getHuffmanPsiValueBuffer(HuffmanCompressedPsi *cPsi, uint *buffer, size_t ini, size_t end) {
	
	register size_t index;
	size_t sampleIndex, positionsSinceSample, ptr;

	unsigned int psiValue, absolute_value, huffmanCode; 
	unsigned int binaryLenght, runLenght;
	
	unsigned int runLenghtStart = cPsi->nS - 64 - cPsi->T;
	unsigned int negStart = cPsi->nS - 64;
	unsigned int bigStart = cPsi->nS - 32;	

	size_t position = ini;
	sampleIndex = position / cPsi->T;
	psiValue = bitread(cPsi->samples,sampleIndex*cPsi->sampleSize,cPsi->sampleSize);
	ptr = bitread64(cPsi->samplePointers,sampleIndex*cPsi->pointerSize,cPsi->pointerSize);
	
	positionsSinceSample = position%cPsi->T;
	
	for(index=0;index<positionsSinceSample;index++) {
		
		ptr = decodeHuff(&cPsi->diffsHT,&huffmanCode,cPsi->stream,ptr);
		
		if(huffmanCode < runLenghtStart) { 	// Incremento directo
			psiValue += huffmanCode;
		}	
		else 
			if(huffmanCode < negStart) {	// Estamos nun run
				runLenght = huffmanCode - runLenghtStart;
				if(index+runLenght>=positionsSinceSample){
					//return psiValue+positionsSinceSample-index;
					psiValue = psiValue+positionsSinceSample-index;
					break;
				}					
				else {
					psiValue += runLenght;
					index += runLenght-1;
				}
			}
			else
				if(huffmanCode < bigStart) {	// Negativo
					binaryLenght = huffmanCode-negStart+1;
					absolute_value = bitread(cPsi->stream,ptr,binaryLenght);
					ptr += binaryLenght;
					psiValue -= absolute_value;	
				}
				else {	// Grande
					binaryLenght = huffmanCode-bigStart+1;
					absolute_value = bitread(cPsi->stream,ptr,binaryLenght);
					ptr += binaryLenght;
					psiValue += absolute_value;				 
				}				
	}	
	*buffer++= psiValue;  //value psi(ini)
	
	size_t cpsiT = cPsi->T;	
	for (position = ini+1; position <= end ;  ) {
		if (!(position%cpsiT)){ // a sampled value
			sampleIndex = position / cpsiT;
			psiValue = bitread(cPsi->samples,sampleIndex*cPsi->sampleSize,cPsi->sampleSize);
			//
			//size_t ptrnew = bitread64(cPsi->samplePointers,sampleIndex*cPsi->pointerSize,cPsi->pointerSize);
			//assert (ptr == ptrnew); //ptr is the same position as in the first loop
			//if (ptr != ptrnew) {
			//	printf("\n ptr = %lu and ptrnew = %lu do not match!\n", (ulong) ptr, (ulong) ptrnew);
			//}
		}
		else {
		
			ptr = decodeHuff(&cPsi->diffsHT,&huffmanCode,cPsi->stream,ptr);
			
			if(huffmanCode < runLenghtStart) { 	// Incremento directo
				psiValue += huffmanCode;
			}	
			else 
				if(huffmanCode < negStart) {	// Estamos nun run
					runLenght = huffmanCode - runLenghtStart;
	        	//	{uint l;
				//		if (runLenght > (end-position)) 
				//			runLenght = end-position;
				//		for (l=0;l<runLenght;l++) {
				//			psiValue++;
				//			*buffer++=psiValue;							
				//		}
				//		position +=runLenght;
				//		continue;   //skips the end of the loop (ten lines below)
				//	}					 						
					
					
					{uint l;
						if (runLenght > (end-position)) 
							runLenght = end-position+1;   //@corregido 2014.05.01 instead of "end-position" xD!
						for (l=0;l<runLenght;l++) {
							psiValue++;
							*buffer++=psiValue;							
						}
						position +=runLenght;
						continue;   //skips the end of the loop (ten lines below)
					}						 
				}
				else
					if(huffmanCode < bigStart) {	// Negativo
						binaryLenght = huffmanCode-negStart+1;
						absolute_value = bitread(cPsi->stream,ptr,binaryLenght);
						ptr += binaryLenght;
						psiValue -= absolute_value;	
					}
					else {	// Grande
						binaryLenght = huffmanCode-bigStart+1;
						absolute_value = bitread(cPsi->stream,ptr,binaryLenght);
						ptr += binaryLenght;
						psiValue += absolute_value;				 
					}				
			
		}
		*buffer++ = psiValue;
		position ++;
	} 
			
	return;
}
*/


/**/  // version corrected 2015 (problems with runs).
void getHuffmanPsiValueBuffer(HuffmanCompressedPsi *cPsi, uint *buffer, size_t ini, size_t end) {
	
	register size_t index;
	size_t sampleIndex, positionsSinceSample, ptr;

	unsigned int psiValue, absolute_value, huffmanCode; 
	unsigned int binaryLenght, runLenght;
	
	unsigned int runLenghtStart = cPsi->nS - 64 - cPsi->T;
	unsigned int negStart = cPsi->nS - 64;
	unsigned int bigStart = cPsi->nS - 32;	

	size_t position = ini;
	sampleIndex = position / cPsi->T;
	psiValue = bitread(cPsi->samples,sampleIndex*cPsi->sampleSize,cPsi->sampleSize);
	ptr = bitread64(cPsi->samplePointers,sampleIndex*cPsi->pointerSize,cPsi->pointerSize);
	
	positionsSinceSample = position%cPsi->T;
	
	int inRun=0;//@@
	int inRunRemain=0;//@@
	
	for(index=0;index<positionsSinceSample;index++) {
		
		ptr = decodeHuff(&cPsi->diffsHT,&huffmanCode,cPsi->stream,ptr);
		
		if(huffmanCode < runLenghtStart) { 	// Incremento directo
			psiValue += huffmanCode;
		}	
		else 
			if(huffmanCode < negStart) {	// Estamos nun run
				runLenght = huffmanCode - runLenghtStart;
				if(index+runLenght>=positionsSinceSample){
					//return psiValue+positionsSinceSample-index;
					psiValue = psiValue+ positionsSinceSample-index;
					inRun=1;//@@
					inRunRemain = runLenght + index-positionsSinceSample;//@@
					break;
				}					
				else {
					psiValue += runLenght;
					index += runLenght-1;
				}
			}
			else
				if(huffmanCode < bigStart) {	// Negativo
					binaryLenght = huffmanCode-negStart+1;
					absolute_value = bitread(cPsi->stream,ptr,binaryLenght);
					ptr += binaryLenght;
					psiValue -= absolute_value;	
				}
				else {	// Grande
					binaryLenght = huffmanCode-bigStart+1;
					absolute_value = bitread(cPsi->stream,ptr,binaryLenght);
					ptr += binaryLenght;
					psiValue += absolute_value;				 
				}				
	}	
	*buffer++= psiValue;  //value psi(ini)
	
	//@@ //--- AVOIDS PROBLES IN RUN... BUG FIXED on 2014.11.20
	if (inRun) {
		int iters = ((end-ini) >inRunRemain)? inRunRemain: (end-ini);
		ini+=iters;
		
		while (iters>0) {		
			psiValue++;	
			*buffer++= psiValue;
			iters --;
		}
		
	}
	//@@
	
	size_t cpsiT = cPsi->T;	
	for (position = ini+1; position <= end ;  ) {

		if (!(position%cpsiT)){ // a sampled value
			sampleIndex = position / cpsiT;
			psiValue = bitread(cPsi->samples,sampleIndex*cPsi->sampleSize,cPsi->sampleSize);
			//
			//size_t ptrnew = bitread64(cPsi->samplePointers,sampleIndex*cPsi->pointerSize,cPsi->pointerSize);
			//assert (ptr == ptrnew); //ptr is the same position as in the first loop
			//if (ptr != ptrnew) {
			//	printf("\n ptr = %lu and ptrnew = %lu do not match!\n", (ulong) ptr, (ulong) ptrnew);
			//}
		}
		else {
		
			ptr = decodeHuff(&cPsi->diffsHT,&huffmanCode,cPsi->stream,ptr);
			
			if(huffmanCode < runLenghtStart) { 	// Incremento directo
				psiValue += huffmanCode;
			}	
			else 
				if(huffmanCode < negStart) {	// Estamos nun run
					runLenght = huffmanCode - runLenghtStart;
	/*				{uint l;
						if (runLenght > (end-position)) 
							runLenght = end-position;
						for (l=0;l<runLenght;l++) {
							psiValue++;
							*buffer++=psiValue;							
						}
						position +=runLenght;
						continue;   //skips the end of the loop (ten lines below)
					}					 						
					*/
					
					{uint l;
						if (runLenght > (end-position)) 
							runLenght = end-position+1;   //@corregido 2014.05.01 instead of "end-position" xD!
						for (l=0;l<runLenght;l++) {
							psiValue++;
							*buffer++=psiValue;							
						}
						position +=runLenght;
						continue;   //skips the end of the loop (ten lines below)
					}						 
				}
				else
					if(huffmanCode < bigStart) {	// Negativo
						binaryLenght = huffmanCode-negStart+1;
						absolute_value = bitread(cPsi->stream,ptr,binaryLenght);
						ptr += binaryLenght;
						psiValue -= absolute_value;	
					}
					else {	// Grande
						binaryLenght = huffmanCode-bigStart+1;
						absolute_value = bitread(cPsi->stream,ptr,binaryLenght);
						ptr += binaryLenght;
						psiValue += absolute_value;				 
					}				
			
		}
		*buffer++ = psiValue;
		position ++;
	} 
			
	return;
}


void storeHuffmanCompressedPsi(HuffmanCompressedPsi *compressedPsi, char *filename) {

	FILE *file;
	THuff H;

	if( (file = fopen(filename, "w")) ==NULL) {
		printf("Cannot open file %s\n", filename);
		exit(0);
	}
	size_t write_err;
	
	write_err=fwrite(&(compressedPsi->T), sizeof(int),1,file);
	// Almacenar o arbol de huffman
	H = compressedPsi->diffsHT;
	write_err=fwrite(  &H.max, sizeof(int),1,file);
	write_err=fwrite(  &H.lim, sizeof(int),1,file);
	write_err=fwrite(  &H.depth, sizeof(int),1,file);
//	write( file, H.s.spos, (H.lim+1)*sizeof(int));
	write_err=fwrite(  H.s.symb,sizeof(int), (H.lim+1),file);	
	write_err=fwrite(  H.num,sizeof(int), (H.depth+1),file);
	write_err=fwrite(  H.fst,sizeof(int), (H.depth+1),file);
	// Fin de almacenar o arbol de huffman
	write_err=fwrite(  &(compressedPsi->nS), sizeof(int),1,file);
	write_err=fwrite(  &(compressedPsi->numberOfSamples), sizeof(size_t),1,file);
	write_err=fwrite(  &(compressedPsi->sampleSize), sizeof(int),1,file);
	write_err=fwrite( 	compressedPsi->samples, sizeof(int), (((size_t)compressedPsi->numberOfSamples*compressedPsi->sampleSize+W-1)/W), file);	
	write_err=fwrite(  &(compressedPsi->pointerSize), sizeof(int),1,file);
	write_err=fwrite( 	compressedPsi->samplePointers, sizeof(size_t), (((size_t)compressedPsi->numberOfSamples*compressedPsi->pointerSize+WW-1)/WW), file);
	write_err=fwrite(  &(compressedPsi->streamSize), sizeof(size_t),1,file);
	write_err=fwrite( 	compressedPsi->stream, sizeof(int),((compressedPsi->streamSize+W-1)/W) , file);
	size_t byteswritentostream = write_err;
	write_err=fwrite(  &(compressedPsi->totalMem), sizeof(size_t),1,file);

/*

	printf("\n\n to save");
	printf("\n compressedPSI->NS = %u",compressedPsi->nS);
	printf("\n numberofsamples = %zu",compressedPsi->numberOfSamples);
	printf("\n samplesize_Bits = %u bits",compressedPsi->sampleSize);
	printf("\n tamaño de array samples (bytes) = %zu", (((size_t)compressedPsi->numberOfSamples*compressedPsi->sampleSize+W-1)/W)*sizeof(int));	
	printf("\n pointersize_Bits = %u bits",compressedPsi->pointerSize);
	printf("\n tamaño de array pointers = %zu (bytes)", (( (size_t)compressedPsi->numberOfSamples*compressedPsi->pointerSize+WW-1)/WW)*sizeof(size_t) );
	printf("\n streamsize (lonxitude en bits) = %zu",compressedPsi->streamSize);
	printf("\n stream (tamaño en bytes) = %zu",	((compressedPsi->streamSize+W-1)/W)*sizeof(int));
	printf("\n      bytes writen for **stream = %zu**",	sizeof(uint)*byteswritentostream);
	
	printf("\n totalMem compressed psi = %zu bytes", compressedPsi->totalMem);
	fflush(stdout);fflush(stderr);
*/
	
	fclose(file);	

}



HuffmanCompressedPsi loadHuffmanCompressedPsi(char *filename) {
	
	HuffmanCompressedPsi compressedPsi;

 	THuff H;
     
	FILE *file;

	if( (file = fopen(filename,"r"))==NULL ) {
		printf("Cannot read file %s\n", filename);
		exit(0);
	}
		fflush(stdout);fflush(stderr);
	
	size_t read_err;
	
	read_err=fread(&(compressedPsi.T), sizeof(int), 1, file);
	// Cargamos o arbol de Huffman
	read_err=fread( &H.max, sizeof(int),1, file);
	read_err=fread( &H.lim, sizeof(int),1, file);
	read_err=fread( &H.depth, sizeof(int),1, file);
	//H.s.spos = (unsigned int *) malloc((H.lim+1)*sizeof(int));
	//H.s.spos =H.s.symb = (unsigned int *) malloc((H.lim+1)*sizeof(int));
	H.s.symb = (unsigned int *) malloc((H.lim+1)*sizeof(int));
	H.num = (unsigned int *) malloc((H.depth+1)*sizeof(int));	
	H.fst = (unsigned int *) malloc((H.depth+1)*sizeof(int));	

	//read(file, H.s.spos, (H.lim+1)*sizeof(int));
	//fprintf(stderr," \n read %d spos bytes\n", (H.lim+1)*sizeof(int));
	read_err=fread( H.s.symb, sizeof(int), (H.lim+1),file);	

	read_err=fread( H.num, sizeof(int), (H.depth+1),file);
	read_err=fread( H.fst, sizeof(int), (H.depth+1),file);	
	compressedPsi.diffsHT = H;
	// Fin da carga do arbol de Huffman
	read_err=fread( &(compressedPsi.nS), sizeof(int), 1,file);
	read_err=fread( &(compressedPsi.numberOfSamples), sizeof(size_t), 1, file);
	
	read_err=fread( &(compressedPsi.sampleSize), sizeof(int), 1,file);
	compressedPsi.samples = (unsigned int *)malloc((( (size_t)compressedPsi.numberOfSamples*compressedPsi.sampleSize+W-1)/W)*sizeof(int));
	read_err=fread( compressedPsi.samples, sizeof(int), (( (size_t)compressedPsi.numberOfSamples*compressedPsi.sampleSize+W-1)/W) , file );
	
	read_err=fread( &(compressedPsi.pointerSize), sizeof(int),1,file);
	compressedPsi.samplePointers = (size_t *)malloc((( (size_t)compressedPsi.numberOfSamples*compressedPsi.pointerSize+WW-1)/WW)*sizeof(size_t));
	read_err=fread( compressedPsi.samplePointers, sizeof(size_t), (( (size_t)compressedPsi.numberOfSamples*compressedPsi.pointerSize+WW-1)/WW), file);
	
	read_err=fread( &(compressedPsi.streamSize), sizeof(size_t),1,file);
	compressedPsi.stream = (unsigned int *)malloc(((compressedPsi.streamSize+W-1)/W)*sizeof(int));
	size_t readbytesstream=fread( compressedPsi.stream, sizeof(uint), ((compressedPsi.streamSize+W-1)/W), file);
	read_err=fread( &(compressedPsi.totalMem), sizeof(size_t),1,file);
	
	
/*

	printf("\n\n to load");
	printf("\n compressedPSI->NS = %u",compressedPsi.nS);
	printf("\n numberofsamples = %zu",compressedPsi.numberOfSamples);
	printf("\n samplesize_Bits = %u bits",compressedPsi.sampleSize);
	printf("\n tamaño de array samples (bytes) = %zu", (((size_t)compressedPsi.numberOfSamples*compressedPsi.sampleSize+W-1)/W)*sizeof(int));	
	printf("\n pointersize_Bits = %u bits",compressedPsi.pointerSize);
	printf("\n tamaño de array pointers = %zu (bytes)", (( (size_t)compressedPsi.numberOfSamples*compressedPsi.pointerSize+WW-1)/WW)*sizeof(size_t) );
	printf("\n streamsize (lonxitude en bits) = %zu",compressedPsi.streamSize);
	printf("\n stream (tamaño en bytes) = %zu",	((compressedPsi.streamSize+W-1)/W)*sizeof(int));
	printf("\n      bytes read for **stream = %zu**",	sizeof(uint)*readbytesstream);
	                                            
	printf("\n totalMem compressed psi = %zu bytes", compressedPsi.totalMem);
	fflush(stdout);fflush(stderr);
*/	

	fclose(file);			
	return compressedPsi;

}



/*

void storeHuffmanCompressedPsi(HuffmanCompressedPsi *compressedPsi, char *filename) {

	int file;
	THuff H;

	if( (file = open(filename, O_WRONLY|O_CREAT, 0700)) < 0) {
		printf("Cannot open file %s\n", filename);
		exit(0);
	}
	ssize_t write_err;
	
	write_err=write(file, &(compressedPsi->T), sizeof(int));
	// Almacenar o arbol de huffman
	H = compressedPsi->diffsHT;
	write_err=write(file, &H.max, sizeof(int));
	write_err=write(file, &H.lim, sizeof(int));
	write_err=write(file, &H.depth, sizeof(int));
//	write(file, H.s.spos, (H.lim+1)*sizeof(int));
	write_err=write(file, H.s.symb, (H.lim+1)*sizeof(int));	
	write_err=write(file, H.num, (H.depth+1)*sizeof(int));
	write_err=write(file, H.fst, (H.depth+1)*sizeof(int));
	// Fin de almacenar o arbol de huffman
	write_err=write(file, &(compressedPsi->nS), sizeof(int));
	write_err=write(file, &(compressedPsi->numberOfSamples), sizeof(size_t));
	write_err=write(file, &(compressedPsi->sampleSize), sizeof(int));
	write_err=write(file,	compressedPsi->samples, (((size_t)compressedPsi->numberOfSamples*compressedPsi->sampleSize+W-1)/W)*sizeof(int));	
	write_err=write(file, &(compressedPsi->pointerSize), sizeof(int));
	write_err=write(file,	compressedPsi->samplePointers, (((size_t)compressedPsi->numberOfSamples*compressedPsi->pointerSize+WW-1)/WW)*sizeof(size_t));
	write_err=write(file, &(compressedPsi->streamSize), sizeof(size_t));
	write_err=write(file,	compressedPsi->stream, ((compressedPsi->streamSize+W-1)/W)*sizeof(int));
	size_t byteswritentostream = write_err;
	write_err=write(file, &(compressedPsi->totalMem), sizeof(size_t));



	printf("\n\n to save");
	printf("\n compressedPSI->NS = %u",compressedPsi->nS);
	printf("\n numberofsamples = %zu",compressedPsi->numberOfSamples);
	printf("\n samplesize_Bits = %u bits",compressedPsi->sampleSize);
	printf("\n tamaño de array samples (bytes) = %zu", (((size_t)compressedPsi->numberOfSamples*compressedPsi->sampleSize+W-1)/W)*sizeof(int));	
	printf("\n pointersize_Bits = %u bits",compressedPsi->pointerSize);
	printf("\n tamaño de array pointers = %zu (bytes)", (( (size_t)compressedPsi->numberOfSamples*compressedPsi->pointerSize+WW-1)/WW)*sizeof(size_t) );
	printf("\n streamsize (lonxitude en bits) = %zu",compressedPsi->streamSize);
	printf("\n stream (tamaño en bytes) = %zu",	((compressedPsi->streamSize+W-1)/W)*sizeof(int));
	printf("\n      bytes writen for **stream = %zu**",	byteswritentostream);
	
	printf("\n totalMem compressed psi = %zu bytes", compressedPsi->totalMem);
	fflush(stdout);fflush(stderr);

	
	close(file);	

}



HuffmanCompressedPsi loadHuffmanCompressedPsi(char *filename) {
	
	HuffmanCompressedPsi compressedPsi;

 	THuff H;
     
	int file;

	if( (file = open(filename, O_RDONLY)) < 0) {
		printf("Cannot read file %s\n", filename);
		exit(0);
	}
		fflush(stdout);fflush(stderr);
	
	ssize_t read_err;
	
	read_err=read(file, &(compressedPsi.T), sizeof(int));
	// Cargamos o arbol de Huffman
	read_err=read(file, &H.max, sizeof(int));
	read_err=read(file, &H.lim, sizeof(int));
	read_err=read(file, &H.depth, sizeof(int));
	//H.s.spos = (unsigned int *) malloc((H.lim+1)*sizeof(int));
	//H.s.spos =H.s.symb = (unsigned int *) malloc((H.lim+1)*sizeof(int));
	H.s.symb = (unsigned int *) malloc((H.lim+1)*sizeof(int));
	H.num = (unsigned int *) malloc((H.depth+1)*sizeof(int));	
	H.fst = (unsigned int *) malloc((H.depth+1)*sizeof(int));	

	//read(file, H.s.spos, (H.lim+1)*sizeof(int));
	//fprintf(stderr," \n read %d spos bytes\n", (H.lim+1)*sizeof(int));
	read_err=read(file, H.s.symb, (H.lim+1)*sizeof(int));	

	read_err=read(file, H.num, (H.depth+1)*sizeof(int));
	read_err=read(file, H.fst, (H.depth+1)*sizeof(int));	
	compressedPsi.diffsHT = H;
	// Fin da carga do arbol de Huffman
	read_err=read(file, &(compressedPsi.nS), sizeof(int));
	read_err=read(file, &(compressedPsi.numberOfSamples), sizeof(size_t));	
	
	read_err=read(file, &(compressedPsi.sampleSize), sizeof(int));
	compressedPsi.samples = (unsigned int *)malloc((( (size_t)compressedPsi.numberOfSamples*compressedPsi.sampleSize+W-1)/W)*sizeof(int));
	read_err=read(file, compressedPsi.samples, (( (size_t)compressedPsi.numberOfSamples*compressedPsi.sampleSize+W-1)/W)*sizeof(int));
	
	read_err=read(file, &(compressedPsi.pointerSize), sizeof(int));
	compressedPsi.samplePointers = (size_t *)malloc((( (size_t)compressedPsi.numberOfSamples*compressedPsi.pointerSize+WW-1)/WW)*sizeof(size_t));
	read_err=read(file, compressedPsi.samplePointers, (( (size_t)compressedPsi.numberOfSamples*compressedPsi.pointerSize+WW-1)/WW)*sizeof(size_t));
	
	read_err=read(file, &(compressedPsi.streamSize), sizeof(size_t));
	compressedPsi.stream = (unsigned int *)malloc(((compressedPsi.streamSize+W-1)/W)*sizeof(int));
	size_t readbytesstream=read(file, compressedPsi.stream, ((compressedPsi.streamSize+W-1)/W)*sizeof(int));
	read_err=read(file, &(compressedPsi.totalMem), sizeof(size_t));
	
	close(file);
	


	printf("\n\n to load");
	printf("\n compressedPSI->NS = %u",compressedPsi.nS);
	printf("\n numberofsamples = %zu",compressedPsi.numberOfSamples);
	printf("\n samplesize_Bits = %u bits",compressedPsi.sampleSize);
	printf("\n tamaño de array samples (bytes) = %zu", (((size_t)compressedPsi.numberOfSamples*compressedPsi.sampleSize+W-1)/W)*sizeof(int));	
	printf("\n pointersize_Bits = %u bits",compressedPsi.pointerSize);
	printf("\n tamaño de array pointers = %zu (bytes)", (( (size_t)compressedPsi.numberOfSamples*compressedPsi.pointerSize+WW-1)/WW)*sizeof(size_t) );
	printf("\n streamsize (lonxitude en bits) = %zu",compressedPsi.streamSize);
	printf("\n stream (tamaño en bytes) = %zu",	((compressedPsi.streamSize+W-1)/W)*sizeof(int));
	printf("\n      bytes read for **stream = %zu**",	readbytesstream);
	                                            
	printf("\n totalMem compressed psi = %zu bytes", compressedPsi.totalMem);
	fflush(stdout);fflush(stderr);
	
			
	return compressedPsi;

}
*/
