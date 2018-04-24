
/* basics.c
 * Copyright (C) 2011, Antonio Fariña all rights reserved.
 * Original version from pizza-chili code.
 * Modified by Antonio Fariña in 2010.
 *
 * basics.c: Basic bitwise operations + bitVector manipulation, and others.
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

// Basics

// #include "basics.h" included later to avoid macro recursion for malloc
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>




		// Memory management
	
	void *Malloc (size_t n)
	
	   { void *p;
	     if (n == 0) return NULL;
	     p = (void*) malloc (n);
	     if (p == NULL)
	        { fprintf (stderr,"Malloc, Could not allocate %zu bytes\n", (size_t) n);
	          exit(1);
	        }
	     return p;
	   }
	
	void Free (void *p)
	
	   { if (p) free (p); 
	   }
	
	void *Realloc (void *p, size_t n)
	
	   { if (p == NULL) return Malloc (n);
	     if (n == 0) { Free(p); return NULL; }
	     p = (void*) realloc (p,n);
	     if (p == NULL)
	        { fprintf (stderr,"Realloc, Could not allocate %zu bytes\n",n);
	          exit(1);
	        }
	     return p;
	   }

#include "basics.h"

        // bits needed to represent a number between 0 and n

uint bits (size_t n)

   { uint b = 0;
     while (n)
	{ b++; n >>= 1; }
     return b;
   }



        // returns e[p..p+len-1], assuming len <= W

uint bitread (uint *e, size_t p, size_t len)

   { uint answ;
     e += p/W; p %= W;
     answ = *e >> p;
     if (len == W)
	  { if (p) answ |= (*(e+1)) << (W-p);
	  }
     else { if (p+len > W) answ |= (*(e+1)) << (W-p);
            answ &= (1<<len)-1;
	  }
     return answ;
   }


  	// writes e[p..p+len-1] = s, len <= W

void bitwrite (register uint *e, register size_t p, 
	       register size_t len, register uint s)

   { e += p/W; p %= W;
     if (len == W)
	  { *e |= (*e & ((1<<p)-1)) | (s << p);
            if (!p) return;
            e++;
            *e = (*e & ~((1<<p)-1)) | (s >> (W-p));
	  }
     else { if (p+len <= W)
	       { *e = (*e & ~(((1<<len)-1)<<p)) | (s << p);
		 return;
	       }
	    *e = (*e & ((1<<p)-1)) | (s << p);
            e++; len -= W-p;
            *e = (*e & ~((1<<len)-1)) | (s >> (W-p));
	  }
   }
  	// writes e[p..p+len-1] = 0



void bitzero (register uint *e, register size_t p, 
	       register uint len)

   { e += p/W; p %= W;
     if (p+len >= W)
	{ *e &= ~((1<<p)-1);
	  len -= p;
	  e++; p = 0;
	}
     while (len >= W)
	{ *e++ = 0;
	  len -= W;
	}
     if (len > 0)
	*e &= ~(((1<<len)-1)<<p);
   }





/******************************************************************************/
/** to work with 64bit-values, only 64bit code (non portable code!!!!)        */
/******************************************************************************/
        // returns e[p..p+len-1], assuming len <= W

size_t bitread64 (size_t *e, size_t p, size_t len)

   { size_t answ;
     e += p/WW; p %= WW;
     answ = *e >> p;
     if (len == WW)
	  { if (p) answ |= (*(e+1)) << (WW-p);
	  }
     else { if (p+len > WW) answ |= (*(e+1)) << (WW-p);
            answ &= ( ((size_t)1)<<len)-1;
	  }
     return answ;
   }


  	// writes e[p..p+len-1] = s, len <= W

void bitwrite64 (register size_t *e, register size_t p, 
	       register size_t len, register size_t s)

   { e += p/WW; p %= WW;
     if (len == WW)
	  { *e |= (*e & ((((size_t)1)<<p)-1)) | (s << p);
            if (!p) return;
            e++;
            *e = (*e & ~((((size_t)1)<<p)-1)) | (s >> (WW-p));
	  }
     else { if (p+len <= WW)
	       { *e = (*e & ~(((((size_t)1)<<len)-1)<<p)) | (s << p);
		 return;
	       }
	    *e = (*e & ((((size_t)1)<<p)-1)) | (s << p);
            e++; len -= WW-p;
            *e = (*e & ~((((size_t)1)<<len)-1)) | (s >> (WW-p));
	  }
   }
  	// writes e[p..p+len-1] = 0








