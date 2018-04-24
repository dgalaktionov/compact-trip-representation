/* LZBaseLimitHops.h
 * Copyright (C) 2014-current_year, Alberto Ordóñez, all rights reserved.
 *
 * Streategies to obtain the base sequence from a LZ Parsing. Adds to the base those whose length is <=beta 
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

#include <sequence/LZBaseLimitHops.h>
#include <utils/libcdsBasics.h>
using namespace std;
using namespace cds_utils;

namespace cds_static
{

	
	LZBaseLimitHops::LZBaseLimitHops(uint max_hops){
		this->beta = max_hops;
	}
	/** Destroys the Wavelet Tree */
	LZBaseLimitHops::~LZBaseLimitHops(){

	}

	LZBaseLimitHops* LZBaseLimitHops::setDelta(uint max_hops){
		this->beta = max_hops;
		return this;
	}

	uint LZBaseLimitHops::computeBase(uint *input, size_t length,  SequenceLZEndBase *seq){
			uint *C = new uint[length+1];
		uint pointerBase=0;
		uint *bmpExplicits;
		createEmptyBitmap(&bmpExplicits,seq->z+1);
		uint *pBase = new uint[seq->z+1];
		uint nExplicit=0;
		for (uint i=0;i<length;i++) C[i]=0;
		
		uint prevPos = 0;
		uint nrules = seq->z;
		
		uint *maxC_rule = new uint[nrules];

		uint pos=0;
		for (uint i=1;i<=nrules;i++){
			if (seq->phraseLengths[i-1]==0){
				maxC_rule[i-1]=0;
				assert(pos<length);
				C[pos]=0;
				pos++;
			}else{
				pos = seq->getPointerRule(i);
				assert(pos<=length);
				C[pos]=0;
				maxC_rule[i-1]=0;
				int j= pos-1;
				uint ref = seq->getPointerRule((*(seq->refs))[i-1]+1);
				for (int d=0;j>prevPos;j--,d++){
					C[--pos] = C[ref-d]+1;
					maxC_rule[i-1]=max(maxC_rule[i-1],C[pos]);
					if (maxC_rule[i-1]>=beta) break;
				}
				if (maxC_rule[i-1]>=beta){
					pos = seq->getPointerRule(i);
					for (int j=pos;j>prevPos;j--)
						C[j]=0;
				}

			}
			prevPos = seq->getPointerRule(i);
		}
		uint toExplicit=0;
		uint lengthExplicit = 0;
		
		for (uint i=0;i<nrules;i++){
			if (maxC_rule[i]>=beta ){
				toExplicit++;
				lengthExplicit+=(seq->phraseLengths[i]+1);
			}
		}
		cerr << "avgLength: " << 1.0*lengthExplicit/toExplicit << endl;
		cerr << "toExplicit: " << toExplicit << endl;
		cerr << "length base: " << lengthExplicit << endl;
		cerr << "%base out of total: " << 1.0 * lengthExplicit / length << endl;
		uint *sequence = new uint[lengthExplicit+255];
		for (uint i=0;i<nrules;i++){

			if (seq->phraseLengths[i]>0 && maxC_rule[i]<beta) continue;
			if (seq->phraseLengths[i]==0){
				bitset(bmpExplicits,i);
				sequence[pointerBase]=seq->trailing[i];
				pBase[nExplicit++]=pointerBase;
				pointerBase++; 
			}else{
				uint start, end;
				end = seq->getPointerRule((*(seq->refs))[i]+1);
				start = end-seq->phraseLengths[i]+1;
				
				bitset(bmpExplicits,i);
				for (uint j=start;j<=end;j++){
					sequence[pointerBase++]=input[j]-1;
				}
				sequence[pointerBase]=seq->trailing[i];
				pBase[nExplicit++]=pointerBase;
				pointerBase++;
			}
		}

		// uint max_v_base=0;
		// if (sequence[pointerBase-1]==0){
		// 	for (uint i=0;i<pointerBase;i++)
		// 		max_v_base=max(max_v_base,sequence[i]);
		// 	sequence[pointerBase-1]=max_v_base+1;
		// }	
		//TODO...the symbols must be contiguous...
		cerr << "nExplicit: " << nExplicit << ", z: " << seq->z << endl;
		seq->explicity = new BitSequenceRG(bmpExplicits,seq->z,32);
		assert(seq->getPointerRule(seq->endings->rank1(seq->endings->getLength()-1))==length);
		assert(nExplicit<=seq->z);

		seq->pointersBase = new DAC(pBase,nExplicit);
		uint ngaps,*gaps=nullptr;
		getGaps(sequence,(size_t)pointerBase,ngaps,gaps);
		if (gaps){
			//padding the sequence
			for (uint i=0;i<ngaps;i++)
				sequence[pointerBase++]=gaps[i];
		}

		// Sequence *seq_base = new SequenceLZEnd(sequence,pointerBase,dic, false);
		// cerr << "seq_base: " << seq_base->getSize()*8.0 / pointerBase << endl;

		seq->base = seq->seqBuilder->build(sequence,pointerBase);

		pointerBase-=ngaps;
		// assert(testExtractPhrases(input));
		delete [] gaps;
		delete [] bmpExplicits;
		delete [] sequence;
		delete [] pBase;
		return 0;
	}
	
};
