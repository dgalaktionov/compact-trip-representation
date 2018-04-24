/* psiGonzalo.c
 * Copyright (C) 2011, Antonio Fariña and Eduardo Rodriguez, all rights reserved.
 * Modified version from Gonzalo Navarro's.
 *
 * psiGonzalo.c: Compressed Representation for Psi in CSA.
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
#include "psiGonzalo.h"

	
// IMPLEMENTACI�N DAS FUNCI�NS

void destroyGonzaloCompressedPsi(GonzaloCompressedPsi *compressedPsi) {
	
	//free(compressedPsi->Hlen.s.spos);
	//free(compressedPsi->Hacc.s.spos);		
	freeHuff(compressedPsi->Hlen);
	freeHuff(compressedPsi->Hacc);	
	free(compressedPsi->cPsi);
	free(compressedPsi->bposS);
}


GonzaloCompressedPsi gonzaloCompressPsi(uint *Psi, uint psiSize, uint T, uint HUFF) {
	
	GonzaloCompressedPsi compressedPsi;
	
	register uint i;
	uint oi,j;
	int ok,k;
	register uint _cptr;

	uint *_cPsi;
	uint *_bposS;
		
	uint links = psiSize;
	uint samplen = T;		
	uint _bplen;
	uint pslen;	
	uint totexc;
		
	uint *acc,*lacc;
	THuff Hacc, Hlen;
	
	uint totalSize;
	
	// Construe os arboles de huffman, o dos valores directos
	// e o das lonxitudes dos runs. Usa como vectores auxiliares de frecuencias
	// a acc e lacc, que finalmente libera.
	acc = (uint *)malloc (HUFF*sizeof(uint));
	lacc = (uint *)malloc ((samplen-1)*sizeof(uint));
	for (k=0;k<HUFF;k++) acc[k]=0;
	for (k=0;k<samplen-1;k++) lacc[k]=0;
	
	ok = 0; 
	k = Psi[0];
	for (i=0;i<=links;i++) { 
		if ((k == 1) && (i % samplen)) { if (ok != 1) oi = i; }
		else { 
			if (ok == 1) { 
				acc[1]++;
			    lacc[i-oi-1]++;
		  	}
		    if (i % samplen) 
				if ((k < 1) || (k >= HUFF)) acc[0]++;
		      	else acc[k]++;
		}
	    ok = (i % samplen) ? k : 0;
		k = Psi[i+1]-Psi[i];
	}
	    
	if (ok == 1) { 
		acc[1]++; 
		lacc[i-oi-1]++;
	}
	
	Hacc = createHuff (acc,HUFF-1, UNSORTED);
	Hlen = createHuff (lacc,samplen-2, UNSORTED);
	totexc = acc[0];
	pslen = bits(psiSize+1);
	_bplen = bits(Hacc.total+Hlen.total+(1+links/samplen+totexc)*pslen);
	_bposS = (uint *)malloc ((((1+links/samplen)*_bplen+W-1)/W)*sizeof(uint));
	_cPsi  = (uint *)malloc (((Hacc.total+Hlen.total+(1+links/samplen+totexc)*pslen+W-1)/W)*sizeof(uint));	
	
	_cptr = 0; 
	ok = 0; 
	k = Psi[0];
	
	for (i=0;i<=links;i++) { 
		
		if ((k == 1) && (i % samplen)) { if (ok != 1) oi = i; }
		else { 
			if (ok == 1) { 
				_cptr = encodeHuff (Hacc,1,_cPsi,_cptr);
			    _cptr = encodeHuff(Hlen,i-oi-1,_cPsi,_cptr);
		  	}
		   	if (i % samplen) { 
				if ((k > 1) && (k < HUFF)) _cptr = encodeHuff (Hacc,k,_cPsi,_cptr);
		        else {
					_cptr = encodeHuff (Hacc,0,_cPsi,_cptr);
			       	bitwrite (_cPsi,_cptr,pslen,Psi[i]);
				 	_cptr += pslen;
			  	}
			}
		  	else { 
				bitwrite (_bposS,(i/samplen)*_bplen,_bplen,_cptr);
			    bitwrite (_cPsi,_cptr,pslen,Psi[i]);
			    _cptr += pslen;
			}
		}
	   	ok = (i % samplen) ? k : 0;
		k = Psi[i+1]-Psi[i];
	}
		
	if (ok == 1) { 
		_cptr = encodeHuff (Hacc,1,_cPsi,_cptr);
		_cptr = encodeHuff(Hlen,i-oi-1,_cPsi,_cptr);
	}
	
	// Calculamos o espacio total
	totalSize = (((1+links/samplen)*_bplen+W-1)/W)*sizeof(uint) +
		((Hacc.total+Hlen.total+(1+links/samplen+totexc)*pslen+W-1)/W)*sizeof(uint) +
		5*sizeof(int) + sizeHuff(Hacc) + sizeHuff(Hlen);
	printf("\n\tCompressed Psi size = %d bytes\n", totalSize);
	
	// Necesario antes de decodificar
	prepareToDecode(&Hacc);
	prepareToDecode(&Hlen);
	
	// Asignamos os valores e devolvemos psi comprimido
	compressedPsi.links = psiSize;
	compressedPsi.totexc = totexc;
	compressedPsi.cPsi = _cPsi;
	compressedPsi.samplen = samplen;
	compressedPsi.bposS = _bposS;
	compressedPsi.bplen = _bplen;
	compressedPsi.pslen = pslen;
	compressedPsi.Hacc = Hacc;
	compressedPsi.Hlen = Hlen;
	compressedPsi.totalMem = totalSize;
	
	free(acc); 
	free(lacc);
	
	return compressedPsi;	
}



int getGonzaloPsiValue(GonzaloCompressedPsi *compressedPsi, unsigned int position) {

	uint *cPsi = compressedPsi->cPsi;
	uint samplen = compressedPsi->samplen;
	uint *bposS = compressedPsi->bposS;
	uint bplen = compressedPsi->bplen;
	uint pslen = compressedPsi->pslen;
	THuff *Hacc = &compressedPsi->Hacc;
	THuff *Hlen = &compressedPsi->Hlen;
	
	uint sampj,cptr,val,dval,rlen,head,hlen;
	
	sampj = (position/samplen)*samplen;
 	cptr = bitread(bposS,(sampj/samplen)*bplen,bplen);
	head = cptr;
	val = bitread(cPsi,head,pslen);
	head += pslen;
			
	while (sampj < position) {
		
		head = decodeHuff(Hacc,&dval,cPsi,head);
		
		if (dval == 0) { 
			
			val = bitread(cPsi,head,pslen);
			head += pslen;
			sampj++;
		}
		else 
			if (dval == 1) {
				head = decodeHuff(Hlen,&rlen,cPsi,head);
				rlen++;
		    	if (sampj + rlen >= position) return val + (position-sampj);
		    	val += rlen;
		    	sampj += rlen;
	 		}
			else { 
				val += dval;
		    	sampj++;
		  	}
			
	}
	    
	return val;
	
}


void storeGonzaloCompressedPsi(GonzaloCompressedPsi *compressedPsi, char *filename) {
	
	int file;
	THuff Hacc;
	THuff Hlen;
	
	if( (file = open(filename, O_WRONLY|O_CREAT, 0700)) < 0) {
		printf("Cannot open file %s\n", filename);
		exit(0);
	}
		
	// Copias locales dos arboles de HUFFMAN
	Hacc = compressedPsi->Hacc;	
	Hlen = compressedPsi->Hlen;
	
	ssize_t write_err;
	
	write_err=write(file, &(compressedPsi->links), sizeof(int));
	write_err=write(file, &(compressedPsi->totexc), sizeof(int));	
	write_err=write(file, &(compressedPsi->samplen), sizeof(int));	
	write_err=write(file, &(compressedPsi->bplen), sizeof(int));	
	write_err=write(file, &(compressedPsi->pslen), sizeof(int));
	// Almacenar o arbol de huffman principal
	write_err=write(file, &Hacc.max, sizeof(int));
	write_err=write(file, &Hacc.lim, sizeof(int));
	write_err=write(file, &Hacc.depth, sizeof(int));
//	write(file, Hacc.s.spos, (Hacc.lim+1)*sizeof(int));
	write_err=write(file, Hacc.s.symb, (Hacc.lim+1)*sizeof(int));	
	write_err=write(file, Hacc.num, (Hacc.depth+1)*sizeof(int));
	write_err=write(file, Hacc.fst, (Hacc.depth+1)*sizeof(int));
	// Fin de almacenar o arbol de huffman principal
	// Almacenar o arbol de huffman das lonxitudes dos runs
	write_err=write(file, &Hlen.max, sizeof(int));
	write_err=write(file, &Hlen.lim, sizeof(int));
	write_err=write(file, &Hlen.depth, sizeof(int));
//	write(file, Hlen.s.spos, (Hlen.lim+1)*sizeof(int));
	write_err=write(file, Hlen.s.symb, (Hlen.lim+1)*sizeof(int));	
	write_err=write(file, Hlen.num, (Hlen.depth+1)*sizeof(int));
	write_err=write(file, Hlen.fst, (Hlen.depth+1)*sizeof(int));
	// Fin de almacenar o arbol de huffman das lonxitudes dos runs
	write_err=write(file,	compressedPsi->bposS, ((compressedPsi->bplen*(1+compressedPsi->links/compressedPsi->samplen)+W-1)/W)*sizeof(uint));		
	write_err=write(file,	compressedPsi->cPsi, ((Hacc.total+Hlen.total+(1+compressedPsi->links/compressedPsi->samplen+compressedPsi->totexc)*compressedPsi->pslen+W-1)/W)*sizeof(int));

	write_err=write(file, &(compressedPsi->totalMem), sizeof(int));
	
	close(file);
	
}


GonzaloCompressedPsi loadGonzaloCompressedPsi(char *filename) {
	
	GonzaloCompressedPsi compressedPsi;

 	THuff Hacc;
	THuff Hlen;
     
	int file;
	
	if( (file = open(filename, O_RDONLY)) < 0) {
		printf("Cannot read file %s\n", filename);
		exit(0);
	}
	ssize_t read_err;
	read_err=read(file, &(compressedPsi.links), sizeof(int));
	read_err=read(file, &(compressedPsi.totexc), sizeof(int));
	read_err=read(file, &(compressedPsi.samplen), sizeof(int));
	read_err=read(file, &(compressedPsi.bplen), sizeof(int));
	read_err=read(file, &(compressedPsi.pslen), sizeof(int));
	// Cargamos o arbol de Huffman principal
	read_err=read(file, &Hacc.max, sizeof(int));
	read_err=read(file, &Hacc.lim, sizeof(int));
	read_err=read(file, &Hacc.depth, sizeof(int));
	//Hacc.s.spos = (unsigned int *) malloc((Hacc.lim+1)*sizeof(int));
	Hacc.s.symb = (unsigned int *) malloc((Hacc.lim+1)*sizeof(int));
	Hacc.num = (unsigned int *) malloc((Hacc.depth+1)*sizeof(int));	
	Hacc.fst = (unsigned int *) malloc((Hacc.depth+1)*sizeof(int));	
	//read(file, Hacc.s.spos, (Hacc.lim+1)*sizeof(int));
	read_err=read(file, Hacc.s.symb, (Hacc.lim+1)*sizeof(int));	
	read_err=read(file, Hacc.num, (Hacc.depth+1)*sizeof(int));
	read_err=read(file, Hacc.fst, (Hacc.depth+1)*sizeof(int));	
	compressedPsi.Hacc = Hacc;
	// Fin da carga do arbol de Huffman	principal
	// Cargamos o arbol de Huffman coas lonxitudes dos runs
	read_err=read(file, &Hlen.max, sizeof(int));
	read_err=read(file, &Hlen.lim, sizeof(int));
	read_err=read(file, &Hlen.depth, sizeof(int));
	//Hlen.s.spos = (unsigned int *) malloc((Hlen.lim+1)*sizeof(int));
	Hlen.s.symb = (unsigned int *) malloc((Hlen.lim+1)*sizeof(int));
	Hlen.num = (unsigned int *) malloc((Hlen.depth+1)*sizeof(int));	
	Hlen.fst = (unsigned int *) malloc((Hlen.depth+1)*sizeof(int));	
	//read(file, Hlen.s.spos, (Hlen.lim+1)*sizeof(int));
	read_err=read(file, Hlen.s.symb, (Hlen.lim+1)*sizeof(int));	
	read_err=read(file, Hlen.num, (Hlen.depth+1)*sizeof(int));
	read_err=read(file, Hlen.fst, (Hlen.depth+1)*sizeof(int));	
	compressedPsi.Hlen = Hlen;
	// Fin da carga do arbol de Huffman	coas lonxitudes dos runs
	compressedPsi.bposS = (uint *) malloc (((compressedPsi.bplen*(1+compressedPsi.links/compressedPsi.samplen)+W-1)/W)*sizeof(uint));
	read_err=read(file, compressedPsi.bposS, ((compressedPsi.bplen*(1+compressedPsi.links/compressedPsi.samplen)+W-1)/W)*sizeof(uint));	
	compressedPsi.cPsi = (uint *) malloc (((Hacc.total+Hlen.total+(1+compressedPsi.links/compressedPsi.samplen+compressedPsi.totexc)*compressedPsi.pslen+W-1)/W)*sizeof(uint));
	read_err=read(file, compressedPsi.cPsi, ((Hacc.total+Hlen.total+(1+compressedPsi.links/compressedPsi.samplen+compressedPsi.totexc)*compressedPsi.pslen+W-1)/W)*sizeof(uint));

	read_err=read(file, &(compressedPsi.totalMem), sizeof(int));	
	close(file);
	
	return compressedPsi;
	
}