/////////////////////////////////////////////////////////////////////////
void checkBitwriteBitRead()
{
	size_t i;
	uint nbits;
	size_t ptr, ptrr;
	uint buffer[10]; 
	size_t total= 50000000;
	size_t step= total/1000; 
	
	
	for (ptrr=0; ptrr<=32;ptrr++) {
	//ptr=8; {
		ptr = ptrr+0;
		for (i=0;i<total;i++) {
			if(i%step==0) fprintf(stderr, "Testing bitwrite32/bitread32 %3.1f%%  (i=%lu)(ptr=%lu)(k=%u)\r", (float)i/total*100 , (ulong) i, (ulong) ptr, nbits);
			nbits = bits(i);
			bitwrite(buffer,ptr,nbits, i);
			uint tmp = bitread(buffer,ptr,nbits);
			
			if (tmp != i) {
				printf("\n bitwrite32/bitread32 failed: value src= %u value recovered = %u, k-bits=%u, offset = %u",
				                                                           (uint) i,tmp, nbits,(uint)ptr);
				exit(0);
			}
			//assert(tmp == Psi[index]);
		}
		fprintf(stderr, "Testing bitwrite32/bitread32 %3.1f%%  (i=%lu)(ptr=%lu)(k=%u) OK\n", (float)i/total*100 , (ulong) i, (ulong) ptr, nbits);
//			bitwrite64(samplePointers,samplePointersPtr,pointerSize,ptr);	
	}
}





/////////////////////////////////////////////////////////////////////////
//checkBitwriteBitRead(NULL, 0, 70000000);				
//checkBitwriteBitReadValue(BUFFER, 23,67120000);

//si buffer !=NULL, se tiene en cuenta "offset"
void checkBitwriteBitReadoFFSET(uint* buffer, size_t offset, uint maxval)
{
	size_t i;
	uint nbits;
	size_t ptr, ptrr;
	uint buff[10]; 
	if (!buffer) {buffer=buff; offset=0;}
	
	size_t total= maxval;
	size_t step= total/1000; 
	
	
	for (ptrr=0; ptrr<=32;ptrr++) {
	//ptr=8; {
		ptr = ptrr+offset;
		for (i=0;i<total;i++) {
			if(i%step==0) fprintf(stderr, "##Testing bitwrite32/bitread32 %3.1f%%  (i=%lu)(ptr=%lu)(k=%u)\r", (float)i/total*100 , (ulong) i, (ulong) ptr, nbits);
			nbits = bits(i);
			bitwrite(buffer,ptr,nbits, i);
			uint tmp = bitread(buffer,ptr,nbits);
			
			if (tmp != i) {
				printf("\n ##bitwrite32/bitread32 failed: value src= %u value recovered = %u, k-bits=%u, offset = %u",
				                                                           (uint) i,tmp, nbits,(uint)ptr);
				exit(0);
			}
			//assert(tmp == Psi[index]);
		}
		fprintf(stderr, "##Testing bitwrite32/bitread32 %3.1f%%  (i=%lu)(ptr=%lu)(k=%u) OK\n", (float)i/total*100 , (ulong) i, (ulong) ptr, nbits);
//			bitwrite64(samplePointers,samplePointersPtr,pointerSize,ptr);	
	}
}
	

//si buffer !=NULL, se tiene en cuenta "offset"
void checkBitwriteBitReadValue(uint* buffer, size_t ptr, uint value)
{
	uint i=value;
	size_t nbits = bits(i);
	bitwrite(buffer,ptr,nbits, i);
	uint tmp = bitread(buffer,ptr,nbits);

	if (tmp != i) {
		printf("\n @@bitwrite32/bitread32 failed: value src= %u value recovered = %u, k-bits=%u, offset = %lu",
																   (uint) i,tmp, (uint)nbits,(ulong)ptr);
		exit(0);
	}
	//assert(tmp == Psi[index]);
	fprintf(stderr, "##Testing bitwrite32/bitread32   (i=%lu)(ptr=%lu)(k=%lu) OK\n",   (ulong) i, (ulong) ptr, (ulong) nbits);
}
	


	
