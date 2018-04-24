/*  
 * Copyright (C) 2012, Andres Abeliuk, all rights reserved.
 *
 * Andres Abeliuk <aabeliuk@dcc.uchile.cl>
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


#include <trees/GCT.h>
#include <climits>
#include <cstdlib>
#include <utils/cppUtils.h>
#include <stdexcept>

using namespace cds_utils;

int PRNC = 0;
int PRNR = 0;
int PRNP = 0;
int PRNL = 0;

#ifndef GET_OVERLAP
	#define GET_OVERLAP(lstb,fstb) (OVERLAP_LEAFS[(lstb<<1)|fstb])
#endif

int NullFreqs = 1 << (8 * sizeof (int) - 1);
int deleted_rules, trim_rules;
int tt;
#ifndef MASK_LAST_BIT
	#define MASK_LAST_BIT 0x0000000000000001
#endif

#ifndef min
	#define min(x,y) (((x)<(y))?x:y)
#endif
#ifndef max
	#define max(x,y) (((x)>(y))?x:y)
#endif


namespace cds_static {


/** Computes the total excess (#1-#0) in a byte */	
	const signed char pope[] =
		{
		-8,-6,-6,-4,-6,-4,-4,-2,-6,-4,-4,-2,-4,-2,-2,0,-6,-4,-4,-2,-4,-2,
		-2,0,-4,-2,-2,0,-2,0,0,2,-6,-4,-4,-2,-4,-2,-2,0,-4,-2,-2,0,-2,0,0,
		2,-4,-2,-2,0,-2,0,0,2,-2,0,0,2,0,2,2,4,-6,-4,-4,-2,-4,-2,-2,0,-4,
		-2,-2,0,-2,0,0,2,-4,-2,-2,0,-2,0,0,2,-2,0,0,2,0,2,2,4,-4,-2,-2,0,
		-2,0,0,2,-2,0,0,2,0,2,2,4,-2,0,0,2,0,2,2,4,0,2,2,4,2,4,4,6,-6,-4,
		-4,-2,-4,-2,-2,0,-4,-2,-2,0,-2,0,0,2,-4,-2,-2,0,-2,0,0,2,-2,0,0,2,
		0,2,2,4,-4,-2,-2,0,-2,0,0,2,-2,0,0,2,0,2,2,4,-2,0,0,2,0,2,2,4,0,2,
		2,4,2,4,4,6,-4,-2,-2,0,-2,0,0,2,-2,0,0,2,0,2,2,4,-2,0,0,2,0,2,2,4,
		0,2,2,4,2,4,4,6,-2,0,0,2,0,2,2,4,0,2,2,4,2,4,4,6,0,2,2,4,2,4,4,6,2,
		4,4,6,4,6,6,8
		};
	/** Minium excess in a byte*/
	const signed char popmine[] =
	{
	-8,-6,-6,-4,-6,-4,-4,-2,-6,-4,-4,-2,-4,-2,-2,0,-6,-4,-4,-2,-4,-2,-2,
	0,-4,-2,-2,0,-2,0,-1,1,-6,-4,-4,-2,-4,-2,-2,0,-4,-2,-2,0,-2,0,-1,1,
	-4,-2,-2,0,-2,0,-1,1,-3,-1,-1,1,-2,0,-1,1,-6,-4,-4,-2,-4,-2,-2,0,-4,
	-2,-2,0,-2,0,-1,1,-4,-2,-2,0,-2,0,-1,1,-3,-1,-1,1,-2,0,-1,1,-5,-3,-3,
	-1,-3,-1,-1,1,-3,-1,-1,1,-2,0,-1,1,-4,-2,-2,0,-2,0,-1,1,-3,-1,-1,1,-
	2,0,-1,1,-7,-5,-5,-3,-5,-3,-3,-1,-5,-3,-3,-1,-3,-1,-1,1,-5,-3,-3,-1,
	-3,-1,-1,1,-3,-1,-1,1,-2,0,-1,1,-5,-3,-3,-1,-3,-1,-1,1,-3,-1,-1,1,-2,
	0,-1,1,-4,-2,-2,0,-2,0,-1,1,-3,-1,-1,1,-2,0,-1,1,-6,-4,-4,-2,-4,-2,-2,
	0,-4,-2,-2,0,-2,0,-1,1,-4,-2,-2,0,-2,0,-1,1,-3,-1,-1,1,-2,0,-1,1,-5,
	-3,-3,-1,-3,-1,-1,1,-3,-1,-1,1,-2,0,-1,1,-4,-2,-2,0,-2,0,-1,1,-3,-1,
	-1,1,-2,0,-1,1
	};
	/** Position in a byte where the min excess occurs*/
	const unsigned char popposmine[] =
	{
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,1,1,0,0,7,7,
	7,7,7,7,7,7,7,7,7,7,1,1,0,0,3,3,3,3,1,1,0,0,2,2,0,0,1,1,0,0,5,5,5,5,
	5,5,5,5,5,5,5,5,1,1,0,0,3,3,3,3,1,1,0,0,2,2,0,0,1,1,0,0,4,4,4,4,4,4,
	0,0,2,2,0,0,1,1,0,0,3,3,3,3,1,1,0,0,2,2,0,0,1,1,0,0,6,6,6,6,6,6,6,6,
	6,6,6,6,6,6,0,0,6,6,6,6,6,6,0,0,2,2,0,0,1,1,0,0,4,4,4,4,4,4,0,0,2,2,
	0,0,1,1,0,0,3,3,3,3,1,1,0,0,2,2,0,0,1,1,0,0,5,5,5,5,5,5,5,5,5,5,5,5,
	1,1,0,0,3,3,3,3,1,1,0,0,2,2,0,0,1,1,0,0,4,4,4,4,4,4,0,0,2,2,0,0,1,1,
	0,0,3,3,3,3,1,1,0,0,2,2,0,0,1,1,0,0
	};
 
 void set_field64(uint *A, int len, size_t index, size_t x) {
        if(len==0) return;
        int i=index*len/W, j=index*len-i*W;
        uint mask = ((j+len) < W ? ~0u << (j+len) : 0)
            | ((W-j) < W ? ~0u >> (W-j) : 0);
        A[i] = (A[i] & mask) | x << j;
        if (j+len>W) {
            mask = ((~0u) << (len+j-W));
            A[i+1] = (A[i+1] & mask)| x >> (W-j);
        }
    }

    uint get_field64(uint *A, size_t len,  size_t index) {
        if(len==0) return 0;
        int i=index*len/W, j=index*len-W*i;
        uint result;
        if (j+len <= W)
            result = (A[i] << (W-j-len)) >> (W-len);
        else {
            result = A[i] >> j;
            result = result | (A[i+1] << (WW-j-len)) >> (W-len);
        }
        return result;
    }

	int GCT::getC(size_t i) const {
        return get_field(C, b, i);
    }
/**
 * Uncoment what you want to test. The tests use assertions so
 * you must have enabled that in the code (do not compile with -DNDEBUG)
 * */
void GCT::testsAll(uint *input, size_t n){
	 //testLeafRankSelect(input);
	 //testAccess(input);
	 //testRankSelect(input);
	 //testFWDBWDSEARCH(input,len);
	//testRmq(input,len);
}
 


void GCT::testRmq(uint *input, size_t n){
	cout << "Testing rmq"<< endl;
	
	int minE=1,maxE=1;

	size_t posMin;
	for (size_t i=0;i<n;i++){
		//~ excess[i]=excess[i-1]+((input[i]==0)?-1:1);
		if ((uint)minE>getExcessBuild(i)){
			minE = getExcessBuild(i);
			posMin = i;
		}
		maxE = max(maxE,(int)getExcessBuild(i));
	}
	
	#if 1
	for (size_t i=0;i<n-1;i++){

		if (posMin<i){
			minE=getExcessBuild(i);
			posMin = i;
			for (size_t k=i+1;k<n;k++){
				if (getExcessBuild(k)<minE){
					minE = getExcessBuild(k);
					posMin=k;
				}
			}

		}
		size_t minErmq;
		if (rmqi(i,n-1,minErmq)!=posMin){
			cout << "i== " << i <<"&& j== " << n-1 << ", posMin: " << posMin << ", returned: " << rmqi(i,n-1,minErmq) << ", min: " << minErmq << endl;
		}
			assert(rmqi(i,n-1,minErmq)==posMin);
			assert(minErmq == minE);
					
	}
	cout << "Until end OK" << endl;
	for (size_t i=0;i<n/2;i++){
			minE=INT_MAX;
			for (size_t k=i;k<n-i;k++){
				if (getExcessBuild(k)<minE){
					minE=getExcessBuild(k);
					posMin=k;
				}
			}
		size_t minErmq;
		
		if (rmqi(i,n-i-1,minErmq)!=posMin || minErmq!=minE){
			cout << "i: " << i <<", j: " << n-i-1 << ", posMin: " << posMin << ", returned: " << rmqi(i,n-i-1,minErmq) << "(min: " << minE <<" found: " << minErmq << ")" << endl;
		}
			assert(rmqi(i,n-i-1,minErmq)==posMin);
			assert(minErmq == minE);
			
	}
	cout << "Narrowing OK" << endl;
	#endif
	
	//~ rmqi(29,32,bla);
	for (size_t i=0;i<10000;i++){
			size_t ini = ((float)rand()/RAND_MAX)*(n-2);
			size_t fin = ini+((float)rand()/RAND_MAX)*(n-ini-2)+1;
			minE = INT_MAX;
			for (size_t k=ini;k<=fin;k++){
				if (getExcessBuild(k)<minE){
					minE=getExcessBuild(k);
					posMin=k;
				}
			}
		size_t minErmq;
		//~ if (ini==9 && fin==11)
		//~ if (ini==13 && fin==52)
			//~ cout << "ini: " << ini << ", fin: " << fin << endl;
		//~ if (ini==27945 && fin==7757097)
			//~ cout << endl;
		if (rmqi(ini,fin,minErmq)!=posMin || minErmq!=minE){
			cout << "i== " << ini <<"&& j== " << fin << ", posMin: " << posMin << ", returned: " << rmqi(ini,fin,minErmq) << "(min: " << minE <<" found: " << minErmq << ")" << endl;
		}
		//~ else{
			//~ cout << "OK" << endl;
		//~ }
			//~ assert(rmqi(ini,fin,minErmq)==posMin);
			//~ assert(minErmq == minE);
			
	}
	cout << "Test rmq OK" << endl;
}

void GCT::testFWDBWDSEARCH(uint *input, size_t n){
	cout << "Testing FWD/BWD"<< endl;

	
	
	uint minE=1,maxE=1;
	assert(bitget(input,0)==1);
	//~ excess[0]=1;
	//~ excessTest++;
	
	for (size_t i=1;i<n;i++){
		//~ excess[i]=excess[i-1]+((input[i]==0)?-1:1);
		minE = min(minE,(uint)getExcessBuild(i));
		maxE = max(maxE,(uint)getExcessBuild(i));
	}
	

	#if 0
	for (size_t j=1;j<maxE;j++){
		for (size_t i=0;i<n-1;){
			if (getExcessBuild(i++)==j){
				size_t k;
				for (k=i;k<n;k++){
					if (getExcessBuild(i)>getExcessBuild(k) && getExcessBuild(k)==j){
						if(fwdExcess(i,j)!=k){
							cerr << "1-error: fwdExcess("<<i<<","<<j<<")="<<fwdExcess(i,j) << " but it should be " << k << endl;
							assert(false);
						}
						break;
					}
				}
				i=k;
			}
		}
	}
	
	cout << "Test fwdExcess OK" << endl;
	#endif
	
	for (size_t j=1;j<maxE;j++){
		for (int i=n-1;i>0;){
			if (getExcessBuild(i--)==j){
				int k;
				for (k=i;k>=0;k--){
					if (getExcessBuild(i)>getExcessBuild(k) && getExcessBuild(k)==j){
						//~ cout << "i: " << i <<", j: " << j << endl;
						
						//~ if (i==7755444)
							//~ cout << endl;
						size_t res = bwd_search(i,j);
						if (res>=i){
							cerr << "res(" << res << ")>=i("<<i<<")"<<endl;
							assert(false);
						}
						if(bwd_search(i,j)!=k){
							cerr << "2-error: bwd_search("<<i<<","<<j<<")="<<bwd_search(i,j) << " but it should be " << k << endl;
							assert(false);
						}
						break;
					}
				}
				i=k;
			}
		}
	}
	cout << "Test BWD_SEARCH OK" << endl;
}
void GCT::testFWDBWDSEARCH_leafs(uint *input, size_t n){
	cout << "Testing FWD/BWD"<< endl;

	
	
	uint minE=1,maxE=1;
	assert(bitget(input,0)==1);
	//~ excess[0]=1;
	//~ excessTest++;
	//~ unsigned short int *excess = excessTest+1;
	for (size_t i=1;i<n;i++){
		//~ excess[i]=excess[i-1]+((input[i]==0)?-1:1);
		minE = min(minE,(uint)getExcessBuild(i));
		maxE = max(maxE,(uint)getExcessBuild(i));
	}
	
	#if 1
	for (size_t j=1;j<maxE;j++){
		for (size_t i=0;i<n-1;){
			if (getExcessBuild(i++)==j){
				size_t k;
				for (k=i;k<n;k++){
					if (getExcessBuild(i)>getExcessBuild(k) && getExcessBuild(k)==j){
						fwdExcess(i,j);
						break;
					}
				}
				i=k;
			}
		}
	}
	
	cout << "Test FWDEXCESS OK" << endl;
	#endif
	
	for (size_t j=1;j<maxE;j++){
		for (int i=n-1;i>0;){
			if (getExcessBuild(i--)==j){
				int k;
				for (k=i;k>=0;k--){
					if (getExcessBuild(i)>getExcessBuild(k) && getExcessBuild(k)==j){
						bwd_search(i,j);
						break;
					}
				}
				i=k;
			}
		}
	}
	cout << "Test BWD_SEARCH OK" << endl;
}

void GCT::testExcess(uint *input, size_t n){
	//~ uint *excess; 
	//~ excess = new uint[n];
	//~ assert(bitget(input,0)==1);
	//~ excess[0]=1;
	//~ assert(getExcess(0)==excess[0]);
	//~ for (size_t i=1;i<n;i++){
		//~ excess[i]=excess[i-1]+((bitget(input,i)==0)?-1:1);
		//~ assert(getExcess(i)==excess[i]);
		//~ size_t e;
		//~ rank0(i,e);
		//~ assert(e==excess[i]);
		//~ rank1(i,e);
		//~ assert(e==excess[i]);
	//~ }
	
}

void GCT::testAccess(uint *input){
	
	size_t lim = min((size_t)10000,(size_t)len);
	assert(lim>0);
	for (size_t i=0;i<lim;i++){
		int excess;
		size_t val = access(i,excess);
		assert(bitget(input,i)==val);
		assert(getExcessBuild(i)==excess);
	}
	for (size_t i=0;i<lim;i++){
		uint leaf;
		size_t val = access_and_leaf_rank(i,leaf);
		assert(bitget(input,i)==val);
		if(bsLeafsTest->rank1(i)!=leaf){
			cout << "Error test access"<< endl;
		}
	}
	cout << "Partial test access (up to " << lim << ") OK" << endl;
}

void GCT::testLeafRankSelect(uint *input){
	
	size_t lim = min((size_t)10000,(size_t)len/2);
	assert(lim>0);
	for (int i=0;i<lim;i++){
		size_t res = brLeafSelect(i+1);
		/**
			why res+1? brLeafSelect returns the position of where the 1 of the leaf (pair 10) is. 
			However, in bsLeafTest, the positions marked as leafs are those corresponding to the "0" of the pair 10.
		*/
		// if(!(bsLeafsTest->rank1(res+1)==(i+1))){
		// 	cerr << "i+1: " << i+1 << ", res: "<< res << endl;
		// } 
		assert(bsLeafsTest->rank1(res+1)==(i+1));
	}
	cout << "Partial test leaf Rank/Select (up to " << lim << ") OK" << endl;
}

void GCT::testRankSelect(uint *input){
	cout << "Testing rank" << endl;
	size_t countZeroes,cOnes;
	countZeroes=cOnes=0;
	uint limit = min(len,10000);
	
	for (int i=0;i<limit;i++){
		if (bitget(input,i)==0) countZeroes++;
		else cOnes++;
		if (rank0(i)!=countZeroes){
			cout << "rank0("<<i<<"), expected: " << countZeroes << ", obtained: " << rank0(i) << endl;
		}
		
		if (rank1(i)!=cOnes){
			cout << "rank1("<<i<<"): expected: " << cOnes << ", obtained: " << rank1(i) << endl;
		}
		assert(rank0(i)==countZeroes);
		assert(rank1(i)==cOnes);
	}
	size_t e;
	cout << "Tests rank OK" << endl;
	#if 1

	size_t z=0;
	for (size_t i=0;i<limit;i++){
		if (bitget(input,i)==0){
			z++;
			size_t res;
			if ((res=select0(z,e))!=i || (e!=getExcessBuild(res))){
				cout << "select0(" << z << "): " << select0(z,e) <<", z="<<i<< endl;
				assert(false);
			}
			if (e!=getExcessBuild(res))
				cout << "error en select0" << endl;
		}
	}
	cout << "Test select0 OK" << endl;
	z=0;
	for (size_t i=0;i<limit;i++){
		if (bitget(input,i)==1){
			z++;
		//~ while(z<len && input[z]==1)z++;
			if (select1(z,e)!=i){
				cout << "select1(" << z << "): " << select1(z,e) <<", z="<<i<< endl;
				assert(false);
			}
		}
		//~ assert(select0(i+1)==z);
		//~ z++;
	}
	cout << "Test select1 OK" << endl;
	#endif
	//~ z=0;
	//~ for (size_t i=0;i<countZeroes;i++){
		//~ while(z<len && input[z]!=1)z++;
		//~ assert(select1(i+1)==z);
		//~ z++;
	//~ }

}



/**
 * Used to check the correctness of most operations (inside assertions). 
 * */
size_t GCT::getExcessBuild(size_t pos){
	size_t r1 = bsExcessBuild->rank1(pos);
	size_t r0 = pos+1-r1;
	return r1-r0;
}

GCT * GCT::newGCTFromRevBitmap(uint _delta, uint _samplingC, uint *bmp_input, ulong _len, bool candel){

	uint *new_bmp;
	if (candel){
		new_bmp = bmp_input;
	}else{
		createEmptyBitmap(&new_bmp,_len);
	}
	uint nints = uint_len(_len,1);
	for (uint i=0;i<nints;i++){
		uint _W = W/2;
		for (uint j=0;j<_W;j++){
			uint l = bitget(bmp_input+i,W-j-1);
			uint r = bitget(bmp_input+i,j);
			if (l){
				bit_set(new_bmp + i, j);
			}else{
				bitclean(new_bmp+i,j);
			}
			if(r){
				bit_set(new_bmp + i, W - j - 1);
			}else{
				bitclean(new_bmp+i,W-j-1);
			}
		}
	}
	GCT *gct = GCT::GCTFromBitmap(_delta, _samplingC, new_bmp, _len);

	if (!candel){
		delete [] new_bmp;
	}

	return gct;
}

/**
  delta: sampling of the rules (upper bound of the number of rules you have to expand to get the value (lenght, excess, ...) you are looking for
  samplingC: sampling of the vector C (every 2^samplingC position is sampled)
  input: bitmap with the balanced parentheses in verbatim
  inputGrammar: file that contains the grammar of the input bitmap 
*/

GCT::GCT(uint _delta,  uint _samplingC, uint *input, char *inputGrammar):Parentheses(0) {
	
	ifstream fp(inputGrammar);

    uint hdr = loadValue<uint>(fp);

    len  = loadValue<long>(fp);
    n = loadValue<long>(fp);
    alph = loadValue<int>(fp);
    c = loadValue<long>(fp);

        long *left, *right, *lengths, *nc;
	left = loadValue<long>(fp,n-alph);
	right = loadValue<long>(fp,n-alph);
	lengths = loadValue<long>(fp,n-alph);
	nc = loadValue<long>(fp,c);
    RULES_LEFT = new uint[n-alph];
    RULES_RIGHT = new uint[n-alph];
    rule_lengths = new uint[n-alph];
    newC        = new uint[c];
//        nterms  )
        for (long i=0;i<n-alph;i++){
            RULES_LEFT[i] = (uint)left[i];
            RULES_RIGHT[i] = (uint)right[i];
            rule_lengths[i] = (uint)lengths[i];
        }
        for (long i=0;i<c;i++){
            newC[i] = nc[i];
        }
        delete [] left;
        delete [] right;
        delete [] nc;
        delete [] lengths;
	fp.close();
	delta=_delta;	
	samplingC = _samplingC;
	build(input, len);
	n = len;
}
/**
 delta: sampling of the rules (upper bound of the number of rules you have to expand to get the value (lenght, excess, ...) you are looking for
 samplingC: sampling of the vector C (every 2^samplingC position is sampled)
 Builds a GCT from a bitmap stored in input and length _len. 
*/
//GCT::GCT(uint _delta, uint _samplingC, uint *_input, ulong _len):Parentheses(_len){
//	GCTFromBitmap(_delta, _samplingC, _input, _len);
//}


GCT* GCT::GCTFromBitmap(uint _delta, uint _samplingC, uint *_input, ulong _len){
	GCT *gct = new GCT();
	gct->len = _len;
	gct->delta=_delta;
	gct->samplingC = _samplingC;
	
	Repair *rp = Repair::instanceFromBitmap(_input, _len);
	gct->len = _len;
	gct->n = rp->getNRules()+rp->getAlpha();
	gct->alph = rp->getAlpha();
	gct->c = rp->getNC();
	gct->RULES_LEFT = new uint[gct->n-gct->alph];
	gct->RULES_RIGHT = new uint[gct->n-gct->alph];
	gct->rule_lengths = new uint[gct->n-gct->alph];
	gct->newC = new uint[gct->c];
	for (uint i=0;i<gct->n-gct->alph;i++){
		gct->RULES_LEFT[i]= rp->getLeft(i);
		gct->RULES_RIGHT[i] = rp->getRight(i);
		gct->rule_lengths[i] = rp->getLength(i+gct->alph);
	}
	for (uint i=0;i<gct->c;i++){
		gct->newC[i]=rp->getC(i);
	}
	delete rp;
	gct->build(_input,gct->len);
	gct->n = gct->len;
	return gct;
}

void GCT::build(uint *input,ulong len){


	assert(isATree(input,(ulong)len));
	
	int ETMP[2];
	ETMP[0]=-1;ETMP[1]=1;
	
	int maxte=0;
	int e=0;
	for (int i=0;i<len;i++){
		e+=ETMP[bitget(input,i)];
		maxte=max(maxte,e);
	}


	bsExcessBuild = new BitSequenceRG(input,len,32);
	
	excessTest=NULL;
	uint *bmp_leafs;
	createEmptyBitmap(&bmp_leafs, len);
	for (size_t i=0;i<uint_len(len,1);i++) bmp_leafs[i]=0;
	len--;
	for (size_t i=0;i<len;i++){
		if (bitget(input,i) && !bitget(input,i+1)) bit_set(bmp_leafs, i + 1);
	}
	len++;
	
	bsLeafsTest = new BitSequenceRG(bmp_leafs,len,32);
	delete [] bmp_leafs;
	//~ cout << "#leaves="<< bsLeafsTest->rank1(bsLeafsTest->getLength()-1) << endl;
	

	
	/** Trim and delete the rules*/
	
	uint *symbPerm;
	uint newn=n;
	uint new_alph=alph;
	new_alph = my_trim_rule(symbPerm, newn,rule_lengths, newC);
	verbatimLength = lenTerms[new_alph];
	n = newn;
	alph=new_alph;
	
	/** Build the dictionary (renaming the rules)
	 * PERM[i] contains the new symbol for i*/
	uint *PERM;
	dic = new DictionaryRPTabei(RULES_LEFT,RULES_RIGHT,new_alph,newn,PERM);
	/** Remane the sequence C after building the dictionary*/
	for (int i=0;i<c;i++){
		newC[i]=PERM[newC[i]];
	 }

	/** Rewrite ruleLens terminals*/
	
	uint *INVP = new uint[n];
	for (int i=0;i<n;i++)INVP[PERM[i]]=i;
	for (int i=new_alph;i<n;i++)assert((lengths[INVP[i]]==lengths[INVP[dic->getLeft(i)]]+lengths[INVP[dic->getRight(i)]]));
	

	/**Store C using c*log(R), being R the number of rules (n stands for R in the code)*/
	
	b = bits(n);
	nb_C = uint_len(c+1,b);
	C  = new uint[nb_C];
	for (uint i=0;i<nb_C;i++) C[i]=0;
	for (uint i=0;i<c;i++){
		set_field(C,b,i,newC[i]);
	}
	set_field(C,b,c,newC[c-1]);
	
	delete [] newC;
	delete [] rule_lengths;
	#ifdef VERBOSE
		for (size_t i=0;i<c;i++){
			cout << "C[" << i << "]: "  << newC[i] << endl;
		}
	#endif
	
	delete [] RULES_LEFT;
	delete [] RULES_RIGHT;

	/** Compute the min and total excess of each rule. Max excess is not necessary*/
	
	minExcess = new int [n];
	totalExcess = new int [n];
	
	for (int i=0;i<n;i++){totalExcess[i]=0;minExcess[i]=INT_MAX;}
	minExcess[0]=-1;
	minExcess[1]=1;
	totalExcess[0]=-1;totalExcess[1]=1;
	int mine=INT_MAX;
	
	int *totalE_tmp = new int [n];
	int *minE_tmp = new int[n];
	
	EXCESS = new int[alph];
	for (int i=0;i<alph;i++){
		
		EXCESS[i]=(bitget(terminals,lenTerms[i]))?1:-1;
		minE_tmp[i]=EXCESS[i];
		for (uint j=lenTerms[i]+1;j<lenTerms[i+1];j++){
			EXCESS[i]+=(bitget(terminals,j))?1:-1;
			if (EXCESS[i]<minE_tmp[i])
				minE_tmp[i]=EXCESS[i];
		}
		totalE_tmp[i] = EXCESS[i];
		lengths[i]=lenTerms[i+1]-lenTerms[i];
		minExcess[i]=minE_tmp[i];
		totalExcess[i]=totalE_tmp[i];
	}

	for (int i=alph;i<n;i++){
		totalE_tmp[i]=INT_MAX;
		minE_tmp[i]=INT_MAX;
	}
	maxte=0;
	uint *depthRule = new uint[n-alph];
	int maxmine=INT_MIN;
	for (size_t i=0;i<n-alph;i++){
		int exceso=0;
		uint dr = 2;
		int me;
		computeExcess(i+alph,i+alph,minExcess[i+alph],me,totalExcess[i+alph],exceso,totalE_tmp,minE_tmp,depthRule,dr);
		assert(minExcess[i+alph]<=1);
		maxte=max(maxte,totalExcess[i+alph]);
		if (minExcess[i+alph]>maxmine){
			maxmine=minExcess[i+alph];
		}	
		if (minExcess[i+alph]<mine){
			mine=minExcess[i+alph];
		}		
		#if 0
		cout << "Rule-" << i+alph << ", minE: " << minExcess[i+alph] << ", totalE: " << totalExcess[i+alph] << endl;
		#endif
	}
	maxRuleDepth=1024;
	delete [] depthRule;
	delete [] totalE_tmp;
	delete [] minE_tmp;
	
	
	//TODO: sampling rate maybe to high...check what we can do 
	uint max_length=0;
	for (int i=0;i<n;i++){
		max_length = max(max_length,lengths[i]);
	}
	assert(samplingC>0);
	blockSizeC=(1<<samplingC);
	#ifdef VERBOSE
	cout << "blockSize: " << blockSizeC << endl;
	#endif
	blogBlockSizeC=bits(blockSizeC-1);
	blogBlockSizeCPlus1=blogBlockSizeC+1;
	blockSizeC2=blockSizeC<<1;
	blogBlockSizeC2=blogBlockSizeC+1;
	

	sampleLeafs(INVP,input);	
	sampleC(INVP,input);
	sampleOverheads(delta,INVP);
	compressStructures();
	
	delete [] PERM;
	delete [] INVP;
	delete [] minExcess;
	delete [] totalExcess;
	delete [] lengths;
	testsAll(input,len);
	#ifdef NDEBUG
		delete bsLeafsTest;
		delete bsExcessBuild;
	#endif

}
/**
 * Given an array as input and the size of the compressed structure that represents it (compressed_size), 
 * checks if that compressed data structure performs better in terms of space than just storing it 
 * using n * log (max(input)) bits 
 * 
 * */
bool GCT::isBetterThanTrivialWay(char *message, size_t compressed_size, uint *input, uint n){
	cerr << "Checking compressibility of " << message << endl;
	uint m=0;
	for (uint i=0;i<n;i++)
		m=max(m,input[i]);
	cerr << "Input: " << compressed_size << ", trivial: " << (sizeof(uint)*uint_len(n,bits(m))) << ", ratio: " << (double)compressed_size / (sizeof(uint)*uint_len(n,bits(m))) << endl;
	return (compressed_size<(size_t)(sizeof(uint)*uint_len(n,bits(m))));
}

/**
 * Compress the overhead by using, mainly, DACs and Arrays. 
 * */

void GCT::compressStructures(){

	uint *tmp = new uint[heapLength];
	uint lenSampled=bd->rank1(bd->getLength()-1);
	for (int i=0;i<nSamplesC;i++)tmp[i]=sampleC_p[i];
	
	compressedSampleC_p = new DAC(tmp,nSamplesC,false);
	for (int i=0;i<nSamplesC;i++)
		assert(compressedSampleC_p->access(i)==sampleC_p[i]);
	#if VERBOSE
	if (!isBetterThanTrivialWay("sampleC_p",compressedSampleC_p->getSize(),tmp,nSamplesC)){
		cerr << "It is better not to use DAC for sampleC_p"<<endl;
	}
	#endif
	delete [] sampleC_p;
	
	compressedSampleC_leafs = new DAC(sampleC_leafs,nSamplesC,false);
	for (int i=0;i<nSamplesC;i++){
		assert(compressedSampleC_leafs->access(i)==sampleC_leafs[i]);
	}
	#if VERBOSE
	if (!isBetterThanTrivialWay("sampleC_leafs",compressedSampleC_leafs->getSize(),sampleC_leafs,nSamplesC)){
		cerr << "It is better not to use DAC for sampleC_p"<<endl;
	}	
	#endif

	compressedSampledLengths = new DAC(sampledLengths,lenSampled+1,false);
	for (int i=0;i<=lenSampled;i++){
		assert(sampledLengths[i]==compressedSampledLengths->access(i));
	}

	
	#if VERBOSE
	if (!isBetterThanTrivialWay("sampledLengths",compressedSampledLengths->getSize(),sampledLengths,lenSampled)){
		cerr << "It is better not to use DAC for sampledLengths"<<endl;
	}
	#endif
	delete [] sampledLengths;
	
	compressedRuleLeafs = new DAC(sampledLeafs,lenSampled,false);
	for (int i=0;i<lenSampled;i++){
		assert(sampledLeafs[i]==compressedRuleLeafs->access(i));
	}
	#if VERBOSE
		if (!isBetterThanTrivialWay("rule_sampledLeafs",compressedRuleLeafs->getSize(),sampledLeafs,lenSampled)){
			cerr << "It is better not to use DAC for sampledLengths"<<endl;
		}
	#endif

	for (int i=0;i<lenSampled;i++) {
		sampledTotalExcess[i]-=sampledMinExcess[i];//MIN_TOTAL_EXCESS_RULE;
		assert(sampledTotalExcess[i]>=0);
	}
	
	auto allEq = [](int *input, ulong len){
		if (len==0) return true;
		for (uint i=1;i<len;i++){
			if (input[i]!=input[i-1])
				return false;
		}
		return true;
	};

	auto addDifferent = [](int *&input, ulong len){
		int *new_input = new int[len+1];
		for (uint i=0;i<len;i++){
			new_input[i] = input[i];
		}
		new_input[len] = new_input[len-1]+1;
		auto tmp = input;
		input = new_input;
		delete [] tmp;
		return len+1;
	};
	if (allEq(sampledTotalExcess,lenSampled)){
		ulong new_len = addDifferent(sampledTotalExcess,lenSampled);
		compressedSampledTotalExcess = new DAC((uint *) sampledTotalExcess, new_len, false);
	}else {
		compressedSampledTotalExcess = new DAC((uint *) sampledTotalExcess, lenSampled, false);
	}

	for (int i=0;i<lenSampled;i++) 
		sampledTotalExcess[i]+=sampledMinExcess[i];//MIN_TOTAL_EXCESS_RULE;
	for (int i=0;i<lenSampled;i++){
		if(sampledTotalExcess[i]!=(compressedSampledTotalExcess->access(i)+sampledMinExcess[i])){
			cerr << "Total excess " << endl;
		}
		assert(sampledTotalExcess[i]==(compressedSampledTotalExcess->access(i)+sampledMinExcess[i]));//MIN_TOTAL_EXCESS_RULE));
	}
	#if VERBOSE
	if (!isBetterThanTrivialWay("sampledTotalExcess",compressedSampledTotalExcess->getSize(),(uint*)sampledTotalExcess,lenSampled)){
		cerr << "It is better not to use DAC for sampledTotalExcess"<<endl;
	}
	#endif

	#if VERBOSE
	if (!isBetterThanTrivialWay("sampledTotalExcess",compressedSampledTotalExcess->getSize(),(uint*)sampledTotalExcess,lenSampled)){
		cerr << "It is better not to use DAC for sampledTotalExcess"<<endl;
	}
	#endif


	delete [] sampledTotalExcess;
	
	
	for (int i=0;i<lenSampled;i++){
		assert((MIN_EXCESS-sampledMinExcess[i] )>=0);
		sampledMinExcess[i]=MIN_EXCESS-sampledMinExcess[i];
		assert(sampledMinExcess[i]>=0);
	}
	compressedRulesMinExcess = new DAC((uint*)sampledMinExcess,lenSampled);
	for (int i=0;i<lenSampled;i++){
		assert(sampledMinExcess[i]==compressedRulesMinExcess->access(i));
		sampledMinExcess[i]=MIN_EXCESS-sampledMinExcess[i];
	}
	#if VERBOSE
	if (!isBetterThanTrivialWay("sampledMinExcess",compressedRulesMinExcess->getSize(),(uint*)sampledMinExcess,lenSampled)){
		cerr << "It is better not to use DAC for sampledMinExcess"<<endl;
	}
	#endif
	delete [] sampledMinExcess;
		
	
	
	/** Compress heaps over C*/
	
	/** Compress heap of min excesses*/
	heapSampleC_minE = new DAC((uint*)sampleC_minE,heapLength,false);
	
	size_t cme=(size_t)heapLength;
	
	heapSampleC_minE_array = new Array((uint*)sampleC_minE,cme);
	//~ Array q((uint*)sampleC_minE,cme+1);
	
	for (int i=0;i<heapLength;i++) assert(heapSampleC_minE->access(i)==sampleC_minE[i]);
	for (int i=0;i<heapLength;i++) assert((*heapSampleC_minE_array)[i]==sampleC_minE[i]);
	#if VERBOSE
	if (!isBetterThanTrivialWay("heapSampleC_minE",heapSampleC_minE->getSize(),(uint*)sampleC_minE,heapLength)){
		cerr << "It is better not to use DAC for heapsampleC_minE"<<endl;
	}
	#endif
	delete [] sampleC_minE;
	
	for (int i=0;i<heapLength;i++)tmp[i]=(uint)sampleC_e[i];
	/** Compress heap of total excesses*/
	heapSampleC_e = new DAC(tmp,heapLength,false);
	for (int i=0;i<heapLength;i++){
		assert(sampleC_e[i]>=0);
		assert(heapSampleC_e->access(i)==sampleC_e[i]);
	}
	#if VERBOSE
	if (!isBetterThanTrivialWay("heapSampleC_e",heapSampleC_e->getSize(),tmp,heapLength)){
		cerr << "It is better not to use DAC for heapSampleC_e"<<endl;
	}
	#endif
	delete  [] sampleC_e;
	
	for (int i=0;i<nSamplesC;i++)tmp[i]=(uint)sampleC_o[i];
	compressedSampleC_o = new DAC((uint*)tmp,nSamplesC,false);
	for (int i=0;i<nSamplesC;i++)
		assert(compressedSampleC_o->access(i)==sampleC_o[i]);
	#if VERBOSE
	if (!isBetterThanTrivialWay("sampleC_o",compressedSampleC_o->getSize(),tmp,nSamplesC)){
		cerr << "It is better not to use DAC for sampleC_o"<<endl;
	}
	#endif
	delete [] sampleC_o;
	
	/** Compress pointers to verbatim rules*/
	if (alph>2){
		lenTermFieldWidth =0;
		for (int i=0;i<=alph;i++)
			lenTermFieldWidth=max(lenTermFieldWidth,lenTerms[i]);
		lenTermFieldWidth=bits(lenTermFieldWidth);
		compressedLenTerms = new uint[uint_len(lenTermFieldWidth,alph+1)];
		for (int i=0;i<uint_len(lenTermFieldWidth,alph+1);i++)
			compressedLenTerms[i]=0;
		
		for (int i=0;i<=alph;i++){
			set_field(compressedLenTerms,(size_t)lenTermFieldWidth,(size_t)i,lenTerms[i]);
		}
		
		for (int i=0;i<=alph;i++)
			assert(get_field(compressedLenTerms,lenTermFieldWidth,i)==lenTerms[i]);
	}
	
	delete [] tmp;
	return;
}

/**
 * It marks in a bitmap those rules that has to be sampled, depending on the 
 * sampling rate "sample". The i-th rule is represented in the bitmap by the 
 * bit i-alph.
 * 
 * */
size_t GCT::sampleOverheadsRec(uint rule, uint depth, uint sample, uint *bitVector){
	
	if (rule<alph) return 0;
	rule-=alph;
	bool left,right;
	left=right=true;
	
	if (depth%sample==0){
		if (dic->getLeft(rule+alph)>=alph){
			left = !bitget(bitVector,dic->getLeft(rule+alph)-alph);
			if (left){
				bit_set(bitVector, dic->getLeft(rule + alph) - alph);
			}
		}else{
			left=false;
		}
		
		if (dic->getRight(rule+alph)>=alph){
			right = !bitget(bitVector,dic->getRight(rule+alph)-alph);
			if (right){
				bit_set(bitVector, dic->getRight(rule + alph) - alph);
			}
		}else{
			right=false;
		}
	}
	if (left) sampleOverheadsRec(dic->getLeft(rule+alph),depth+1,sample,bitVector);
		
	if (right) sampleOverheadsRec(dic->getRight(rule+alph),depth+1,sample,bitVector);
	
	return 0;
}

/**
 * It computes the bitmap that marks those rules that has to be sampled and store
 * that samples in the corresponding that structures. 
 * 
 * */
size_t GCT::sampleOverheads(uint sample, uint *INVPERM){
	assert(uint_len(n-alph+1,1));
	uint *bitVector = new uint[uint_len(n-alph+2,1)];
	for (uint i=0;i<uint_len(n-alph+2,1);i++) bitVector[i]=0;
	if (sample>0){
		for (uint i=0;i<n-alph;i++)
			sampleOverheadsRec(i+alph,1,sample,bitVector);
	}else{
		for (int i=0;i<uint_len(n-alph+1,1);i++)
			bitVector[i]=~0;
		bitclean(bitVector,n-alph);	
	}
	bd = new BitSequenceRG(bitVector,n-alph+1,21);
	uint totalSamples = bd->rank1(n-alph);
	#ifdef VERBOSE
		cerr << "Total samples rules: " << totalSamples << endl;
	#endif
	totalSampledRuleBitsInBits = uint_len(totalSamples+1,1)*sizeof(uint)*8;
	sampledRuleFstBit = new uint[uint_len(totalSamples+1,1)];
	sampledRuleLstBit = new uint[uint_len(totalSamples+1,1)];
	for (int i=0;i<uint_len(totalSamples+1,1);i++){
		sampledRuleFstBit[i]=0;
		sampledRuleLstBit[i]=0;
	}
	
	totalSamples++;
	sampledMinExcess  = new int[totalSamples];
	sampledTotalExcess  = new int[totalSamples];
	sampledLengths = new uint[totalSamples];
	sampledLeafs = new uint[totalSamples];
	for (int i=0;i<totalSamples;i++){
		sampledMinExcess[i]=sampledTotalExcess[i]=sampledLengths[i]=sampledLeafs[i]=0;
	}
	totalSamples--;
	//I add a superfluous rule at the end of the vector of lengths (util in rmq)
	for (size_t i=0;i<totalSamples;i++){
		assert((bd->select1(i+1)+alph)<n);
		assert(INVPERM[bd->select1(i+1)+alph]<n);
		sampledLengths[i]=lengths[INVPERM[bd->select1(i+1)+alph]];
		sampledLeafs[i]=ruleLeafs[bd->select1(i+1)+alph];
		sampledMinExcess[i]=minExcess[bd->select1(i+1)+alph];
		sampledTotalExcess[i]=totalExcess[bd->select1(i+1)+alph];
		if (bitget(ruleFstBit,bd->select1(i+1))) bit_set(sampledRuleFstBit, i);
		if (bitget(ruleLstBit,bd->select1(i+1))) bit_set(sampledRuleLstBit, i);
		#if 0
		cout << "rule-" << bd->select1(i+1)+alph << ", length:  " << sampledLengths[i] << ", zeroes: " << sampledZeroes[i] << endl;
		cout << "rule-" << bd->select1(i+1)+alph << ", minE:  " << sampledMinExcess[i] << ", totalE: " << sampledTotalExcess[i] << endl;
		#endif
	}
	
	//~ sampledLengths[totalSamples]=maxL;
	//~ totalSamples++;
	return 0;
}


/** Returns the min excess and, in case of being a terminal rule, it returns
 * the possition where the minium occurs*/
int GCT::getMinExcess(uint rule, int &mine, int &acum_excess, int &pos){
	//in case of terminals, we should return the min and the pos in which 
	//the min occurs
	if (rule<alph){ 
		acum_excess+=EXCESS[rule];
		if (acum_excess<mine){
			mine=acum_excess;
			pos=1;
		}
	}else{
		 
		 if (bd->access(rule-alph)) {
			rule-=alph;
			uint r = bd->rank1(rule)-1;
			int min_excess = MIN_EXCESS-compressedRulesMinExcess->access(r);
			int m;
			if (mine>(m=acum_excess+min_excess))
				mine=m;
			acum_excess += (compressedSampledTotalExcess->access(r)+min_excess);
			
		}else{ 
			getMinExcess(dic->getLeft(rule),mine,acum_excess,pos);
			getMinExcess(dic->getRight(rule),mine,acum_excess,pos);
			
		}
		
	
	}
	return mine;
}
int GCT::getTotalExcessAndLength(uint rule,int &acum_excess,size_t &length){
	if (rule<alph){ 
		acum_excess+=EXCESS[rule];
		length++;
	}else{
		 if (bd->access(rule-alph)) {
			rule-=alph;
			uint r = bd->rank1(rule)-1;
			int min_excess = MIN_EXCESS-compressedRulesMinExcess->access(r);
			acum_excess += (compressedSampledTotalExcess->access(r)+min_excess);
			length+=compressedSampledLengths->access(r);
		}else{ 
			getTotalExcessAndLength(dic->getLeft(rule),acum_excess,length);
			getTotalExcessAndLength(dic->getRight(rule),acum_excess,length);
			
		}
	}
	return acum_excess;
	
}


uint GCT::getRuleLeaf(size_t rule, uint &leaf, uchar &fstb, uchar &lstb){
	if (rule<alph){ 
		fstb=lstb=rule;
		leaf=rule;
	}else{		 
		size_t t1;
		 if (bd->access(t1=(rule-alph))) {
			rule=t1;
			uint r = bd->rank1(rule)-1;
			leaf+=compressedRuleLeafs->access(r);
			size_t d,res;
			d=rule>>5;res=rule&mask31;
			fstb=(ruleFstBit[d]>>res)&1;
			lstb=(ruleLstBit[d]>>res)&1;
		}else{ 
			uchar lstb1,fstb2;
			getRuleLeaf(dic->getLeft(rule),leaf,fstb,lstb1);
			getRuleLeaf(dic->getRight(rule),leaf,fstb2,lstb);
			leaf+=GET_OVERLAP(lstb1,fstb2);
		}
	}
	return leaf;
}

int GCT::getRuleAll(uint rule,int &mine,int &acum_excess,size_t &length,uint &leafs,uchar &fstb,uchar &lstb){
	if (rule<alph){ 
		//brute force
		//~ mine=getMinExcessTerminal(rule,mine,acum_excess,pos,getLenTerms(rule+1))
		int t1;
		if ((t1=acum_excess+EXCESS[rule])<mine){
			mine=t1;
		}
		acum_excess=t1;
		length++;
		fstb=lstb=rule;
	}else{
		 
		 if (bd->access(rule-alph)) {
			rule-=alph;
			uint r = bd->rank1(rule)-1;
			int min_excess = MIN_EXCESS-compressedRulesMinExcess->access(r);
			int m;
			if (mine>(m=acum_excess+min_excess))
				mine=m;
			acum_excess += (compressedSampledTotalExcess->access(r)+min_excess);
			length+=compressedSampledLengths->access(r);
			leafs+=compressedRuleLeafs->access(r);
			//insert the code of bitget to avoid operations (W=32 --> optimize divisions)
			size_t d,res;
			d=rule>>5;res=rule&mask31;
			fstb=(ruleFstBit[d]>>res)&1;
			lstb=(ruleLstBit[d]>>res)&1;
			
		}else{ 
			uchar lstb1,fstb2;
			getRuleAll(dic->getLeft(rule),mine,acum_excess,length,leafs,fstb,lstb1);
			getRuleAll(dic->getRight(rule),mine,acum_excess,length,leafs,fstb2,lstb);
			leafs+=GET_OVERLAP(lstb1,fstb2);
			
		}
	}
	return mine;
}

int GCT::getRuleAllButLength(uint rule,int &mine,int &acum_excess,uint &leafs,uchar &fstb,uchar &lstb){
	if (rule<alph){ 
		//brute force
		//~ mine=getMinExcessTerminal(rule,mine,acum_excess,pos,getLenTerms(rule+1))
		int t1;
		if ((t1=acum_excess+EXCESS[rule])<mine){
			mine=t1;
		}
		acum_excess=t1;
		fstb=lstb=rule;
	}else{
		 
		 if (bd->access(rule-alph)) {
			rule-=alph;
			uint r = bd->rank1(rule)-1;
			int min_excess = MIN_EXCESS-compressedRulesMinExcess->access(r);
			int m;
			if (mine>(m=acum_excess+min_excess))
				mine=m;
			acum_excess += (compressedSampledTotalExcess->access(r)+min_excess);
			leafs+=compressedRuleLeafs->access(r);
			//insert the code of bitget to avoid operations (W=32 --> optimize divisions)
			size_t d,res;
			d=rule>>5;res=rule&mask31;
			fstb=(ruleFstBit[d]>>res)&1;
			lstb=(ruleLstBit[d]>>res)&1;
			
		}else{ 
			uchar lstb1,fstb2;
			getRuleAllButLength(dic->getLeft(rule),mine,acum_excess,leafs,fstb,lstb1);
			getRuleAllButLength(dic->getRight(rule),mine,acum_excess, leafs,fstb2,lstb);
			leafs+=GET_OVERLAP(lstb1,fstb2);
			
		}
	}
	return mine;
}

int GCT::getRuleAllButMin(uint rule,int &acum_excess,size_t &length,uint &leafs,uchar &fstb,uchar &lstb){
	if (rule<alph){ 
		//brute force
		//~ mine=getMinExcessTerminal(rule,mine,acum_excess,pos,getLenTerms(rule+1))
		acum_excess+=EXCESS[rule];
		length++;
		fstb=lstb=rule;
	}else{
		 size_t t1;
		 if (bd->access(t1=(rule-alph))) {
			rule=t1;
			uint r = bd->rank1(rule)-1;
			int min_excess = MIN_EXCESS-compressedRulesMinExcess->access(r);
			acum_excess += (compressedSampledTotalExcess->access(r)+min_excess);
			length+=compressedSampledLengths->access(r);
			leafs+=compressedRuleLeafs->access(r);
			//insert the code of bitget to avoid operations (W=32 --> optimize divisions)
			//	#define bitget(e,p) ((((e)[(p)/W] >> ((p)%W))) & 1)
			size_t d,res;
			d=rule>>5;res=rule&mask31;
			fstb=(ruleFstBit[d]>>res)&1;
			lstb=(ruleLstBit[d]>>res)&1;
			
		}else{ 
			uchar lstb1,fstb2;
			getRuleAllButMin(dic->getLeft(rule),acum_excess,length,leafs,fstb,lstb1);
			getRuleAllButMin(dic->getRight(rule),acum_excess,length,leafs,fstb2,lstb);
			leafs+=GET_OVERLAP(lstb1,fstb2);
			
		}
	}
	return acum_excess;
}

int GCT::getRuleLengthAndLeaf(uint rule,size_t &length,uint &leaves,uchar &fstb,uchar &lstb){
	if (rule<alph){ 
		length++;
		fstb=lstb=rule;
	}else{
		 size_t t1;
		 if (bd->access(t1=(rule-alph))) {
			rule=t1;
			uint r = bd->rank1(rule)-1;
			length+=compressedSampledLengths->access(r);
			leaves+=compressedRuleLeafs->access(r);
			size_t d,res;
			d=rule>>5;res=rule&mask31;
			fstb=(ruleFstBit[d]>>res)&1;
			lstb=(ruleLstBit[d]>>res)&1;
			
		}else{ 
			uchar lstb1,fstb2;
			getRuleLengthAndLeaf(dic->getLeft(rule),length,leaves,fstb,lstb1);
			getRuleLengthAndLeaf(dic->getRight(rule),length,leaves,fstb2,lstb);
			leaves+=GET_OVERLAP(lstb1,fstb2);
			
		}
	}
	return leaves;
}

int GCT::getMinTotalExcessAndLength(uint rule, int &mine, int &acum_excess,size_t &length){
	//in case of terminals, we should return the min and the pos in which 
	//the min occurs
	if (rule<alph){ 
		//brute force
		//~ mine=getMinExcessTerminal(rule,mine,acum_excess,pos,getLenTerms(rule+1))
		int t1;
		if ((t1=acum_excess+EXCESS[rule])<mine){
			mine=t1;
		}
		acum_excess=t1;
		length++;
		
	}else{
		 
		 if (bd->access(rule-alph)) {
			rule-=alph;
			uint r = bd->rank1(rule)-1;
			int min_excess = MIN_EXCESS-compressedRulesMinExcess->access(r);
			int m;
			if (mine>(m=acum_excess+min_excess))
				mine=m;
			acum_excess += (compressedSampledTotalExcess->access(r)+min_excess);
			length+=compressedSampledLengths->access(r);
		}else{ 
			getMinTotalExcessAndLength(dic->getLeft(rule),mine,acum_excess,length);
			getMinTotalExcessAndLength(dic->getRight(rule),mine,acum_excess,length);
			
		}
	}
	return mine;
}

int GCT::getMinTotalExcess(uint rule, int &mine, int &acum_excess){
	//in case of terminals, we should return the min and the pos in which 
	//the min occurs
	if (rule<alph){ 
		//brute force
		//~ mine=getMinExcessTerminal(rule,mine,acum_excess,pos,getLenTerms(rule+1));
		mine=EXCESS[rule];
		acum_excess+=mine;
		
	}else{
		 
		 if (bd->access(rule-alph)) {
			rule-=alph;
			uint r = bd->rank1(rule)-1;
			int min_excess=MIN_EXCESS-compressedRulesMinExcess->access(r);
			int m;
			if (mine>(m=acum_excess+min_excess))
				mine=m;
			acum_excess += (compressedSampledTotalExcess->access(r)+min_excess);
		}else{ 
			getMinTotalExcess(dic->getLeft(rule),mine,acum_excess);
			getMinTotalExcess(dic->getRight(rule),mine,acum_excess);
			
		}
	}
	return mine;
}

int GCT::getMinExcessTerminalTest(uint rule, int &mine, int &acum_excess, int &pos, int lim){
	int me=0,te=0;
		//in case of terminals, we should return the min and the pos in which 
		//the min occurs
		int e = bitget(terminals,lenTerms[rule])?1:-1;
		uint ppos,i;
		ppos=lenTerms[rule];
		i=lenTerms[rule]+1;
		me=te= e;
		for (;i<lim;i++){
			e =bitget(terminals,i)?1:-1; 
			te+=(int)e;
			if (te<me){
				me =te;
				ppos=i;
			}
		}
		if (me+acum_excess<mine){
			mine=me+acum_excess;
			pos=ppos-lenTerms[rule];
		}
		acum_excess = acum_excess+te;
		return 0;
}

inline int GCT::getMinExcess(uint rule){
	//~ int pos;
	//~ getMinExcess(rule,min_e,acum_e,pos);
	//~ 
	//~ assert(min_e==minExcess[rule]);
	//~ return min_e;
	int mine=INT_MAX;
	if (rule<alph){ 
		int acum_excess=0;
		acum_excess+=EXCESS[rule];
		if (acum_excess<mine){
			mine=acum_excess;
		}
	}else{
		 
		 if (bd->access(rule-alph)) {
			rule-=alph;
			uint r = bd->rank1(rule)-1;
			mine=(MIN_EXCESS-compressedRulesMinExcess->access(r));
			
		}else{ 
			int acum_excess=0,pos=0;
			getMinExcess(dic->getLeft(rule),mine,acum_excess,pos);
			getMinExcess(dic->getRight(rule),mine,acum_excess,pos);
			
		}
	}
	return mine;
}

int GCT::getMinExcess(uint rule, int &pos){
	int acum_e=0,min_e=INT_MAX;
	getMinExcess(rule,min_e,acum_e,pos);
	//~ assert(min_e==minExcess[rule]);
	return min_e;
}

int GCT::getMaxExcess(uint rule){
	return maxExcess[rule];
}

int GCT::getTotalExcess(uint rule){
	int te=0; 
	if (rule<alph){
		//TODO @date
		te = EXCESS[rule];
		//te = getTotalExcessRankTerminals(rule,EXCESS,getLenTerms(rule+1));
	}
	else if (bd->access(rule-alph)) {
		//TODO comment the following line and uncomment the next to it
		uint r = bd->rank1(rule-alph)-1;
		//~ te = sampledTotalExcess[];
		int min_excess = MIN_EXCESS-compressedRulesMinExcess->access(r);
		te = compressedSampledTotalExcess->access(r)+min_excess;
		//~ int te2 = compressedSampledTotalExcessDiff->access(r)+compressedRulesMinExcess->access(r);
		//~ if (te2!=te) cout << "Error in gettotalExcess" << "(t1: " << te << ", t2: " << te2 << endl;
		//~ assert(te==sampledTotalExcess[bd->rank1(rule)-1]);
	}else 
		te = getTotalExcess(dic->getLeft(rule))+getTotalExcess(dic->getRight(rule));
	return te;
}

size_t GCT::getLength(uint rule){
	uint l;
	if (rule<alph) 
		l = 1;
	else if (bd->access(rule-alph)) {
		l = compressedSampledLengths->access(bd->rank1(rule-alph)-1);
	}else 
		l = getLength(dic->getLeft(rule))+getLength(dic->getRight(rule));
	return l;	
}

size_t GCT::getSampleL(size_t pos){
	return samplesLen[pos];
}

size_t GCT::getSampleR(size_t pos){
	return samplesZeroes[pos];
}

size_t GCT::findPosInCByOnePos(size_t izero, size_t &sumZeroes, size_t &sumLengths){
	size_t l,r,p; 
	l=0;
	r=nSamples;
	p=(l+r)/2;
	while(l<r){
		if (izero<getSampleL(p)-getSampleR(p)){
			r=p-1;
		}else if (izero==getSampleL(p)-getSampleR(p)){
			sumLengths = getSampleL(p);
			sumZeroes = getSampleL(p)-getSampleR(p);
			return p*sampleRateC;
		}else{
			l = p+1;
		}
		p=(l+r)/2;
	}
	
	if (getSampleL(p)-getSampleR(p)>izero) p--;
	sumLengths = getSampleL(p);
	sumZeroes = getSampleL(p)-getSampleR(p);
	
	
	for (size_t i=p*sampleRateC;;i++){
		if (sumZeroes+lengths[getC(i)]-zeroes[getC(i)]>izero)
			return i;
		else{
			sumLengths+=lengths[getC(i)];
			sumZeroes +=lengths[getC(i)]-zeroes[getC(i)];
		}
	}
}


size_t GCT::findPosInCByZeroPos(size_t izero, size_t &sumZeroes, size_t &sumLengths){
	size_t l,r,p; 
	l=0;
	r=nSamples;
	p=(l+r)/2;
	while(l<r){
		if (izero<getSampleR(p)){
			r=p-1;
		}else if (izero==getSampleR(p)){
			sumLengths = getSampleL(p);
			sumZeroes = getSampleR(p);
			return p*sampleRateC;
		}else{
			l = p+1;
		}
		p=(l+r)/2;
	}
	
	if (getSampleR(p)>izero) p--;
	sumLengths = getSampleL(p);
	sumZeroes = getSampleR(p);
	
	
	for (size_t i=p*sampleRateC;;i++){
		if (sumZeroes+zeroes[getC(i)]>izero)
			return i;
		else{
			sumLengths+=lengths[getC(i)];
			sumZeroes +=zeroes[getC(i)];
		}
	}
}

size_t GCT::findPosInCByPos(size_t pos, size_t &sumZeroes, size_t &sumLengths){
	size_t l,r,p; 
	l=0;
	r=nSamples;
	p=(l+r)/2;
	while(l<r){
		if (pos<getSampleL(p)){
			r=p-1;
		}else if (pos==getSampleL(p)){
			sumLengths = getSampleL(p);
			sumZeroes = getSampleR(p);
			return p*sampleRateC;
		}else{
			l = p+1;
		}
		p=(l+r)/2;
	}
	
	if (getSampleL(p)>pos) p--;
	sumLengths = getSampleL(p);
	sumZeroes = getSampleR(p);
	
	
	for (size_t i=p*sampleRateC;;i++){
		if (sumLengths+lengths[getC(i)]>pos)
			return i;
		else{
			sumLengths+=lengths[getC(i)];
			sumZeroes +=zeroes[getC(i)];
		}
	}
}
//i \in [1,#0's]
size_t GCT::select0(size_t i, size_t &excess){
	assert(i>0);
	i<<=1;
	int c1,p=0,l,lens,acum_pos=0,pos_bottom;
	c1=heapDepth-1;
	//~ cminus1=c1;
	int leftmost_block=lengthsHeapLevels[1]-1;
	
	while(c1>0){
		p<<=1;
		c1--;
		int pplus1=p+1;
		pos_bottom=min(pplus1<<c1,leftmost_block);
		lens = (pos_bottom<<blogBlockSizeC)-getSampleC_o(pos_bottom);
		l = getSampleC_e(pos_bottom);
		if (i>(excess=(lens-l))){
			p=pplus1;
		}
	}
	
	p=min(p,leftmost_block);
	acum_pos = (p<<blogBlockSizeC)-getSampleC_o(p);
	excess = getSampleC_e(p);
	size_t posInC = getSampleC_p(p);
	size_t rule = getC(posInC);
	int t1,t2;
	while(((t1=getLength(rule)+acum_pos)-(t2=getTotalExcess(rule)+excess))<i){
		acum_pos=t1;
		excess=t2;
		rule=getC(++posInC);
	}
	return searchRuleDown_s0(rule,acum_pos,excess,i);
	
}

size_t GCT::searchRuleDown_s0(size_t rule, size_t acum_pos, size_t &acum_e, size_t target){
	
	int t1,t2,t3;
	int leftlen;
	size_t r = rule;
	while(r>=alph){
		int lefte = getTotalExcess((t1=dic->getLeft(r)));
		leftlen = getLength(t1);
		if ((t2=acum_pos+leftlen)-(t3=acum_e+lefte)>=target){
			r=t1;
		}else{
			acum_pos=t2;
			acum_e=t3;
			r=dic->getRight(r);
		}
	}
	//~ if (!r) acum_pos++;
	//~ if ((t2-t3)!=target)acum_pos++;
	acum_e+=EXCESS[r];
	return acum_pos;
}

size_t GCT::searchRuleDown_s1(size_t rule, size_t acum_pos, size_t &acum_e, size_t target){
	
	int t1,t2,t3;
	int leftlen;
	size_t r = rule;
	while(r>=alph){
		int lefte = getTotalExcess((t1=dic->getLeft(r)));
		leftlen = getLength(t1);
		if ((t3=acum_e+lefte)+(t2=acum_pos+leftlen)>=target){
			r=t1;
		}else{
			acum_pos=t2;
			acum_e=t3;
			r=dic->getRight(r);
		}
	}
	//~ if (!r) acum_pos++;
	//~ if ((t2-t3)!=target)acum_pos++;
	acum_e+=EXCESS[r];
	return acum_pos;
}


size_t GCT::select1(size_t i,size_t &excess){
	
	assert(i>0);
	i<<=1;
	int c1,p=0,l,lens,acum_pos=0,pos_bottom;
	c1=heapDepth-1;
	int leftmost_block=lengthsHeapLevels[1]-1;
	while(c1>0){
		p<<=1;
		c1--;
		pos_bottom=min((p+1)<<c1,leftmost_block);
		lens = (pos_bottom<<blogBlockSizeC)-getSampleC_o(pos_bottom);
		l = getSampleC_e(pos_bottom);
		if (i>(excess=(lens+l))){
			p++;
		}
	}
	
	p=min(p,leftmost_block);
	acum_pos = (p<<blogBlockSizeC)-getSampleC_o(p);
	excess = getSampleC_e(p);
	size_t posInC = getSampleC_p(p);
	size_t rule = getC(posInC);
	int t1,t2;
	while(((t1=getLength(rule)+acum_pos)+(t2=getTotalExcess(rule)+excess))<i){
		acum_pos=t1;
		excess=t2;
		rule=getC(++posInC);
	}
	return searchRuleDown_s1(rule,acum_pos,excess,i);
}

inline int GCT::getSampleC_e(uint pos){
	//~ return sampleC_e[pos];
	return heapSampleC_e->access(pos);
}

inline int GCT::getSampleC_p(uint pos){
	//~ return sampleC_p[pos];
	return compressedSampleC_p->access(pos);
}

inline int GCT::getSampleC_o(uint pos){
	//~ return sampleC_o[pos];
	return compressedSampleC_o->access(pos);
}
inline int GCT::getSampleC_minE(uint pos){
	//~ return sampleC_minE[pos];
	//~ return heapSampleC_minE->access(pos);
	return (int)(*heapSampleC_minE_array)[pos];
}

inline int GCT::getSampleC_leaf(uint pos){
	//~ return sampleC_minE[pos];
	//~ return heapSampleC_minE->access(pos);
	return compressedSampleC_leafs->access(pos);
}

size_t GCT::getExcess(size_t pos){
	size_t sumL,posInC,rule;
	int p,excess;
	findPos_fwd(pos,sumL,p,excess,rule,posInC);
	assert(rule==1 || rule == 0);
	excess+=EXCESS[rule];
	return excess;
}
size_t GCT::rank0(size_t pos){
	uint bit;int excess;
	return internal_rank0(pos, excess, bit);
}

inline size_t GCT::internal_rank0(size_t pos, int &excess, uint &bit){
	size_t sumL,posInC,p;
	int sumZ;
	p = pos >> blogBlockSizeC;
	posInC=getSampleC_p(p);
	sumL = (p<<blogBlockSizeC)-getSampleC_o(p);
	sumZ = getSampleC_e(p);
	size_t rule = getC(posInC);
	size_t lenr;
	while((lenr=sumL+getLength(rule)) <= pos){
		sumL =lenr;
		sumZ +=getTotalExcess(rule);
		rule=getC(++posInC);
	}
	
	while(rule>=alph){
		size_t left;size_t l;
		left = getLength((l=dic->getLeft(rule)));
		size_t t1;
		if ((t1=sumL+left)<=pos){
			sumL=t1;
			sumZ+=getTotalExcess(l);
			rule=dic->getRight(rule);
		}else{
			rule=l;
		}
	}
	assert(rule>=0 && rule<alph);
	bit=rule;
	excess = sumZ+EXCESS[rule];
	return (pos+1-excess)>>1;
}

/**
 * Returns wheter a 0/1 occurs at position pos of the bitmap 
 * */
ulong GCT::access(uint pos){
	
	size_t sumL,posInC,p;
	p = pos >> blogBlockSizeC;
	posInC=getSampleC_p(p);
	sumL = (p<<blogBlockSizeC)-getSampleC_o(p);
	size_t rule = getC(posInC);	
	size_t lenr;
	while((lenr=sumL+getLength(rule)) <= pos){
		sumL =lenr;
		rule=getC(++posInC);
	}
	
	while(rule>=alph){
		size_t left;size_t l;
		left = getLength((l=dic->getLeft(rule)));
		size_t t1;
		if ((t1=sumL+left)<=pos){
			sumL=t1;
			rule=dic->getRight(rule);
		}else{
			rule=l;
		}
	}
	return rule;
}

ulong GCT::excessAt(uint pos){
	int excess=0; uint bit=0;
	internal_rank0(pos,excess,bit);
	return excess;
}

/**
 * Peforms and acces but also a leaf_rank up to position "i". 
 * 
 * */
size_t GCT::access_and_leaf_rank(size_t i, uint &leaf){
	
	size_t acump;
	uint acumLeafs;
	uchar fstb,prev,lstb;
	size_t posInC, sumL, rule;
	size_t p = i >>blogBlockSizeC;
	posInC=getSampleC_p(p);
	sumL = (p<<blogBlockSizeC)-getSampleC_o(p);
	leaf = getSampleC_leaf(p);
	rule = getC(posInC);	
	assert(bsLeafsTest==nullptr || leaf==bsLeafsTest->rank1(sumL));
	size_t l;
	acump=sumL;acumLeafs=leaf;
	prev=0;
	getRuleLengthAndLeaf(rule,acump,acumLeafs,fstb,lstb);
	while(acump <= i){
		sumL =acump;
		updateLeaf(leaf,acumLeafs,prev,fstb,lstb);
		rule=getC(++posInC);
		getRuleLengthAndLeaf(rule,acump,acumLeafs,fstb,lstb);
		assert(bsLeafsTest==nullptr || leaf==bsLeafsTest->rank1(sumL));
	}
	assert(bsLeafsTest==nullptr || leaf==bsLeafsTest->rank1(sumL));
	
	leaf+=GET_OVERLAP(prev,fstb);
	prev=0;
	while(rule>=alph){
		assert(bsLeafsTest==nullptr || leaf==bsLeafsTest->rank1(sumL));
		l=dic->getLeft(rule);
		acump=sumL;acumLeafs=leaf;
		getRuleLengthAndLeaf(l,acump,acumLeafs,fstb,lstb);
		if (acump<=i){
			sumL=acump;
			updateLeaf(leaf,acumLeafs,prev,fstb,lstb);
			rule=dic->getRight(rule);
		}else{
			rule=l;
		}
	}
	leaf+=GET_OVERLAP(prev,rule);
	//TODO: need to correct leaf as:
	leaf+=(rule==1 && !access(i+1))?1:0;
	return rule;
}
	
size_t GCT::access(size_t pos, int &excess){
	uint bit;
	internal_rank0(pos,excess,bit);
	return bit;
}

/**
 * Given a position in [1,4n], it computes the rank of leafs up to 
 * the given position
 * */
ulong GCT::brLeafRank(uint pos){
	uint leaf;
	access_and_leaf_rank(pos,leaf);	
	return leaf;
}

/**
 * It returns the position where leaf "i" is. A leaf is identified as 
 * a pair of 10, and the position returned is the one of the "1". 
 * 
 * */
ulong GCT::brLeafSelect(uint i){
	//we first binary search the samples of leaves.
	uint l,r,p=-1;
	l=0;r=nSamplesC-1;
	uint s;
	while(l<=r){
		p=((l+r)>>1);
		s=getSampleC_leaf(p);
		if (s<i){
			l=p+1;
		}else if (s>i){
			r=p-1;
		}else{
			break;
		}
		
	}
	//TODO Improve this part: 
	//revisar esto: me tengo que colocar en una posición con menos hojas de las que estoy buscando. 
	//para empezar esto ya debería ir en la búsqueda binaria. 
	while (s>=i){
		p--;
		s = getSampleC_leaf(p);
	}
	/**
	 * Set up the pointers and start a sequential search acumulating the excess, number of leaves, 
	 * position, ...
	 * */
	size_t acump;
	uint acumLeafs;
	uchar fstb,prev,lstb;
	size_t sumL = (p<<blogBlockSizeC)-getSampleC_o(p);
	uint posInC=getSampleC_p(p);
	size_t rule = getC(posInC);	
	
	//prev=0 since the first time we do not have to add the overlap (it is included in sampleC_p[p]
	prev=0;
	uint leaf = getSampleC_leaf(p);

	acump=sumL;acumLeafs=leaf;
	getRuleLengthAndLeaf(rule,acump,acumLeafs,fstb,lstb);

	while(acumLeafs<i){
		sumL = acump;
		updateLeaf(leaf,acumLeafs,prev,fstb,lstb);
		assert(bsLeafsTest==nullptr || leaf == bsLeafsTest->rank1(sumL));
		rule=getC(++posInC);
		getRuleLengthAndLeaf(rule,acump,acumLeafs,fstb,lstb);
	}
	leaf+=GET_OVERLAP(prev,fstb);
	//we found the rule where the leaf is -> search down moving left or right 
	//depending on the number of leaves of each subrule
	while(rule>=alph){
		assert(bsLeafsTest==nullptr || leaf==bsLeafsTest->rank1(sumL));
		l=dic->getLeft(rule);
		acump=sumL;acumLeafs=leaf;
		getRuleLengthAndLeaf(l,acump,acumLeafs,fstb,lstb);		
		if (acumLeafs<i){
			sumL=acump;
			updateLeaf(leaf,acumLeafs,prev,fstb,lstb);
			rule=dic->getRight(rule);
		}else{
			rule=l;
		}
	}
	leaf+=GET_OVERLAP(prev,rule);
	//Given a leaf "10" the bit in the bitmap bsLeafTest is set in the position of the 0. 
	assert(bsLeafsTest==nullptr || leaf==bsLeafsTest->rank1(sumL));
	if (rule==0)sumL--;
	assert(bsExcessBuild->access(sumL));
	return sumL;
	
}
/**isLeaf(
 * Requires P[i]=1
 * Returns whether a position i is a leaf or not (the bit at i is  set to 0 
 * -recall P[i]=1 is required-)
 * */
bool GCT::isLeaf(uint i, uint &leaf_rank){
	bool ret = access_and_leaf_rank(i+1,leaf_rank);
	//assert(bsLeafsTest==nullptr || leaf_rank <= bsLeafsTest->rank1(len-1));
	return !ret;
}

/**
 * Moves down in a rule hierachy to determine if the bit at pos target is set.
 * */
size_t GCT::searchRuleDown_access(size_t rule, size_t acum_pos, size_t &acum_e, size_t target){
	
	int t1,t2,t3;
	int leftlen;
	size_t r = rule;
	while(r>=alph){
		int lefte = getTotalExcess((t1=dic->getLeft(r)));
		leftlen = getLength(t1);
		if ((t2=acum_pos+leftlen)-(t3=acum_e+lefte)>=target){
			r=t1;
		}else{
			acum_pos=t2;
			acum_e=t3;
			r=dic->getRight(r);
		}
	}
	//~ if (!r) acum_pos++;
	//~ if ((t2-t3)!=target)acum_pos++;
	acum_e+=EXCESS[r];
	return r;
}


/**
 * Computes the rank0 in the tree bitmap. It yields in 
 * excess the excess of that position (recall excess = #1-#0)
 * 
 * */
size_t GCT::rank0(size_t pos, size_t &excess){
	
	//Find the block where position is contained in O(1)
	size_t sumZ,sumL,posInC,p;
	p = pos >> blogBlockSizeC;
	posInC=getSampleC_p(p);
	sumL = (p<<blogBlockSizeC)-getSampleC_o(p);
	//we acumulate the excess instead of the number of 0's (the excess is what
	//we have stored). 
	sumZ = getSampleC_e(p);
	size_t rule = getC(posInC);	
	size_t lenr;
	
	//TODO getLength and getTotalExcess in one call
	//search forward until reach a rule that overtakes the searched position
	while((lenr=sumL+getLength(rule)) <= pos){
		sumL =lenr;
		sumZ +=getTotalExcess(rule);
		rule=getC(++posInC);
	}
	
	//search the rule down moving left/right depending on the acumulated position
	while(rule>=alph){
		size_t left;size_t l;
		left = getLength((l=dic->getLeft(rule)));
		size_t t1;
		if ((t1=sumL+left)<=pos){
			sumL=t1;
			sumZ+=getTotalExcess(l);
			rule=dic->getRight(rule);
		}else{
			rule=l;
		}
	}
	assert(rule>=0 && rule<alph);
	//we always finish in a leaf, which is a terminal rule (in this case rule={0|1}}
	excess=sumZ+EXCESS[rule];
	//finally, we translate the excess into the rank0 -> #0 = (pos+1-excess)/2
	return (pos+1-excess)>>1;
}

/**
 * Computes the rank1 from rank0
 * */
size_t GCT::rank1(size_t pos){
	return pos-rank0(pos)+1;
}

/**
 * Computes the rank1 and stores in excess the excess at position pos.
 * */
size_t GCT::rank1(size_t pos,size_t &excess){
	return pos-rank0(pos,excess)+1;
}

/**
 * Find the position "i" in the original sequence of bits, reporting the 
 * position in the vector C where it occurs, the excess at that psition, the leaf rule that 
 * points to that position and a recursion stack storing the parent of the left rules we 
 * take when we search down a rule. 
 * */


inline size_t GCT::findPos_fwd(size_t i,size_t &sumL, int &p, int &excess, size_t &rule, size_t &posInC){
	//find the block in O(1)
	p = i >>blogBlockSizeC;
	posInC=getSampleC_p(p);
	sumL = (p<<blogBlockSizeC)-getSampleC_o(p);
	excess = getSampleC_e(p);
	rule = getC(posInC);	
	size_t l;
	//mover forward updating excesses, acumPos (sumL), etc.
	while((l=sumL+getLength(rule)) <= i){
		sumL=l;
		excess +=getTotalExcess(rule);
		rule=getC(++posInC);
		assert(getExcessBuild(sumL-1)==excess);
	}
	p=0;
	//we found the rule, now we search inside it
	//move down in the rule hierarchy
	while(rule>=alph){
		size_t left = getLength((l=dic->getLeft(rule)));
		size_t t1;
		if ((t1=sumL+left)<=i){
			sumL=t1;
			excess+=getTotalExcess(l);//getZeroes(rules->get_left(rule-alph));
			rule=dic->getRight(rule);
		}else{
			//if we move left, we stotr the parent of rule l (which is "rule")
			stackRulePath[p]=rule;
			stackRuleValues[p++]=sumL;
			rule=l;
		}
	}
	excess+=EXCESS[rule];
	
	return sumL;
}

/**
 * Performs the sampe that findPos_fwd but if we know we are not going to find a given excess e in the rule that covers
 * position "i", we simply return not filling the recursion stack. It works well since it also returns p=0, telling 
 * the following algorithm it does not have to search the recursion stack. 
 * 
 * */

inline size_t GCT::findPos_fwd_with_e(size_t i,size_t &sumL, int &p, int &excess, size_t &rule, size_t &posInC, int e,size_t &rulei){

	int acume;size_t acump;
	p = i >>blogBlockSizeC;
	posInC=getSampleC_p(p);
	//TODO better store log2(blockSize) and carry out this as a shift (blockSize must be a power of 2) 
	sumL = (p<<blogBlockSizeC)-getSampleC_o(p);
	excess = getSampleC_e(p);
	rule = getC(posInC);	
	size_t l;
	acume=0;acump=sumL;
	getTotalExcessAndLength(rule,acume,acump);
	while(acump <= i){
		sumL =acump;
		excess +=acume;//getTotalExcess(rule);
		rule=getC(++posInC);
		if (sumL>0)assert(getExcessBuild(sumL-1)==excess);
		acume=0;
		getTotalExcessAndLength(rule,acume,acump);
	}
	p=0;
	if (e>=0 && excess+getMinExcess(rule)>e){
		 excess+=acume;//getTotalExcess(rule);
		 sumL=acump-1;//getLength(rule)-1;
		 return 0;
	}
	//move down in the rule hierarchy
	while(rule>=alph){
		l=dic->getLeft(rule);
		acume=0;acump=sumL;
		getTotalExcessAndLength(l,acume,acump);
		
		if (acump<=i){
			sumL=acump;
			excess+=acume;//getTotalExcess(l);//getZeroes(rules->get_left(rule-alph));
			rule=dic->getRight(rule);
		}else{
			stackRulePath[p]=rule;
			stackRuleValues[p++]=sumL;
			rule=l;
		}
	}

	rulei=rule; 
	excess+=EXCESS[rule];
	assert(getExcessBuild(sumL)==excess);	
	return 0;
}
/**
 * The same but searching backwards
 * */
inline size_t GCT::findPos_bwd_with_e(size_t i, size_t &sumL, int &p, int &excess, size_t &rule, size_t &posInC,int e,size_t &eIni){
	int acume;size_t acump;
	p = i >>blogBlockSizeC;
	posInC=getSampleC_p(p);
	sumL = (p<<blogBlockSizeC)-getSampleC_o(p);
	excess = getSampleC_e(p);
	rule = getC(posInC);	
	size_t l;
	acume=0;acump=sumL;
	getTotalExcessAndLength(rule,acume,acump);
	while(acump <= i){
		sumL =acump;
		excess +=acume;//getTotalExcess(rule);
		rule=getC(++posInC);
		if (sumL>0)assert(getExcessBuild(sumL-1)==excess);
		acume=0;
		getTotalExcessAndLength(rule,acume,acump);
	}
	p=0;
	if (e>=0 && excess+getMinExcess(rule)>e){
		eIni=-1;
		sumL--;
		return 0;
	}
	//move down in the rule hierarchy
	while(rule>=alph){

		l=dic->getLeft(rule);
		acume=0;acump=sumL;
		getTotalExcessAndLength(l,acume,acump);
		//~ size_t left = getLength(l);
		if (acump<=i){
			stackRulePath[p] = rule;
			stackRuleValues[p++]=sumL;
			sumL=acump;
			excess+=acume;//getTotalExcess(l);
			rule=dic->getRight(rule);
		}else{
			rule=dic->getLeft(rule);
		}
	}
	eIni=rule;
	assert(getExcessBuild(sumL)==excess+EXCESS[rule]);
	sumL--;
	assert(getExcessBuild(sumL)==excess);
	return 0;
}


/** Returns the excess at the begining of the rule. To get the excess at position 
 * i we have to add the excess generate by the terminal symbol pointed by rule*/
inline size_t GCT::findPos_bwd(size_t i, size_t &sumL, int &p, int &excess, size_t &rule, size_t &posInC){
	p = i >> blogBlockSizeC;
	posInC=getSampleC_p(p);
	sumL = (p<<blogBlockSizeC)-getSampleC_o(p);
	excess = getSampleC_e(p);
	rule = getC(posInC);	
	size_t l;
	while((l=sumL+getLength(rule)) <= i){
		sumL=l;
		excess +=getTotalExcess(rule);
		rule=getC(++posInC);
		assert(getExcessBuild(sumL-1)==excess);
	}
	p=0;
	//move down in the rule hierarchy
	while(rule>=alph){
		size_t t1;
		size_t left = getLength((l=dic->getLeft(rule)));
		if ((t1=sumL+left)<=i){
			stackRulePath[p] = rule;
			stackRuleValues[p++]=sumL;
			sumL=t1;
			excess+=getTotalExcess(l);//getZeroes(rules->get_left(rule-alph));
			rule=dic->getRight(rule);
		}else{
			rule=dic->getLeft(rule);
		}
	}
	return 0;
}



/**
 * if the excess e is >= 0, then we search backward for and excess equal to e
 * starting at position i (inclusive). 
 * In case the excess e<0, we search backward for an excess equal to 
 * E[i]+(e+1) starting at position i-1. For instance, to search backward for an excess e=E[i] (note
 * the search starts at i-1),  
 * we should call the function withe e=-1 (the searched excess would be E[i]+(e+1) = E[i] +(-1+2) = E[i].
 * To simulate the enclose operation (bwd_search(P,pi,i,2)), e must be -3. 
 * 
 * */
ulong GCT::bwd_search(uint i, int e){
	
//~ i;//you search starting in the next position
	size_t sumL,posInC,rule;
	int p,excess;

	size_t rulei;
	/*eIni the rule (<alph) at position i*/
	findPos_bwd_with_e(i,sumL,p,excess,rule,posInC,e,rulei);

	
	if (e<0){
		assert(sumL+1==i);
		//if (rulei==0) excess--;
		/*excess+EXCESS[rule] is the excess at position i*/
		// if (backBigger){
		// 	e=excess+EXCESS[rule]-(e+1);	
		// }else{
			e=excess+EXCESS[rule]+(e+1);	
			if (e==-1) return 0;//search backward for an excess ==-1 means findClose(close parentheses root)
			if (e==excess) return sumL;
		// }
	}else{
		if (rule<alph){
			if (excess+EXCESS[rule]==e) return i;	
			if (excess == e)return sumL;	
		}
	}	
	if (e<=0) return -1;
	assert(getExcessBuild(sumL)==excess);
	
	//~ assert(excess==inie);
		//search the rule back
		/** The rule rule does not contain the searched min --> i set the excess 
		 * to inie (the excess we have at the beggining of the rule*/
		//~ inie = excess;//nuevo
		//~ excess=inie;
	int mine,acume;size_t acump;
		
		while(p>0){
			p--;
			assert(getExcessBuild(sumL)==excess);
			rule =dic->getLeft(stackRulePath[p]);
			acume=0;mine=INT_MAX;acump=0;
			getMinTotalExcessAndLength(rule,mine,acume,acump);
			excess -= acume;//getTotalExcess(rule);	
			//down to the left
			if (excess+mine<=e){// && excess+getMaxExcess(rule)>=e){
				return searchRuleDown_bwd(rule,e,excess,stackRuleValues[p]-1);
			}else{
				sumL-=acump;//getLength(rule);
			}
		}
		//move a position forward in C
		size_t pos = sumL>>blogBlockSizeC;
		int lim = (pos<<blogBlockSizeC)-getSampleC_o(pos)-1;
		rule = getC(--posInC);
		//search the following rules up to the next sampling of C that is covered by a leaf of the RMM
		mine=INT_MAX;acume=0;acump=0;
		getMinTotalExcessAndLength(rule,mine,acume,acump);
		int t1;
		while((int)(t1=sumL-acump)>=lim){
			int totE = excess-acume;//getTotalExcess(rule);
			if (mine+totE > e){// || getMaxExcess(rule)+totE<e){
				sumL=t1;
				posInC--;
				excess = totE;
			}else{
				return searchRuleDown_bwd(rule,e,totE,sumL-getLength(rule));
			}
			rule = getC(posInC);
			mine=INT_MAX;acume=0;acump=0;
			getMinTotalExcessAndLength(rule,mine,acume,acump);
		}
		//search the RMM
		p=searchRMM_bwd(pos,1,excess,e);
	
	p++;
	posInC=getSampleC_p(p);

	rule=getC(posInC);
	sumL=(p<<blogBlockSizeC) - getSampleC_o(p)-1;
	excess = getSampleC_e(p);
	assert(getExcessBuild(sumL)==excess);
	
	while(excess+getMinExcess(rule)>e){
		rule=getC(--posInC);
		sumL-=getLength(rule);
		excess-=getTotalExcess(rule);
	}
	
	//search the rule down
	//en la función se le resta length(rule) a acum_pos para colocarla al principio pero aquí
	//ya tenemos el puntero apuntando al principio ->error
	return searchRuleDown_bwd(rule,e,excess,sumL);

}

/**
 * Searches backwards the RMM estructure
 * 
 * */
size_t GCT::searchRMM_bwd(size_t p, int index, int &excess, size_t e){
	
	while(index<heapDepth){
		if (p&0x1){
			size_t pminus1=p-1;
			if ( getSampleC_minE(pminus1+lengthsHeapLevels[index-1])<=e){// && sampleC_maxE[p+lengthsHeapLevels[index-1]]>=e){
					//TODO SHIFT IN SAMPLEC_E IMPLIES ADD +1
					//sampleC_e[i] is aligned with sampleC_minE[i-1]
					//CHECK commented while writing paper JSEA
					//excess=getSampleC_e(pminus1+lengthsHeapLevels[index-1]);
					index--;
					p--;
					break;
			}
		}
		p>>=1;
		index++;
	}
	//found the min, go Down searching for the rule with the min excess 
	while(index>0){
		//check left child
		index--;
		p<<=1;
		if (getSampleC_minE(p+1+lengthsHeapLevels[index])<=e){// && sampleC_maxE[p+1+lengthsHeapLevels[index]]>=e){
			p++;
		}//being down the left child implies do nothing (the excess does not vary).
	}
	//CHECK commented while writing paper JSEA
	//excess=getSampleC_e(p);
	return p;
}

/**
 * Given a rule, it returns the offset within it that contains its min. excess.
 * 
 * */
size_t GCT::findPosMin(size_t rule){
	uint offset=0;
	while(rule>alph){
		if (getMinExcess(dic->getLeft(rule))>getMinExcess(dic->getRight(rule))){
			offset+=getLength(dic->getLeft(rule));
			rule = dic->getRight(rule);
		}
	}
	if (EXCESS[rule]<0){
		return offset+1;
	}
	return offset;
}

size_t GCT::findPosMax(size_t rule){
	uint offset=0;
	while(rule>alph){
		if (getMinExcess(dic->getLeft(rule))<getMinExcess(dic->getRight(rule))){
			offset+=getLength(dic->getLeft(rule));
			rule = dic->getRight(rule);
		}
	}
	if (EXCESS[rule]<0){
		return offset+1;
	}
	return offset;
}

inline void GCT::updateLeaf(uint &leaf, uint &acumLeafs, uchar &prev, uchar fstb, uchar lstb){
	acumLeafs+=GET_OVERLAP(prev,fstb);
	prev=lstb;
	leaf=acumLeafs;
}

inline void GCT::updateLeaf_bwd(uint &leaf, uint &acumLeafs, uchar &prev, uchar fstb, uchar lstb){
	//we search backwards...the overlap is obtained from the first bit of the leftmost rule and the 
	//last or the rigthmost.
	acumLeafs-=GET_OVERLAP(lstb,prev);
	prev=fstb;
	leaf-=acumLeafs;
}

size_t GCT::getLeafFromPos(size_t pos, uint start, uint end){
	
	size_t acump,rule;
	uint acumLeafs;
	uchar fstb,prev,lstb;
	uint posInC,leaf;
	size_t sumL;

	size_t p = pos >>blogBlockSizeC;
	posInC   = getSampleC_p(p);
	sumL     = (p<<blogBlockSizeC)-getSampleC_o(p);
	leaf     = getSampleC_leaf(p);
	rule     = getC(posInC);	
	assert(bsLeafsTest==nullptr || leaf==bsLeafsTest->rank1(sumL));
	size_t l;
	acump=sumL;acumLeafs=leaf;
	prev=0;
	getRuleLengthAndLeaf(rule,acump,acumLeafs,fstb,lstb);
	while(acump <= pos){
		sumL =acump;
		updateLeaf(leaf,acumLeafs,prev,fstb,lstb);
		rule=getC(++posInC);		
		getRuleLengthAndLeaf(rule,acump,acumLeafs,fstb,lstb);
		assert(bsLeafsTest==nullptr || leaf==bsLeafsTest->rank1(sumL));
	}
	assert(bsLeafsTest==nullptr || leaf==bsLeafsTest->rank1(sumL));
	leaf+=GET_OVERLAP(prev,fstb);
	prev=0;
	//move down in the rule hierarchy
	while(rule>=alph){
		assert(bsLeafsTest==nullptr || leaf==bsLeafsTest->rank1(sumL));
		l=dic->getLeft(rule);
		acump=sumL;acumLeafs=leaf;
		getRuleLengthAndLeaf(l,acump,acumLeafs,fstb,lstb);
		if (acump<=pos){
			sumL=acump;
			updateLeaf(leaf,acumLeafs,prev,fstb,lstb);
			rule=dic->getRight(rule);
		}else{
			rule=l;
		}
	}
	leaf+=GET_OVERLAP(prev,rule);
	return leaf;
}

/**
 * Computes the position of the minimum excess with [i,j], and yields in 
 * minErmq the value of that excess. During the search, each time we find a new minium, 
 * we remember what that min is (minErmq), the rule that generates it (ruleMin), the excess 
 * up to the beggining of that rule (minAcumE), etc. This is just a lazy evaluation, that is, 
 * we do not search for the exact position until we know we have found the rule that contains 
 * the min.  
 * 
 * */
size_t GCT::rmqi(size_t i, size_t j, size_t &minErmq){
	if (i>=j) return -1;
	size_t sumL,posInC,rule;
	int p,excess;
	size_t posMin,ruleMin;

	findPos_fwd(i,sumL,p,excess,rule,posInC);
	int minAcumE=excess;
	minErmq=excess;	
	posMin=i;
	//ruleMin=-1 tells we know the exact position of the min. 
	ruleMin=-1;
	size_t l2;
	assert(getExcessBuild(sumL)==excess);
	
		//search the rule back
		while(p>0){
			assert(getExcessBuild(sumL)==excess);
			p--;
			//down to the left
			int acume=0,mine=INT_MAX;
			size_t acump=sumL;
			rule = dic->getRight(stackRulePath[p]);
			getMinTotalExcessAndLength(rule,mine,acume,acump);
			if (acump>j){
				if (excess+mine<minErmq){
					//search min inside
					//~ minErmq=t1;
					searchPartialRule_rmq(rule,j,excess,sumL,posMin,minErmq,ruleMin,minAcumE);
				}
				if (ruleMin!=-1)
					searchRuleDown_rmq(ruleMin,j,minAcumE,posMin,posMin,minErmq,ruleMin);
				return posMin;
			}else{
				if ((l2=excess+mine)<minErmq){
					minErmq=l2;
					ruleMin=rule;
					posMin=sumL;
					minAcumE=excess;
				}				
				excess+=acume;//getTotalExcess(rule);
				sumL=acump;
			}
		}
		size_t posplus1;
		size_t pos = (sumL>>(blogBlockSizeCPlus1));
		size_t lim = (((posplus1=pos+1))<<(blogBlockSizeCPlus1));//-getSampleC_o(pos);
		pos = (posplus1<<1);
		size_t lim2 = lim-getSampleC_o(pos)+getLength(getC(getSampleC_p(pos)));
		lim = min(lim2,j+1);
		rule = getC(++posInC);
		int acume=0,mine=INT_MAX;
		size_t acump=sumL;
		getMinTotalExcessAndLength(rule,mine,acume,acump);
		//search the following rules up to the next sampling of C that is covered by a leaf of the RMM
		while(acump<lim){
			assert(getExcessBuild(sumL)==excess);
			if ((l2=mine+excess) < minErmq){ // TODO
				ruleMin = rule;
				posMin = sumL;
				minAcumE=excess;
				minErmq = l2;
			}
			sumL=acump;
			posInC++;
			excess +=acume;// getTotalExcess(rule);	
			rule = getC(posInC);
			acume=0,mine=INT_MAX;acump=sumL;
			getMinTotalExcessAndLength(rule,mine,acume,acump);
		}
		if (acump>=j){
			if (excess+mine< minErmq) {
				size_t me=minErmq,rm;
				searchPartialRule_rmq(rule,j,excess,sumL,posMin,me,rm,minAcumE);//minErmq,ruleMin,minAcumE);
				if (me<minErmq) {
					minErmq = me;
					ruleMin = rm;
				}
			}
		}else {
			//SEARCH THE RMM-TREE
			if (sumL != j) {
				p = searchRMM_rmq(pos - 1, 0, excess, minAcumE, sumL, j, minErmq, posMin, ruleMin);
			}
		}
		
		if (ruleMin==-1) return posMin;
		searchRuleDown_rmq(ruleMin,j,minAcumE,posMin,posMin,minErmq,ruleMin);
		return posMin;
}

/**
 * Searches the RMM tree. 
 * 
 * */
size_t GCT::searchRMM_rmq(size_t p, int index, int &excess, int &minAcumE, size_t &acum_pos,size_t j, size_t &minE,size_t &posMin, size_t &ruleMin){

	size_t pMin=posMin;
	size_t prevm;
	size_t t1,t2,t3;
	int level=1,levelMin=0,inip=p;
	prevm=minE;
		
		p>>=1;
		acum_pos=(p+1)<<(blogBlockSizeC+1);
		inip=p;
		while(acum_pos<=j){
			if (!(inip&0x1)){
				if (((t2=(acum_pos+(blockSizeC<<level))))<j){
					if ((t1=getSampleC_minE(p+1+lengthsHeapLevels[level]))<minE){
						posMin=p+1;
						minE=t1;
						levelMin=level;
					}
				}else{
					break;//start search down
				}
				acum_pos=t2;
			}
			level++;
			p>>=1;
			inip>>=1;
		}
		p++;
		bool partial=true;
		while(level>0){
			int pshift=p<<1;
			int levelminus1=level-1;
			if (getSampleC_minE(p+lengthsHeapLevels[level])>=minE){
				 partial=false;
				 break;
			 }
			if (acum_pos+(blockSizeC<<level)>j){
				p=pshift;
				if ((t1=acum_pos+(blockSizeC<<levelminus1))<=j){
					if ((t2=getSampleC_minE(pshift+lengthsHeapLevels[levelminus1]))<minE){
						minE=t2;
						posMin=pshift;
						levelMin=levelminus1;
					}
					//we continue searching in the right branch
					acum_pos=t1;
					p++;
					
				}
			}else{
				t3 = pshift+lengthsHeapLevels[levelminus1];
				if ((t1=getSampleC_minE(t3))>(t2=getSampleC_minE(t3+1))){
					acum_pos+=((1<<(levelminus1))<<blogBlockSizeC);
					posMin=p+1;
					p=pshift+1;
					minE=t2;
				}else{
					posMin=p;
					minE=t1;
					p=pshift;
				}
				partial=false;
				levelMin=levelminus1;
				break;
			}
			level--;
		}
		
	if (partial && getSampleC_minE(p)<minE){
		size_t m=minE;
		iterate_fwd(getSampleC_p(p),(size_t)acum_pos-getSampleC_o(p)-1,getSampleC_e(p),j,minAcumE,m,ruleMin,posMin);
		if (m<minE){
			minE=m;
			return posMin;
		}				
	}else{					
		while(levelMin>0){
			int levelminus1=levelMin-1;
			size_t pshift=posMin<<1;
			if (getSampleC_minE(pshift+lengthsHeapLevels[levelminus1])!=minE){
				posMin=pshift+1;
			}else{
				posMin=pshift;
			}
			levelMin=levelminus1;
		}
	}	
	
	if (prevm>minE){
		p = posMin<<levelMin;
		int excess=getSampleC_e(p);
		acum_pos = (p<<blogBlockSizeC)-getSampleC_o(p)-1;
		searchRMM_rmq_linear(p,0,excess,minAcumE,acum_pos,j,minE,posMin,ruleMin);
		//TODO toqué esto cuando eliminé warnings
		return posMin;
	}else
		return pMin;

}


size_t GCT::searchRMM_rmq_linear(size_t p, int index, int &excess, 
		int &minAcumE, size_t &acum_pos,size_t j, size_t &minE,size_t &posMin, size_t &ruleMin){

	acum_pos = p<<blogBlockSizeC;
	
	size_t t1;
	posMin=p;
	//we know the min--> when t1==m break the loop.
	while((t1=getSampleC_minE(posMin))!=minE){
		posMin++;
	}	
	acum_pos = (posMin<<blogBlockSizeC)-getSampleC_o(posMin)-1;
	excess = getSampleC_e(posMin);
	size_t posInC=getSampleC_p(posMin);
	size_t rule = getC(posInC);
	int mine,acume;size_t acump;
	mine=INT_MAX;acume=0;acump=acum_pos;
	getMinTotalExcessAndLength(rule,mine,acume,acump);
	//
	while((t1=(mine+excess)) > minE){
		acum_pos=acump;
		excess += acume;//getTotalExcess(rule);	
		rule = getC(++posInC);
		mine=INT_MAX;acume=0;acump=acum_pos;
		getMinTotalExcessAndLength(rule,mine,acume,acump);
	}
	if (t1==minE){
		ruleMin = rule;
		posMin = acum_pos;
		minAcumE=excess;
		return posMin;
	}
	
	if (acum_pos<j){
		/**
		 * It could happen that we find a block with minE=a but that block 
		 * is covered by a rule with length > blockSizeC. Being e the excess
		 * at the current position, a situation like the following can happen:
		 *  getMinExcess(rule)+e < minE but j is before curren_position+getLength(rule).
		 * Thus, although we know that minE is what we are searching for, when we have to 
		 * carry out such a search in searchPartialRule_rmq, we only search for min_excess < minE
		 * (it is supposing we already have a valid position for that minE, but, in this case, 
		 * we don't). To force the algorithm to stop when we find minE, we have to increment it
		 * before calling the function. Thus, when it finds minE, it will get the position of that 
		 * min since what we are passing as a paramenter is actually minE+1.
		 * */
		if (t1!=minE) minE++;
		searchPartialRule_rmq(rule,j,excess,acum_pos,posMin,minE,ruleMin,minAcumE);
	}
	return posMin;
}




/**
	 * Computes the inorder of an INTERNAL node. Defined by Sadakane in 
	 * as rank_{()}(findclose(v+1))
	 * */
ulong GCT::inorder(uint v){
	/**
	 * findclose(v+1) is correct sinde v is internal
	 * and thus P[v+1]=1.
	 * */
	return brLeafRank(findClose((uint)v+1));
}

/**
* Computes the depth of a node. The root has length 0. It returns the
* excess at a given position -1.
*/
ulong GCT::depth(uint v){
	return (v+1-(rank0(v)<<1))-1;
}

/**
 * if the excess e is >= 0, then we search forward for and excess equal to e.
 * In case the excess is < 0, then we search for an excess equal to 
 * E[i]+(e+1). For instance, if we want to search for an excess 
 * excess(i) starting at position i+1, we have to call fwdExcess(i,-1).
 * **/
 ulong GCT::fwd_search(uint i, int e){
	//~ i;//you search starting in the next position
	size_t sumL,posInC,rule;
	int p,excess;
	size_t rulei;
	findPos_fwd_with_e(i,sumL,p,excess,rule,posInC,e,rulei);
	if (excess == e) return sumL;
	if (e<0){
		//if (rulei==0) return -1;
		/*note e<0*/
		assert(getExcessBuild(sumL)==excess);
		e=excess+(e+1);
	}
	

	if (e<0) return -1;
	assert(getExcessBuild(sumL)==excess);
	int acume,mine;size_t acump;
		//search the rule bottom-up
		while(p>0){
			assert(getExcessBuild(sumL)==excess);
			p--;
			//down to the left
			rule = dic->getRight(stackRulePath[p]);
			acume=0;mine=INT_MAX;acump=sumL;
			getMinTotalExcessAndLength(rule,mine,acume,acump);
			if (excess+mine<=e){// && excess+getMaxExcess(rule)>=e){
				return searchRuleDown_fwd(rule,e,excess,sumL);
				break;
			}else{
				excess+=acume;//getTotalExcess(rule);
				sumL=acump;//getLength(rule);
			}
			assert(getExcessBuild(sumL)==excess);
		}
		
		size_t pos = (sumL>>blogBlockSizeC)+1;
		int lim = (pos<<blogBlockSizeC)-getSampleC_o(pos);
		rule = getC(++posInC);
		acume=0;acump=sumL,mine=INT_MAX;
		getMinTotalExcessAndLength(rule,mine,acume,acump);
		//if (mine+excess<=e) return searchRuleDown_fwd(rule,e,excess,sumL);
		while(acump<lim){
			if (mine+excess > e){// || getMaxExcess(rule)+excess<e){
				sumL=acump;
				posInC++;
				excess += acume;//getTotalExcess(rule);
			}else{
				return searchRuleDown_fwd(rule,e,excess,sumL);
			}
			assert(getExcessBuild(sumL)==excess);
			rule = getC(posInC);			
			acume=0;acump=sumL,mine=INT_MAX;
			getMinTotalExcessAndLength(rule,mine,acume,acump);
		}
		//TODO esto no debería ser necesario. Falla con datos de prueba 
		//descritos en bugs.txt. El tema es que a veces buscamos en el RMM cuando, 
		//a pesar de que acump>=lim, ya tenemos los valores de mine calculados de tal forma 
		//que el resultado corresponde a esa regla de la que obtuvimos el mine --> no es necesario 
		//usar el RMM. Debe haber algún problema 
		//en searchRMM porque más adelante hay otro TODO que casca. 
		if (excess+mine<=e) return searchRuleDown_fwd(rule,e,excess,sumL);
		//search the RMM
		uint prePosInC=posInC;
		uint prevE=excess;
		p=searchRMM_fwd(pos-1,1,excess,e);
	
	/**Search the block returned by RMM*/
	posInC=getSampleC_p(p);
	
	//TODO: esto es una chapuza. Resulta que searchRMM_fwd a veces 
	//devuelve una posInC menor que en la que empezaba.... Si eliminamos
	//el primer if, dejando solo el else va a fallar (ver fichero bugs.txt 
	//para obtener datos de prueba).
	if (posInC<prePosInC){
		posInC = prePosInC;
		excess = prevE;
	}else{
		rule=getC(posInC);
		sumL=(p<<blogBlockSizeC) - getSampleC_o(p)-1;
	}
	acume=0;acump=sumL,mine=INT_MAX;
	getMinTotalExcessAndLength(rule,mine,acume,acump);
	while(excess+mine>e){
		sumL=acump;//getLength(rule);
		excess+=acume;//+=getTotalExcess(rule);
		rule=getC(++posInC);		
		assert(getExcessBuild(sumL)==excess);

		acume=0;acump=sumL,mine=INT_MAX;
		getMinTotalExcessAndLength(rule,mine,acume,acump);
	}
	
	//search the rule down
	return searchRuleDown_fwd(rule,e,excess,sumL);
}

 
/**
 * Search up and down the RMM hierarchy searching for the min excess e
 * Parameters: 	
 * 	index: level of the RMM where the search starts
 *  p: offset within the given level of the RMM that covers the initial 
 *  searching position
 * 	excess: accumulated excess until now
 * 	e: excess to search for
 * */
size_t GCT::searchRMM_fwd(size_t p, int index, int &excess, size_t e){
	
	while(index<heapDepth){
		if (!(p&0x1)){	
			size_t pplus1=p+1;
			if ( getSampleC_minE(pplus1+lengthsHeapLevels[index-1])<=e){
				index--;
				p++;
				break;
			}			
		}
		p>>=1;
		index++;
	}
	//found the min, go Down searching for the rule with the min excess 
	while(index>0){
		//check left child
		index--;
		p<<=1;
		if (getSampleC_minE(p+lengthsHeapLevels[index])>e){
			p++;
		}//being down the left child implies do nothing (the excess does not vary).
	}
	excess=getSampleC_e(p);
	return p;
}

size_t GCT::searchRuleDown_bwd(size_t rule, size_t e, int acum_excess, size_t acum_pos){
	while(rule>=alph){
			size_t t1,t2;
			size_t r = dic->getRight(rule);
			int minL = getMinExcess(r);
			
			t1=dic->getLeft(rule);
			int acume=0;size_t acump=0;
			getTotalExcessAndLength(t1,acume,acump);
						
			if ((t2=acum_excess+acume)+minL<=e){// && (acum_excess+totE)+maxL>=e){
				acum_excess= t2;
				acum_pos+=acump;//getLength(t1);
				rule=r;
			}else{
				rule=t1;
			}
		}
		assert(rule>=0 && rule<alph);
		acum_excess+=EXCESS[rule];//getTotalExcessRankTerminals(rule,EXCESS,(int)getLenTerms(rule+1));
		//~ acum_pos+=(getLenTerms(rule+1)-getLenTerms(rule));
		acum_pos++;
		assert(getExcessBuild(acum_pos)==acum_excess);
		if (acum_excess!=e){
			acum_pos--;//=searchExcessInTerm_bwd((uint)rule,(int)getLenTerms(rule+1),acum_excess,e,getLenTerms(rule+1)-getLenTerms(rule+1)-1);
			assert(acum_excess+EXCESS[rule]==e);
		}
		assert(acum_excess==e);
		return acum_pos;
}

size_t GCT::iterate_fwd(int posInC, size_t acum_pos, int excess, size_t lim, int &minAcumE, size_t &minE, size_t &ruleMin, size_t &posMin){
	
	size_t rule = getC(posInC);
	size_t t1;
	int mine=INT_MAX,acume=0;size_t acump=acum_pos;
	getMinTotalExcessAndLength(rule,mine,acume,acump);
	
	while(acump<=lim){
			if ((t1=(mine+excess)) < minE){ // TODO
				ruleMin = rule;
				posMin = acum_pos;
				minE = t1;
				minAcumE=excess;
				//~ break;
			}
			posInC++;
			acum_pos=acump;
			excess += acume;//getTotalExcess(rule);	
			rule = getC(posInC);
			
		mine=INT_MAX,acume=0;acump=acum_pos;
		getMinTotalExcessAndLength(rule,mine,acume,acump);
	}
	if (acum_pos<lim){
		searchPartialRule_rmq(rule,lim,excess,acum_pos,posMin,minE,ruleMin,minAcumE);
	}
	return 0;
}

void GCT::iterate_EQUAL_fwd(int posInC, size_t acum_pos, int excess, size_t lim, int &minAcumE, size_t &minE, size_t &ruleMin, size_t &posMin){
	size_t rule = getC(posInC);
	size_t l,t1;
	while(acum_pos+(l=getLength(rule))<=lim){
			if ((t1=(getMinExcess(rule)+excess)) == minE){ // TODO
				ruleMin = rule;
				posMin = acum_pos;
				minE = t1;
				minAcumE=excess;
				return ;
			}
			posInC++;
			acum_pos+=l;
			excess += getTotalExcess(rule);	
			rule = getC(posInC);
	}
	//TODO 
	if (acum_pos<lim){
		searchPartialRule_rmq(rule,lim,excess,acum_pos,posMin,minE,ruleMin,minAcumE);
	}
	
	//check if the beggining of the next rule contains the minimum
}


size_t GCT::searchRuleDown_fwd(size_t rule, size_t e, int acum_excess, size_t acum_pos){
		while(rule>=alph){
			size_t l;
			int left = getMinExcess((l=dic->getLeft(rule)));
			//~ int maxL = getMaxExcess(dic->getLeft(rule));
			size_t r = l;
			
			if (acum_excess+left>e){//|| acum_excess+maxL<e){
				acum_excess= acum_excess + getTotalExcess(r);
				acum_pos+=getLength(r);
				rule=dic->getRight(rule);
			}else{
				rule=r;
			}
		}
		assert(rule>=0 && rule<alph);
		/**With trimmed rules*/
		//~ acum_pos+=searchExcessInTerm((uint)rule,(int)getLenTerms(rule),acum_excess,e)+1;
		if (acum_excess+EXCESS[rule]==e) acum_pos++;	
		return acum_pos;
}

/**
 * YOU KNOW THE MIN IS INSIDE THIS RULE...YOU NEED TO FIND THAT MIN WITHIN THE INTERVAL [0,j]
 * 
 * */
size_t GCT::searchRuleDown_rmq(size_t rule, size_t j, int acum_excess, size_t acum_pos, size_t &minPos, size_t &minE, size_t &minRule){
		//we are searching for the position of minE and we know it is inside the given rule. In 
		//order to search that postion, we have to increment minE, otherwise we 
		//never enter in the searched rule because acum_excess+minE(rule)>=minE.
		//~ minE++;

		while(rule>=alph){
			size_t r=dic->getLeft(rule);
			int mine=INT_MAX,acume=0;
			size_t acump=acum_pos;
			getMinTotalExcessAndLength(r,mine,acume,acump);
			if (acum_excess+mine==minE){
				rule=r;
				//~ minRule=r;
			}else{
				acum_excess= acum_excess + acume;//getTotalExcess(r);
				acum_pos=acump;//+=getLength(r);
				rule=dic->getRight(rule);
			}		
		}
		assert(rule>=0 && rule<alph);
		//~ int pos=0,mine=minE+1;
		//~ //pos is the offset withing the rule
		//~ getMinExcessTerminal(rule,mine,acum_excess,pos,getLenTerms(rule+1));
		//~ if (mine<minE){
			
			//~ acum_pos+=pos+1;
			minPos=acum_pos+1;
			//~ minE=mine;
		//~ }
		return minPos;
		
}

size_t GCT::searchPartialRule_rmq(size_t rule, size_t j, int acum_excess, size_t acum_pos, size_t &minPos, size_t &minE, size_t &minRule, int &minAcumE){

		size_t t1;
		while(rule>=alph){
			if (acum_excess+getMinExcess(rule)>=minE) return -1;
			size_t r=dic->getLeft(rule);
			
			int mine=INT_MAX,acume=0;size_t acump=acum_pos;
			getMinTotalExcessAndLength(r,mine,acume,acump);
			
			if (acump< j){
				if ((t1=acum_excess+mine)<minE){
					minE = t1;
					minPos = acum_pos;
					minRule=r;
					minAcumE=acum_excess;
					//~ break;
				}
				acum_excess= acum_excess + acume;// getTotalExcess(r);
				acum_pos=acump;
				rule=dic->getRight(rule);
			}else{
				
				rule=r;
			}
		}
		int pos=0,mine=minE;
		
		//alph>2
		//getMinExcessTerminal(rule,mine,acum_excess,pos,getLenTerms(rule)+j-acum_pos);
		if (EXCESS[rule]==-1){
			mine=minE-1;
		}
		if (mine<minE){
			acum_pos+=pos+1;
			minPos=acum_pos;
			minE=mine;
			minAcumE=acum_excess;
			minRule=-1;
		}
		return minPos;
}


void GCT::sampleLeafs(uint *INVP,uint *input){
	
	OVERLAP_LEAFS[0]=0;//00 -> 0 leaf
	OVERLAP_LEAFS[1]=0;//01 -> 0 leaf
	OVERLAP_LEAFS[2]=1;//10 -> 1 leaf
	OVERLAP_LEAFS[3]=0;//11 -> 0 leaf
	
	ruleLeafs = new uint[n];
	ruleFstBit = new uint[uint_len(n-alph,1)];
	ruleLstBit = new uint[uint_len(n-alph,1)];
	for (uint i=0;i<uint_len(n-alph,1);i++){
		ruleFstBit[i]=ruleLstBit[i]=0;
	}
	for (int i=0;i<n;i++)ruleLeafs[i]=-1;
	ruleLeafs[0]=ruleLeafs[1]=0;
	size_t ini,lim;
	ini=lim=0;
	uint clim=c;
	for (int i=0;i<clim;i++){
		
		size_t rule = getC(i);	
		
		lim=ini+lengths[INVP[rule]];
		
		if (rule<alph){
			ini=lim;
			continue;
		}
		//rule already processed --> skip 
		if (ruleLeafs[rule]!=-1){
			ini=lim;
			continue;		}else{
			ruleLeafs[rule]=0;
		}		
		if (bitget(input,ini) && rule>=alph)bit_set(ruleFstBit, rule - alph);
		if (bitget(input,lim-1) && rule>=alph)bit_set(ruleLstBit, rule - alph);
		ruleLeafs[rule]=bsLeafsTest->rank1(lim-1)-bsLeafsTest->rank1(ini);
			//~ for (;ini<lim;ini++){
				//~ ruleLeafs[rule] += (input[ini] && !input[ini+1])?1:0; 
			//~ }
		
		ini=lim;
		
	}
	assert(lim==len);
	for (int i=alph;i<n;i++){
		if (ruleLeafs[i]==-1){
			uint leafs=0;
			bool fb,lb;
			fb=lb=false;
			getLeafsRec(i,alph,ruleLeafs,ruleFstBit,ruleLstBit,fb,lb,leafs);
			if (fb) bit_set(ruleFstBit, i - alph);
			if (lb) bit_set(ruleLstBit, i - alph);
		}
	}
	
}


//sampleC at regular intervals in the length of the original sequence
 void GCT::sampleC(uint *INVP,uint *input) {
		//~ int bs = 16;//this should be a parameter
		
        int i, sumL,sumZ, sumMinE,sumMaxE,totalE,ch;
        sumL = sumZ = sumMinE = sumMaxE = totalE = 0;
        
        nSamples = floor((len - 1) >> blogBlockSizeC)+2;
        nSamplesC =   nSamples+1;
        heapLength= 2*(nSamplesC)+10;
        
        samplesLen = new size_t[nSamplesC];
        sampleC_o  = new size_t[nSamplesC];
        sampleC_p  = new size_t[nSamplesC];
        sampleC_leafs = new uint[nSamplesC];
        
        samplesMinE    = new int[nSamplesC];
       
        sampleC_e = new size_t[heapLength];//heap
        size_t *sampleC_e_tmp = new size_t[heapLength];//heap
        sampleC_minE = new int[heapLength];
        
		#ifdef VERBOSE
			cerr << "nSamplesC: " << nSamplesC << endl;
        #endif
        uint hl = (uint)ceil((double)log(nSamples)/log(2));
        lengthsHeapLevels = new uint[hl+2];
        /**
         * We use, at least during construction, two vectors to store sampleC_e. 
         * The total excess for each sample is stored after substracting 
         * the last totalExcess added (sampleC_e [j] = totalE - totalExcess[getC(i-1)];). 
         * This is perfectly valid to carry out rank but for the RMM tree we need to store
         * sampleC_e [j] = totalE; instead. Actually, we need to store that samplings 
         * without substraction totalExcess[...] only for the internal nodes of the RMM 
         * tree because the "leafs" are processed by a different part of the algorithm for
         * fwdExcess and bwd_search. 
         * */
        // A = new int[size];
        // It is easier to set 0's  at position 0 of each vector in order 
        // to carry out binary searches easily
        for (uint i=0;i<=nSamples;i++){
			samplesLen[i]=0;
			samplesMinE[i]=0;
			sampleC_p[i]=0;
			sampleC_o[i]=0;
			sampleC_leafs[i]=0;
		}
		
		sampleC_e[0]=sampleC_e_tmp[0]=0;
		sampleC_minE[0]=0;
		int minE = totalExcess[getC(0)];
		
		size_t j=1;

		size_t rule = 0;
		sumL=0;ch=0;
		i=0;j=1;
		while(i<len){	
			if (i<j*blockSizeC){
				totalE+=EXCESS[bitget(input,i)];
				minE=min(minE,totalE);
				i++;
			}else if (i==j*blockSizeC){
				int lim = (int) j*blockSizeC;
				while(sumL<=lim){
					rule=getC(ch++);
					/**the vector lengths is sorted according to the initial labeling
					 *of symbols (befor trimming and building the dictionary - what induces a permutation and a renaming of the symbols-*/
					sumL+=lengths[INVP[rule]];
				}
				sumL = sumL-lengths[INVP[rule]];
				ch--;
				
				/*Note that minE is not synchronized with totE*/
				sampleC_o [j] = j*this->blockSizeC-sumL;
				sampleC_p [j] = ch;
				sampleC_e [j] = getExcessBuild(sumL-1);
				//~ cout << "sumL: " << sumL << endl;
				if(totalE!=getExcessBuild(i-1))
					cout << "totE!=getExcessBuild"<<endl;
				assert(totalE==getExcessBuild(i-1));
				sampleC_minE [j] = minE;
				assert(minE>=0);
				minE=INT_MAX;
				
					//~ cout << "(j,p,o,e,mine): " << j << ","<<  sampleC_p [j] << "," << sampleC_o [j] << "," << sampleC_e [j] << "," << sampleC_minE[j]<< endl;
					j++;
			}
			
		}
		if (i%blockSizeC){
			sampleC_p[j] = sampleC_p[j-1];
			sampleC_o[j] = (j*blockSizeC) - ((j-1)*blockSizeC - sampleC_o[j-1]);
			sampleC_minE[j] = minE;
			assert(minE>=0);
			sampleC_e[j] = sampleC_e[j-1];
			j++;
		}
		len--;
		
		/**
		 * Notes about the limits: 
		 * example: 
		 * 010101001 | 0011010 . The "|" marks the limit between that two blocks. 
		 * If a block finish with an 1 and the next (which would belong to the next sample) starts with 
		 * 0, we count that leaf in the current block so that, once the block is found, we do not have to 
		 * check if the previous finishes with one and the next starts with one. The first time
		 * this is already computed. Once we start to iterate inside the block we need that info. 
		 * */
		size_t totalLeafs=0;
		
		for (int i=1;i<j;i++){
			size_t pos = i*blockSizeC-sampleC_o[i];
			
			sampleC_leafs[i]= bsLeafsTest->rank1(pos);
			totalLeafs+=sampleC_leafs[i];
		}
		posLastLeaf = bsLeafsTest->select1(bsLeafsTest->rank1(len-1));
		assert(j<nSamplesC);
		sampleC_leafs[j] = bsLeafsTest->rank1(bsLeafsTest->getLength()-1)+1;
		//~ sampleC_leafs[nSamples]=bsLeafsTest->rank1((j-1)*blockSizeC - sampleC_o[j-1]);
		
		len++;
		#if 0
        for (int i=0;i<j;i++){
			cout << "totlaE_tmp[" << i << "]: " << sampleC_e[i] << endl;
		}
		
        for (int i=0;i<j;i++){
			cout << "totlaE_[" << i << "]: " << sampleC_e[i] << endl;
		}
		#endif
        //build heap on sampleC_e and sampleC_minE
		sampleC_e++;
		sampleC_e_tmp++;
		sampleC_minE++;
		
		j--;
        size_t pos    = 0;
        size_t nelem  = j;
        size_t offset = j;
        lengthsHeapLevels[0]=0;
		heapDepth  = 1;
		//~ heapLevelSize = new uint[(uint)(ceil(log(nelem)/log(2))+2);
		//~ for (int i=0;i<(uint)(ceil(log(nelem)/log(2))+2;i++)heapLevelSize[i]=0;
        while(nelem/2>0){
			lengthsHeapLevels[heapDepth++]=offset;
			for (size_t i=0;i<nelem/2;i++){
				sampleC_minE[offset]=min(sampleC_minE[pos+2*i],sampleC_minE[pos+2*i+1]);
				sampleC_e[offset] = sampleC_e[pos+2*i+1];
				offset++;
			}
			size_t newnelem=nelem;
			if (nelem%2!=0){
				sampleC_minE[offset]=sampleC_minE[pos+nelem-1];
				sampleC_e[offset]=sampleC_e[pos+nelem-1];
				offset++;nelem/=2;nelem++;
			}else{
				nelem/=2;
			}
			pos+=newnelem;
			
		}
		
		heapLength = offset;
		sampleC_e--;
		sampleC_e_tmp--;
		sampleC_minE--;
		//~ sampleC_maxE--;
		j++;
		for (size_t i=0;i<heapLength;i++){
			sampleC_minE[i]=sampleC_minE[i+1];
			//~ sampleC_maxE[i]=sampleC_maxE[i+1];
		}
		//~ for (int i=0;i<heapLength;i++)
			//~ assert(sampleC_minE[i]>=0);
		#if 0
		for (size_t i=0;i<2*nSamples-1;i++){
			assert(sampleC_minE[i]>=0);
			cout << "e[" << i << "]: " << sampleC_e[i] <<", mine[" << i << "]: " << sampleC_minE[i] <<", maxe[" << i << "]: " << sampleC_maxE[i]<< endl;
		}
		#endif
		//~ delete [] sampleC_e_tmp;
        //patch for fwdExcess
        
    }

 
void GCT::sampleCExcess() {
		//~ int bs = 16;//this should be a parameter
		
        int i, sumL,sumZ, sumMinE,sumMaxE,totalE;
        sumL = sumZ = sumMinE = sumMaxE = totalE = 0;
        
        nSamples = floor((len - 1) / this->sampleRateCE)+1;
        //~ cout << "len: " << len << ", nsamples:  " << nSamples << endl;
        samplesMinE    = new int[nSamples];
        
        // A = new int[size];
        // It is easier to set 0's  at position 0 of each vector in order 
        // to carry out binary searches easily
		samplesZeroes[0]=0;
		samplesLen[0]=0;
		samplesMinE[0]=0;
		samplesMaxE[0]=0;
		
		sampleC_p[0]=0;
		sampleC_o[0]=0;
		sampleC_e[0]=0;
		size_t j=1;
        for (i = 1; i < c; i++) {
			sumL += lengths[getC(i - 1)];
			sumZ += zeroes[getC(i-1)];
			sumMinE += minExcess[getC(i-1)];
			sumMaxE += maxExcess[getC(i-1)];
			totalE += totalExcess[getC(i-1)];
            //write sample
			if (sumL+getLength(getC(i))>j*blockSizeC){
				sampleC_p [j] = i-1;
				sampleC_o [j] = j*this->blockSizeC-(sumL-lengths[getC(i-1)]);
				sampleC_o [j] = j*this->blockSizeC-(sumL-lengths[getC(i-1)]);
				sampleC_e [j] = totalE - totalExcess[getC(i-1)];
				assert(sampleC_e [j]<20);
				cout << "(o,p,e): " << sampleC_o [j] << "," << sampleC_p [j] << "," << sampleC_e [j] << endl;
				j++;
			}
			//~ i+=lengths[getC(i-1)];
        }
      
    }
    
void GCT::computeExcessRuleTest(int rule, int alpha,int &e, int *EXCESS){
		if (rule>=alph){
			int left,right;
			
			left =  dic->getLeft(rule);
			//~ if (left<0) left+=len;
			right = dic->getRight(rule);
			//~ if (right<0) right+=len;
			if (left>=alph){
				computeExcessRuleTest(left,alpha,e,EXCESS);
			}else{
				//~ cout << left;
				e+=EXCESS[left];
			}
			if (right>=alph){
				computeExcessRuleTest(right,alpha,e,EXCESS);
			}else{
				e+=EXCESS[right];
				//~ if (left) e++;else e--;
			}
		}else{
			//~ if (rule) e++;else e--;
			e+=EXCESS[rule];
		}
	
}

void GCT::printRule(int rule, int alpha){
		if (rule>=alph){
			int left,right;			
			left =  dic->getLeft(rule);
			right = dic->getRight(rule);
			if (left>=alph){
				printRule(left,alpha);
			}else{
				cout << left;
			}
			if (right>=alph){
				printRule(right,alpha);
			}else{
				cout << right;
			}
		}else{
			cout << rule;
		}
}


/****/
void GCT::getLeafsRec(int rule, int alpha, uint *rule_leafs, uint *first_bit, uint *last_bit, bool &fstb, bool &lstb, uint &leafs){
		
		assert(alph==2);
		if (rule_leafs[rule]!=-1){
			if (rule>=alph){
				fstb=bitget(first_bit,rule-alph);
				lstb=bitget(last_bit,rule-alph);
			}else{
				fstb=lstb=rule;
			}
		}else{
			if (rule>=alph){
				int left,right;			
				left =  dic->getLeft(rule);
				right = dic->getRight(rule);
				
				getLeafsRec(left,alpha,rule_leafs,first_bit,last_bit,fstb,lstb,leafs);
				rule_leafs[rule]=rule_leafs[left];
				if (fstb) bit_set(first_bit, rule - alph);

				bool prev=lstb;
				bool fstb1;
				getLeafsRec(right,alpha,rule_leafs,first_bit,last_bit,fstb1,lstb,leafs);
				rule_leafs[rule]+=rule_leafs[right];
				if (lstb) bit_set(last_bit, rule - alph);
				
				rule_leafs[rule]+=GET_OVERLAP(prev,fstb1);
				
			}else{
				//~ if (rule){
					//~ bit_set(first_bit,rule);
					//~ bit_set(last_bit,rule);
				//~ }
				assert(false);
				rule_leafs[rule]=0;
				fstb=lstb=rule;
				//~ leafs=rule_leafs[rule];
			}
		}
}


void GCT::countOccurrences(int rule, uint *original){
	
		if (rule>=alph){
			uint left,right;
			left =  RULES_LEFT[rule-alph];
			right = RULES_RIGHT[rule-alph];
			
			ruleOccs[left]++;
			ruleOccs[right]++;
			
			if (left>=alph){
				countOccurrences(left,original);
			}else{
				assert(term<len && left==original[term]);
				term++;
				#ifdef PRINT
				cout << left;
				#endif
				
			}
			if (right>=alph){
				countOccurrences(right,original);
			}else{
				assert(term<len && right==original[term]);
				#ifdef PRINT
				cout << right;
				#endif
				term++;
			}
		}else{
			#ifdef PRINT
			cout << rule;
			#endif
			assert(rule==original[term]);
			ruleOccs[rule]++;
			term++;
		}
		
}



    GCT::GCT():Parentheses(0){}


    
    void GCT::computeExcess(size_t rule, size_t subrule, int &minE, int &maxE, int &totalE, int &excess, int *tote, int *mine, uint *depthRules, uint &drule){
		if (subrule>=alph){
			size_t left,right;	
			left =  dic->getLeft(subrule);
			right = dic->getRight(subrule);
			uint dl=drule+1,dr=drule+1;
			/** Process left rule*/
			if (left>=alph){
				if (tote[left]==INT_MAX){
					int e=0,me=INT_MAX,te=0;
					computeExcess(rule,left,me,maxE,te,e,tote,mine,depthRules,dl);
					tote[left]=te;
					mine[left]=me;
				}
				dl = depthRules[left-alph];
				totalE=excess+tote[left];
				minE=min(minE,excess+mine[left]);
				excess=totalE;
			}else{
				dl+=1;
				minE=min(minE,excess+mine[left]);
				excess+=EXCESS[left];
				totalE+=EXCESS[left];
				
				
			}
			/** Process right rule*/
			if (right>=alph){
				if (tote[right]==INT_MAX){
					int e=0,me=INT_MAX,te=0;
					computeExcess(rule,right,me,maxE,te,e,tote,mine,depthRules,dr);
					tote[right]=te;
					mine[right]=me;
				}
				dl= depthRules[right-alph];
				totalE=excess+tote[right];
				minE=min(minE,excess+mine[right]);
				excess=totalE;
			}else{
				dr+=1;
				minE=min(minE,excess+mine[right]);
				excess+=EXCESS[right];
				totalE+=EXCESS[right];
			}
			//~ depthRules[rule-alph]=max(dl,dr);
		}else{
				minE=min(minE,excess+mine[subrule]);
				excess+=EXCESS[subrule];
				totalE+=EXCESS[subrule];
				
		}
		if (rule>=alph){
			tote[rule]=excess;
			mine[rule]=minE;
		}
	}
    
    void GCT::countOnes(size_t rule, size_t subrule, size_t &ones){
		if (subrule>=alph){
			size_t left,right;	
			left =  RULES_LEFT[subrule-alph];
			right = RULES_RIGHT[subrule-alph];
			if (left>=alph){
				countOnes(rule,left,ones);
			}else{
				ones+=left;
			}
			if (right>=alph){
				countOnes(rule,right,ones);
			}else{
				ones+=right;
			}
		}else{
			ones+=subrule;
		}
	}
    
    int GCT::recursiveDelete(int rule, size_t index, HashMap *deleted, HashMap *trimed, uint &ndel, uint *rules_length){
		if (rule>=alph){
			rule-=alph;
			uint lr[2]; // lr stands for left/right
			lr[0] = RULES_LEFT[rule];
			lr[1] = RULES_RIGHT[rule];
			for (size_t i=0;i<2;i++){
				if (lr[i]>=alph){
					lr[i]-=alph;
					
					//the rule has already been trimmed but is below a bigger one
					//that can also be trimed -> add to deleted set						
					if (trimed->get(lr[i])!=-1){
						trimed->erase(lr[i]);
						deleted->put(lr[i]);
						ndel++;
						bitVectorsTrim[lr[i]].second = true;
					}
					
					//everything that is above a trimmed rule is added to the 
					//set of deleted rules
					if (deleted->get(lr[i])==-1){
						deleted->put(lr[i]);
						ndel++;
					}
					
					//if the bitvector assoc with a rule is not empty it is because it 
					//has already been processed (is in delete or trimmed set)
					if (bitVectorsTrim[lr[i]].first==NULL){
						bitVectorsTrim[lr[i]].first  = new uint[uint_len(rules_length[lr[i]],1)];
						for (size_t j=0;j<uint_len(rules_length[lr[i]],1);j++)
							bitVectorsTrim[lr[i]].first[j]=0;
						bitVectorsTrim[lr[i]].second = false;
						recursiveDelete(lr[i],0,deleted,trimed,ndel,rules_length);
					}
					//add the bits generateb by lr[i] children to the index bitmap
					for (size_t j=0;j<rules_length[lr[i]];j++){
						if (bitget(bitVectorsTrim[lr[i]].first,j))
							bit_set(bitVectorsTrim[rule].first, index);
						index++;
					}
					bitVectorsTrim[rule].second = true;
				}else{
					//OJO: I'm suppossing rule 0 generates a 0 and rule 1->1
					if (lr[i]){
						bit_set(bitVectorsTrim[rule].first, index);
					}
					index++;
				}					
			}
			assert(index==rules_length[rule]);
		}
		return 0;
	}
    
int GCT::my_trim_rule(uint *&symbPerm, uint &newn, uint *rule_lengths, uint *newC) {
		
        HashMap *deleted = new HashMap();
        HashMap *trimed = new HashMap();
        
        size_t trim = 1;//no trim is carried out
        
        trim_rules = 0;
		zeroes = new uint[n];
		lengths = new uint[n];
		bitVectorsTrim = new pair<uint*,bool>[n];
		newn = n;
		uint ntrim=0;
		if (lengths==NULL){
			cerr << "LENGTHS is null" << endl;
			exit(0);
		}
		for (size_t i=0;i<n;i++){
			lengths[i]=0;
			zeroes[i]=0;
			bitVectorsTrim[i].first=NULL;
			bitVectorsTrim[i].second=false;
		}
		//Initialize the length of each rule
		lengths[0]=lengths[1]=1;//the terminal symbols
		//the non-terminal...
		for (size_t i=0;i<n-alph;i++){
			lengths[i+alph] = rule_lengths[i];
		}
		//Get the number of zeroes generated by each rule
		zeroes[0]=1;zeroes[1]=0;//0->0; 1->1;
		for (size_t i=0;i<n-alph;i++){
			size_t ones=0;
			countOnes(i+alph,i+alph,ones);
			zeroes[i+alph]=lengths[i+alph]-ones; // #0 from #1
		}	
		for (size_t i=0;i<n-alph;i++){
			assert(lengths[i+alph]==lengths[RULES_RIGHT[i]]+lengths[RULES_LEFT[i]]);
		}	
		for (size_t i=0;i<n-alph;i++){
			assert(zeroes[i+alph]==zeroes[RULES_RIGHT[i]]+zeroes[RULES_LEFT[i]]);
		}
			#if PRINT
				cout << "Printing the first min(n,20) rules" << endl;
				for (size_t i=0;i<min(n,20);i++){
					cout << "Rule-"<<i<<", length=" << lengths[i] << ", zeroes=" << zeroes[i] << endl;
				}
			#endif
		
		uint ndel=0;
        for (size_t i = 0; i < n - alph; i++) {
            //add rules < trim
            if (rule_lengths[i] <= trim) {
				//if a rule has already been deleted (is above a trimmed rule) we continue
				if (deleted->get(i)!=-1) continue;
				//rule already trimmed
				if (trimed->get(i)!=-1) continue;
				//otherwise->trim the rule deleting everything that is below
				trimed->put(i);
				ntrim++;
				if (bitVectorsTrim[i].first==NULL){
					size_t cover      = rule_lengths[i];
					size_t uintscover = uint_len(1,cover);
					bitVectorsTrim[i].first = new uint[uintscover];					
					for (size_t j=0;j<uintscover;j++) bitVectorsTrim[i].first[j]=0;						
					recursiveDelete(i+alph,0,deleted,trimed,ndel,rule_lengths);
				}
				#if VERBOSE
				cout << "rule trimmed : " << i+alph << endl;
				for (size_t j=0;j<rule_lengths[i];j++){
					if (bitget(bitVectorsTrim[i].first,j)){
						cout << "1";
					}else{
						cout << "0";
					}
				}
				cout << endl;
                #endif
            }
        }
        //Rules in C cannot be deleted
      
       for (size_t i=0;i<c;i++){
		   
			if (newC[i]>=alph && deleted->get(newC[i]-alph)!=-1){
				#if VERBOSE
					cout << newC[i] << "-deleted but it shold not be deleted (it is in C)" << endl;
					for (size_t j=0;j<rule_lengths[newC[i]];j++){
						if (bitget(bitVectorsTrim[newC[i]].first,j)){
							cout << "1";
						}else{
							cout << "0";
						}
					}
					cout << endl;
				#endif
				//we should remove the rule from this set and add it as a verbatim one (maybe adding it to 
				//the trimmed set). 
				deleted->erase(newC[i]-alph);
				trimed->put(newC[i]-alph);
				ndel--;
			}
		}
        #if VERBOSE
		for (size_t i = 0; i < n - alph; i++) {
			if (deleted->get(i)!=-1){
				cout << i+alph << "-deleted" << endl;
			}
		}
		
		for (size_t i = 0; i < n - alph; i++) {
			if (trimed->get(i)!=-1){
				cout << i+alph << "-trimed" << endl;
			}
		}
		#endif
		for (size_t i=0;i<n-alph;i++){
			
			if (RULES_LEFT[i]>=alph && (deleted->get(RULES_LEFT[i]-alph)!=-1)){
				deleted->erase(RULES_LEFT[i]-alph);
				trimed->put(RULES_LEFT[i]-alph);
				ndel--;
				int rule = RULES_LEFT[i]-alph;
				if (bitVectorsTrim[rule].first==NULL){
					size_t cover      = rule_lengths[rule+alph];
					size_t uintscover = uint_len(1,cover);
					bitVectorsTrim[i].first = new uint[uintscover];					
					for (size_t j=0;j<uintscover;j++) bitVectorsTrim[rule].first[j]=0;						
					recursiveDelete(rule+alph,0,deleted,trimed,ndel,rule_lengths);
				}
			}
			if (RULES_RIGHT[i]>=alph && (deleted->get(RULES_RIGHT[i]-alph)!=-1)){
				deleted->erase(RULES_RIGHT[i]-alph);
				trimed->put(RULES_RIGHT[i]-alph);
				ndel--;
				
				int rule = RULES_RIGHT[i]-alph;
				if (bitVectorsTrim[rule].first==NULL){
					size_t cover      = rule_lengths[rule+alph];
					size_t uintscover = uint_len(1,cover);
					bitVectorsTrim[i].first = new uint[uintscover];					
					for (size_t j=0;j<uintscover;j++) bitVectorsTrim[rule].first[j]=0;						
					recursiveDelete(rule+alph,0,deleted,trimed,ndel,rule_lengths);
				}
			}
		}
		//cerr << "ndel: " << ndel << endl;
		
		symbPerm = new uint[n];
		for (size_t i=0;i<n;i++) symbPerm[i]=-1;
		size_t new_alph = buildVerbatimBitVector(trimed,symbPerm,rule_lengths);
		for (int i=0;i<n-alph;i++){
			if (trimed->get(i)!=-1){
				assert(symbPerm[i+alph]<(alph+ntrim));
			}
		}
		#if VERBOSE
			for (size_t i=0;i<n;i++){
				if (symbPerm[i]!=-1)
					cout << "symbPerm[" << i << "]: " << symbPerm[i] << endl;
			}
		#endif
		uint index=new_alph;
		//~ cout << "new_alph: " << new_alph << endl;
		//we assign new ids to the non-trimmed rules
		//cerr << "nntrime: " << ntrim << endl;
		
		for (size_t i=0;i<n-alph;i++){
			if (deleted->get(i)!=-1){
				 continue;
			 }
			if (trimed->get(i)==-1){
				if (symbPerm[i+alph]==-1){
					//cout << index << endl;
					symbPerm[i+alph]=index++;
				} 
			}
		}

		//rename the rhs of each rule
		//RULES_LEFT AND RULES_RIGHT store the left/right sibbling of each non-terminal rule
		uint *NEW_RULES_LEFT  = new uint[n-new_alph];
		uint *NEW_RULES_RIGHT = new uint[n-new_alph];
		index=0;
		for (size_t i=0;i<n-alph;i++){
			if (deleted->get(i)!=-1) continue;
			if (trimed->get(i)==-1){
				//~ cout << "index: " << index << endl;
				
				assert(symbPerm[RULES_LEFT[i]]!=-1);
				assert(symbPerm[RULES_RIGHT[i]]!=-1);
				NEW_RULES_LEFT[index]=symbPerm[RULES_LEFT[i]];
				NEW_RULES_RIGHT[index++]=symbPerm[RULES_RIGHT[i]];
			}
		}
		for (uint i=0;i<n-alph;i++)
			lengths[symbPerm[i+alph]]=rule_lengths[i];
		uint *tmp =RULES_LEFT;
		RULES_LEFT = NEW_RULES_LEFT;
		delete [] tmp;
		tmp = RULES_RIGHT;
		RULES_RIGHT = NEW_RULES_RIGHT;
		delete [] tmp;
		//~ assert(index==n-new_alph);
		#if VERBOSE
		for (size_t i=0;i<n-(new_alph+ndel);i++){
			cout << "rule-" << i+new_alph << "->" << RULES_LEFT[i] << " " << RULES_RIGHT[i] << endl;
		}
		#endif
		//cerr << "ntrimed: " << ntrim << endl;
		newn=n-ndel;
		//cerr << "First: " << newC[0] << endl;
		for (int i=0;i<c;i++){
			newC[i]=symbPerm[newC[i]];
		}
		delete  deleted;
		delete  trimed;
		delete [] bitVectorsTrim;
		return new_alph;
    }
    
    /**
     * Writes in a bitVector (terminals) the concatenation of 
     * all the bitstrings generated by the trimed rules. In lenTerms
     * stores a pointer to the beggining of each rule.
     * In symbPerm we store a permutation with the new symbol of each 
     * terminal symbol. (terminal symbols are identified with 
     * the lower numbers starting at 0). 
     * */
    uint GCT::buildVerbatimBitVector(HashMap* trimed, uint *symbPerm, uint *rules_length){
		size_t len=0;
		size_t new_alph=2;
		for (size_t i=0;i<n-alph;i++){
			if (trimed->get(i)!=-1){
				//~ cout << "i: " << i << ", with len: " << rules_length[i] << endl;
				new_alph++;
				len+=rules_length[i];
			}
		}
		//cerr << "bits trimmed: " << len << endl;
		lenTerms = new uint[new_alph+1];
		terminals = new uint[uint_len(max(len+1,(size_t)1),1)];
		//cerr << "uint_len: " << uint_len(max(len,(size_t)1),1) << endl;
		assert(uint_len(max(len,(size_t)1),1)>0);
		for (size_t i=0;i<uint_len(max(len,(size_t)1),1);i++) terminals[i]=0;
		size_t index,indexLen;
		lenTerms[0]=0;lenTerms[1]=1;lenTerms[2]=2;
		symbPerm[0]=0;symbPerm[1]=1;
		bit_set(terminals, 1);
		index=2;indexLen=3;
		for (size_t i=0;i<n-alph;i++){
			if (trimed->get(i)!=-1){
				symbPerm[i+alph]=indexLen-1;
				for (size_t j=0;j<rules_length[i];j++){
					if (bitget(bitVectorsTrim[i].first,j)){
						bit_set(terminals, index);
					}
					index++;
				}
				lenTerms[indexLen++]=index;
			}
		}
		assert(index==len+2);
		#if 0
		for (size_t i=0;i<indexLen;i++){
			cout << lenTerms[i] << endl;
		}
		#endif
		return new_alph;
	}
	
	size_t GCT::getSizeVerbose() const{
		cerr << "Verbatim grammar: " << (double)(2*n+c)*log(n)/log(2)/8 << ""<<endl;
        size_t total = 0;
        cerr << "Dictionary: " << dic->getSize() << endl;
        total+=dic->getSize();//DICTIONARY
        
        cerr << "C: " << sizeof(uint)*nb_C << endl;
        total+=sizeof(uint)*nb_C;//Initial rule
        
        
        assert(compressedSampleC_p->getSize()>0);
        cerr << "Sampled pos in C: " << compressedSampleC_p->getSize() << endl;
        total+=compressedSampleC_p->getSize();//pos sampled in C
        
        assert(compressedSampleC_o>0);
        cerr << "Sampled offset in C: " << compressedSampleC_o->getSize() << endl;
        total+=compressedSampleC_o->getSize();//offsets sampled in C
        
        
        assert(heapSampleC_e>0);
        cerr << "Heap excess: " << heapSampleC_e->getSize() << endl;
        total+=heapSampleC_e->getSize();//excess sampled in C and heap
        
        
        assert(heapSampleC_minE>0);
        //~ total+=heapSampleC_minE->getSize();//min excess sampled in C and heap
        cerr << "Heap min excess: " << heapSampleC_minE_array->getSize() << endl;
        total+=heapSampleC_minE_array->getSize();//min excess sampled in C and heap
        
        
        assert(compressedRulesMinExcess>0);
        cerr << "Rules MIN excess: " << compressedRulesMinExcess->getSize() << endl;
        total+=compressedRulesMinExcess->getSize();//minExcess sampled (rules)
        
        
        assert(compressedSampledLengths>0);
        cerr << "Rules lengths: " << compressedSampledLengths->getSize() << endl;
        total+=compressedSampledLengths->getSize();//lengths sampled (rules)
        
        
        cerr << "Rules total excess: " << compressedSampledTotalExcess->getSize() << endl;
        assert(compressedSampledTotalExcess>0);
        total+=compressedSampledTotalExcess->getSize();//excess sampled (rules)
        
        cerr << "Rules leafs: " << compressedRuleLeafs->getSize()<<endl;
        total+=compressedRuleLeafs->getSize();
        
        cerr << "Sampled leafs in C: " << compressedSampleC_leafs->getSize()<<endl;
		total+=compressedSampleC_leafs->getSize();
        
        cerr << "All fsb/lstb of leafs: " << totalSampledRuleBitsInBits << endl;
        total+=totalSampledRuleBitsInBits;//first-bit and last-bit of each rule
        
        if(delta>0){
			cerr << "Bitmap sampling: " << bd->getSize() << endl;
			total+=bd->getSize();
		}

        cout << "Total: " << total << endl;
        cout << "~bpc: " << 4*(double)total*8/len << endl;
        cout << "Ratio: " << total / ((double)(2*n+c)*log(n)/log(2)/8) << endl;
        return total;
	}

    ulong GCT::getSize() const {
        size_t total = 0;
        total+=dic->getSize();//DICTIONARY
        total+=sizeof(uint)*nb_C;//Initial rule
        assert(compressedSampleC_p->getSize()>0);
        total+=compressedSampleC_p->getSize();//pos sampled in C
        assert(compressedSampleC_o->getSize()>0);
        total+=compressedSampleC_o->getSize();//offsets sampled in C
        assert(heapSampleC_e->getSize()>0);
        total+=heapSampleC_e->getSize();//excess sampled in C and heap
        assert(heapSampleC_minE_array->getSize()>0);
        //~ total+=heapSampleC_minE->getSize();//min excess sampled in C and heap
        total+=heapSampleC_minE_array->getSize();//min excess sampled in C and heap
        assert(compressedRulesMinExcess->getSize()>0);
        total+=compressedRulesMinExcess->getSize();//minExcess sampled (rules)
        assert(compressedSampledLengths->getSize()>0);
        total+=compressedSampledLengths->getSize();//lengths sampled (rules)
        assert(compressedSampledTotalExcess->getSize()>0);
        total+=compressedSampledTotalExcess->getSize();//excess sampled (rules)
        assert(compressedRuleLeafs->getSize()>0);
        total+=compressedRuleLeafs->getSize();
		total+=compressedSampleC_leafs->getSize();
		if(delta>0)
			total+=bd->getSize();
//        total+=totalSampledRuleBitsInBits/sizeof(uint);
        return total;
    }

    //TODO
    void GCT::save(ofstream & fp) const {
    	uint wr = GCT_HDR;
    	saveValue(fp, wr);
    	saveValue(fp, n);
    	saveValue(fp, c);
    	saveValue(fp, nb_C);
		saveValue(fp,nSamplesC);
    	saveValue(fp,delta);
    	saveValue(fp,alph);
    	saveValue(fp,C,nb_C);
    	saveValue(fp,blockSizeC);
    	saveValue(fp,blogBlockSizeC);
    	saveValue(fp,blogBlockSizeCPlus1);
    	saveValue(fp,b);
    	saveValue(fp, heapDepth);
    	dic->save(fp);
    	saveValue(fp, lengthsHeapLevels,heapDepth);
    	compressedSampleC_p->save(fp);
    	compressedSampleC_o->save(fp);
    	heapSampleC_e->save(fp);
    	heapSampleC_minE_array->save(fp);
    	compressedRulesMinExcess->save(fp);
    	compressedSampledLengths->save(fp);
    	compressedSampledTotalExcess->save(fp);
    	compressedRuleLeafs->save(fp);
    	compressedSampleC_leafs->save(fp);
    	saveValue(fp, ruleFstBit, uint_len(n-alph,1));
    	saveValue(fp, ruleLstBit, uint_len(n-alph,1));
    	bd->save(fp);
    	#if !defined NDEBUG
			bsExcessBuild->save(fp);
		#endif
    }

    void GCT::initTables(){
    	EXCESS = new int[2];
    	EXCESS[0]=-1;
    	EXCESS[1]= 1;	

    	OVERLAP_LEAFS[0]=0;//0=00 -> 0 leaf
		OVERLAP_LEAFS[1]=0;//1=01 -> 0 leaf
		OVERLAP_LEAFS[2]=1;//2=10 -> 1 leaf
		OVERLAP_LEAFS[3]=0;//3=11 -> 0 leaf
    }


    GCT * GCT::load(ifstream &fp) {
    	GCT *ret;
    	uint rd = loadValue<uint>(fp);
    	try{
    		if (rd!=GCT_HDR){
    			abort();
    		}
    		ret = new GCT();
    		ret->n = loadValue<ulong>(fp);
    		ret->c = loadValue<uint>(fp);
    		ret->nb_C = loadValue<uint>(fp);
			ret->nSamplesC = loadValue<uint>(fp);
    		ret->delta = loadValue<uint>(fp);
    		ret->alph = loadValue<uint>(fp);
    		ret->C = loadValue<uint>(fp,ret->nb_C);
    		ret->blockSizeC = loadValue<uint>(fp);
    		ret->blogBlockSizeC = loadValue<uint>(fp);
    		ret->blogBlockSizeCPlus1 = loadValue<uint>(fp);
    		ret->b = loadValue<uint>(fp);
    		ret->heapDepth = loadValue<uint>(fp);

    		ret->dic = DictionaryRP::load(fp);
    		
    		ret->lengthsHeapLevels = loadValue<uint>(fp,ret->heapDepth);
    		ret->compressedSampleC_p = DAC::load(fp);
			ret->compressedSampleC_o = DAC::load(fp);
			ret->heapSampleC_e = DAC::load(fp);
			ret->heapSampleC_minE_array = new Array(fp);
			ret->compressedRulesMinExcess = DAC::load(fp);
			ret->compressedSampledLengths = DAC::load(fp);
			ret->compressedSampledTotalExcess = DAC::load(fp);
			ret->compressedRuleLeafs = DAC::load(fp);
			ret->compressedSampleC_leafs = DAC::load(fp);
			ret->ruleFstBit = loadValue<uint>(fp,uint_len(ret->n-ret->alph,1));
			ret->ruleLstBit = loadValue<uint>(fp,uint_len(ret->n-ret->alph,1));
			ret->bd = BitSequence::load(fp);
			ret->bsLeafsTest = nullptr;
			#if !defined NDEBUG
				ret->bsExcessBuild = BitSequence::load(fp);
			#endif
			
			ret->initTables();

    	}catch(...){
    		delete ret;
    		ret = NULL;
    	}
    	return ret;
    }

	GCT::~GCT(){
		delete [] C;
		delete dic;
		delete compressedSampleC_p;
		delete compressedSampleC_o;
		delete heapSampleC_e;
		delete heapSampleC_minE_array;
		delete compressedRulesMinExcess;
		delete compressedSampledLengths;
		delete compressedSampledTotalExcess;
		delete compressedRuleLeafs;
		delete compressedSampleC_leafs;
		delete [] ruleFstBit;
		delete [] ruleLstBit;
		if (delta>0) delete bd;
		#if !defined NDEBUG
			delete bsExcessBuild;
		#endif
		delete [] EXCESS;
	}

    #if 0
    GCT::~GCT() {
        delete dic;
        delete [] ruleLeafs;
        delete [] ruleFstBit;
        delete [] ruleLstBit;
        delete [] sampledRuleFstBit;
        delete [] sampledRuleLstBit;
        delete compressedSampledLengths;
        delete compressedSampledTotalExcess;
        delete compressedRulesMinExcess;
        delete compressedRuleLeafs;
        delete heapSampleC_e;
        if (heapSampleC_minE) delete heapSampleC_minE;
        if (heapSampleC_minE_array) delete heapSampleC_minE_array;
        delete compressedSampleC_o;
        delete compressedSampleC_p;
        delete compressedSampleC_leafs;
        delete sampleC_leafs;
        delete [] heapLevelSize;
        delete [] sampledLeafs;
        delete [] sampledLengths;
        delete [] sampledMinExcess;
        delete [] sampledTotalExcess;
        delete [] sampleC_o;
        delete [] sampleC_p;
        delete [] sampleC_e;
        delete [] lengthsHeapLevels;
        delete [] bitVectorsTrim;
        delete []  sampleC_minE;
        
    }
    #endif


    ulong GCT::fwdExcess(uint s, uint rel){
    	int r = -(rel+1);
    	return fwd_search((size_t)s, r);
    }
    ulong GCT::bwdExcess( uint s, int rel){
    	int r = -(rel+1);
    	return bwd_search((size_t)s, r);
    }
    ulong GCT::rmq( uint s, int t, int &m){
    	size_t mi;
    	ulong ret = (ulong)rmqi((size_t)s,(size_t)t,mi);
    	m = (int)mi;
    	return ret;
    }

    ulong GCT::rankOpen( uint s){
    	return rank1(s);
    }
    ulong GCT::rankClose( uint s){
    	return rank0(s);
    }
    ulong GCT::selectOpen( uint s){
    	size_t e;
    	return select1(s,e);
    }
    ulong GCT::selectClose( uint s){
    	size_t e;
    	return select0(s,e);
    }
    ulong GCT::root_node(){
    	return 0;
    }
    ulong GCT::inspect(uint s){
    	return access(s);
    }

    /**
	 * Requires P[i]=1.
	 * Finds the closing parentheses of P[i].
	 * 
	 * */
    ulong GCT::findClose(uint pos){
	    //assert(bsExcessBuild->access(pos));
		if (pos==0) return n-1;
	    int pos1 = pos + 1;
  		if (!access(pos1))
    		return pos1;
		size_t j = fwd_search(pos,-2);
		return j;
    }
    ulong GCT::findOpen(uint s){
    	ulong j = bwd_search((size_t)s,-1);
    	return j+1;
    	//throw runtime_error("findOpen not implemented");
    }
    
    ulong GCT::parent(uint s){
    	if (s==root_node()) return 0;
    	return enclose(s);
    }

    ulong GCT::parentClose(uint s){
    	throw runtime_error("Parent close not implemented");
    	return 0;
    }
	/* * Require P[pos]=1
	 * Being E[pos] (the excess at position pos), finds the position j<pos
	 * s.t. E[j]=E[pos]-2. Finally returns j+1, what is the position of the 
	 * parent of P[pos].
	 * **/
	ulong GCT::enclose(uint pos){
		ulong j = bwd_search((size_t)pos,-3);
		return j+1ul;
    }

    ulong GCT::levelAncestor(uint s, uint d){
        return bwd_search(s,d)+1;
    }
    ulong GCT::lca(uint a, uint b){
    	if (isAncestor(a,b) || isAncestor(b,a))
			return min(a,b);
		size_t e;
		return parent(rmqi(a, b, e) + 1);
    }
    ulong GCT::preorderRank(uint s){
    	throw runtime_error("preorderRank not implemented");
    	return 0;
    }
    ulong GCT::preorderSelect(uint s){
    	throw runtime_error("preorderSelect not implemented");
    	return 0;
    }
    ulong GCT::postorderRank(uint s){
    	throw runtime_error("postorderRank close not implemented");
    	return 0;
    }
	//ith is the i-th opened parentheses (it returns the i-th+1)
	ulong GCT::nextNode(ulong ith){
		size_t e;
		if (ith==n/2) return (ulong)-1;
		return select1(ith+1,e);
	}

	ulong GCT::nextNodeBottomUp(ulong pos){

		if (pos==0) return (ulong)-1;
		ulong nextSib = nextSibling(pos);
		if (nextSib==(ulong)-1){
			return parent(pos);
		}else{
			return leftmostLeaf(nextSib);
		}
	}

	ulong GCT::id(ulong pos){
		if (isLeaf(pos)){
			//the leafs are marked as 01
			return brLeafRank(pos+1)-1;
		}else{
			size_t e;
			return rank1(pos,e) - brLeafRank(pos)-1;
		}
	}
    /**
    * Assumes s is a node, that is, access(s)==true.
    */
    ulong GCT::isLeaf(uint s){
    	assert(access((size_t)s));
    	return !access((size_t)s+1);
    }

    ulong GCT::subtreeSize(uint s){
    	ulong res = findClose(s);
    	assert(res&0x1);
    	return (res-s-1)>>1;
    }
    ulong GCT::prevSibling(uint x){
    	if (x==0) return (ulong)-1;
    	if (access(x-1)) return (ulong)-1;
    	return findOpen(x-1);
    }

	ulong GCT::leftmostLeaf(ulong pos){

		if (isLeaf(pos))
			return pos;

		ulong rnk = brLeafRank(pos);
		if (rnk==n/2){
			return (ulong)-1;
		}
		return brLeafSelect(rnk+1);
	}

	ulong GCT::lb(ulong pos){
		if (pos>n) return (ulong)-1;
		return brLeafRank(pos);
	}

	ulong GCT::rb(ulong pos){
		if (pos>n) return (ulong)-1;
		return brLeafRank(findClose(pos))-1;
	}

    /**
	 * We are supposing P[i]=1. 
	 * */
    ulong GCT::firstChild(uint s){
  //   	assert(access(s));
  //   	if (!isLeaf(s))
		// 	return s+1;
		// return -1;
		int s1 = s + 1;
  		return access(s1)? s1: (ulong)-1;
    }
    ulong GCT::nextSibling(uint s){
  //   	ulong j = findClose(s);
		// if (access((size_t)j+1))
		// 	return j+1;
		// return -1;
		int t;
  		t = findClose(s)+1;
  		return access(t)?t:(ulong)-1;
    }

    ulong GCT::isAncestor(uint a, uint b){
    	return (a<=b) && (b<=findClose(a));
    }
}
