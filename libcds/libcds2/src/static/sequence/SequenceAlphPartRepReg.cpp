/*  SequenceAlphaPartRep.cpp
 * Copyright (C) 2014-current_year, Alberto Ordóñez, all rights reserved.
 *
 * Alberto Ordóñez <alberto.ordonez@udc.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <sequence/SequenceAlphPartRepReg.h>

#include <iostream>
#include <algorithm>
#include <grammar/Repair.h>

using namespace std;


namespace cds_static
{

	bool SequenceAlphPartRepReg::testGroupAndOffset() {
		
		uint prevCut, prevNsecond, prevSecondSize;
		prevCut = this->cut;
		prevNsecond = this->nSecondGroups;
		prevSecondSize = this->secondGroupSize;

		uint c, s1,s2;
		c = 4;
		s1 = 4; 
		s2 = 3;
		initParameters(c,s1,s2);
		
		assert(group(0, cut)==0);
		assert(group(1, cut)==1);
		assert(group(cut-1, cut)==cut-1);
		assert(group(cut, cut)==cut);
		assert(group(cut+1, cut)==cut && secondGroupSize>=1);
		assert(group(cut+secondGroupSize-1, cut)==cut && secondGroupSize>=1);
		assert(group(cut+secondGroupSize, cut)==cut+1 && secondGroupSize>=1);
		assert(group(cut+secondGroupSize*nSecondGroups-1, cut)==cut+nSecondGroups-1 && secondGroupSize>=1);
		assert(group(cut+secondGroupSize*nSecondGroups, cut)==cut+nSecondGroups && secondGroupSize>=1);
		uint firstThirdGroupSize = this->maskThirdGroup*2;
		assert(group(cut+secondGroupSize*nSecondGroups+firstThirdGroupSize-1, cut)==cut+nSecondGroups && secondGroupSize>=1);
		assert(group(cut+secondGroupSize*nSecondGroups+firstThirdGroupSize, cut)==cut+nSecondGroups+1 && secondGroupSize>=1);


		assert(offset(0,cut,0)==0);
		assert(offset(1,cut,0)==0);
		assert(offset(cut-1,cut,0)==0);
		assert(offset(cut,cut,0)==0);
		assert(offset(cut+1,cut,0)==1);
		assert(offset(cut+secondGroupSize-1,cut,0)==secondGroupSize-1);
		assert(offset(cut+secondGroupSize,cut,0)==0);
		assert(offset(cut+secondGroupSize*nSecondGroups,cut,0)==0);
		assert(offset(cut+secondGroupSize*nSecondGroups+1,cut,0)==1 || firstThirdGroupSize<1);
		assert(offset(cut+secondGroupSize*nSecondGroups+2,cut,0)==2 || firstThirdGroupSize<2);
		assert(offset(cut+secondGroupSize*nSecondGroups+firstThirdGroupSize-1,cut,0)==firstThirdGroupSize-1 || firstThirdGroupSize<2);
		assert(offset(cut+secondGroupSize*nSecondGroups+firstThirdGroupSize,cut,0)==0 || firstThirdGroupSize<2);
		assert(offset(cut+secondGroupSize*nSecondGroups+2*firstThirdGroupSize-1,cut,0)==firstThirdGroupSize-1 || firstThirdGroupSize<2);
		assert(offset(cut+secondGroupSize*nSecondGroups+2*firstThirdGroupSize,cut,0)==0 || firstThirdGroupSize<2);
		assert(offset(cut+secondGroupSize*nSecondGroups+3*firstThirdGroupSize,cut,0)==firstThirdGroupSize || firstThirdGroupSize<2);
		if (c==4 && s1==4 && s2==3) assert(offset(cut+secondGroupSize*nSecondGroups+64+32,cut,0)==32 );
		initParameters(bits(prevCut)-1, prevNsecond, prevSecondSize);
		return true;
	}
	/**
	 * We create a class with just one element for the first cut symbols. The 
	 * remaining are stored in clases as the verbatim algorithm says. 
	 * */
	inline uint SequenceAlphPartRepReg::group(uint pos, uint cut) const{
		uint ret = 0;
		if(pos<=cut){
			ret = pos;
		}else if (pos<=cut+(nSecondGroups*secondGroupSize)){
			ret = (pos-cut) / secondGroupSize+cut;
		}else{
			ret = bits((pos-cut-secondGroupSize*nSecondGroups)|maskThirdGroup)-minBitThirdGroup+cut+nSecondGroups;
		}
		return ret;
	}

	/**
	 * Offset inside each class. If the class is <=cut the offset is 0 since 
	 * the class has only one element. 
	 * */
	inline uint SequenceAlphPartRepReg::offset(uint pos, uint cut, uint group) const{
		uint ret = 0;
		if(pos<=cut){
			ret = 0;
		}else if (pos<=cut+(nSecondGroups*secondGroupSize)){
			pos-=cut;
			ret = pos % secondGroupSize;
		}else{
			pos -= (cut+nSecondGroups*secondGroupSize);
			uint b = pos&maskThirdGroupOffset;
			if (b)
				ret = pos - (1<<(bits(b)-1));
			else
				ret = pos;

		}
		return ret;
	}

	void SequenceAlphPartRepReg::initParameters(uint _cut,uint _nSecondGroups, uint _secondGroupSize){
		//NOTE: in SequenceAlphPart, cut = (1<<_cut)-1
		//this->cut = (1<<_cut)-1;
		this->cut = 1<<_cut;
		this->secondGroupSize = _secondGroupSize;
		this->nSecondGroups = _nSecondGroups;
		this->maskThirdGroupOffset = (~0u) << (bits(this->cut + this->secondGroupSize*this->nSecondGroups)-1);
		this->minBitThirdGroup = bits(this->cut + this->secondGroupSize*this->nSecondGroups)-1;
		this->maskThirdGroup = 1<<(this->minBitThirdGroup-1);
		this->cteGroupSizes = this->cut+1+this->nSecondGroups*this->secondGroupSize;	
	}

	SequenceAlphPartRepReg::SequenceAlphPartRepReg(uint * seq, size_t n, uint _cut, uint _nSecondGroups, uint _secondGroupSize, SequenceBuilder * lenIndexBuilder, 
						SequenceBuilder * seqsBuilder, SequenceBuilder * seqFirstClasses, PermutationBuilder *pb) : SequenceAlphPart(n) {

		// Say we are using the builders
		lenIndexBuilder->use();
		seqsBuilder->use();

		
		initParameters(_cut, _nSecondGroups,  _secondGroupSize);
		
		assert(testGroupAndOffset());
		// Compute the size of the alphabet
		sigma = 0;
		for(uint i=0;i<n;i++) sigma=max(sigma,seq[i]);

		// Compute the frequency of each symbol
		size_t * occ = new size_t[sigma+1];
		for(uint i=0;i<=sigma;i++) occ[i] = 0;
		for(uint i=0;i<n;i++) occ[seq[i]]++;

		// Create pairs (frequency,symbol) and then sort by frequency
		pair<size_t,uint> * pairs = new pair<size_t,uint>[sigma+2];
		for(uint i=0;i<=sigma;i++)
			pairs[i] = pair<size_t,uint>(occ[i],i);
		pairs[sigma+1] = pair<size_t,uint>(0,sigma+1);
		sort(pairs,pairs+sigma+2,greater<pair<size_t,uint> >());

		uint * groupForSymb = new uint[sigma+2];
		revPermFreq = new uint[sigma+2];

		origsigma = sigma;
		sigma = 0;
		while(pairs[sigma].first>0) {
			revPermFreq[pairs[sigma].second]=sigma;
			sigma++;
		}

		// We don't need occ anymore
		delete [] occ;

		// Fill alphSortedbyFreq and lengthForSymb
		alphSortedByFreq = new uint[sigma+1];

		// We estimate maxLen, it may be smaller if many symbols have 0 frequency
		maxLen = group(sigma,cut);

		// Initialize the lengths of each sequence in indexesByLength
		uint * lenLength = new uint[maxLen+1];
		for(uint i=0;i<maxLen;i++)
			lenLength[i] = 0;

		// Compute the actual value for lenLengths and maxLen
		for(uint i=0;i<=sigma;i++) {
			if(pairs[i].first==0) break;
			alphSortedByFreq[i] = pairs[i].second;
			uint sl = group(i,cut);
			groupForSymb[pairs[i].second] = sl;
			lenLength[sl]+=pairs[i].first;
			maxLen = sl;
		}
		
		// Now we build lengthsIndex
		uint * tmpSeq = new uint[n];
		for(uint i=0;i<n;i++)
			tmpSeq[i] = groupForSymb[seq[i]];
		groupsIndex = lenIndexBuilder->build(tmpSeq,n);
		delete [] tmpSeq;

		if ((int)(maxLen-cut+1)<=0){
			cerr << "Warning: AlphabetPartRepReg contains just one sequence" << endl;
		}

		if ((int)(maxLen-cut+1)>0){
			// Now we build the other sequences
			uint ** seqs = new uint*[maxLen-cut+1];
			for(uint i=0;i<maxLen-cut;i++)
				seqs[i] = new uint[lenLength[i+cut+1]];

			// Lets compute the offsets
			for(uint i=0;i<maxLen+1;i++)
				lenLength[i] = 0;

			for(uint i=0;i<n;i++) {
				if(groupForSymb[seq[i]]>cut) {
					seqs[groupForSymb[seq[i]]-cut-1][lenLength[groupForSymb[seq[i]]]++] = offset(revPermFreq[seq[i]],cut,groupForSymb[seq[i]]);
					//cout << "Group=" << groupForSymb[seq[i]] << " offset=" << offset(revPermFreq[seq[i]],cut,groupForSymb[seq[i]]) << endl;
				}
			}

			indexesByLength = new Sequence*[maxLen-cut+1];
			for(uint i=0;i<maxLen-cut;i++) {
				if (i<nSecondGroups){
					indexesByLength[i] = seqFirstClasses->build(seqs[i],lenLength[i+cut+1]);
				}else{
					indexesByLength[i] = seqsBuilder->build(seqs[i],lenLength[i+cut+1]);
				}
				delete [] seqs[i];
			}
			delete [] seqs;
		}
		
		delete [] groupForSymb;
		delete [] lenLength;
		delete [] pairs;

		// We are done with the builders
		lenIndexBuilder->unuse();
		seqsBuilder->unuse();

		this->maxLen = maxLen;
		this->sigma = sigma;
	}

	SequenceAlphPartRepReg::SequenceAlphPartRepReg() : SequenceAlphPart(0) {
		length = 0;
		sigma = 0;
	}

	SequenceAlphPartRepReg::~SequenceAlphPartRepReg() {
		for(uint i=0;maxLen>cut && i<maxLen-cut;i++)
			delete indexesByLength[i];
		delete [] indexesByLength;
		delete groupsIndex;
		delete [] revPermFreq;
		delete [] alphSortedByFreq;
	}

	void SequenceAlphPartRepReg::save(ofstream & fp) const
	{
		uint type = ALPHPARTREP_HDR;
		saveValue(fp,type);
		saveValue(fp,length);
		saveValue(fp,sigma);
		saveValue(fp,origsigma);
		saveValue(fp,maxLen);
		saveValue(fp,cut);
		saveValue(fp,revPermFreq,sigma+1);
		saveValue(fp,alphSortedByFreq,sigma+1);
		groupsIndex->save(fp);
		for(uint i=0;maxLen>cut && i<maxLen-cut;i++)
			indexesByLength[i]->save(fp);
	}

	SequenceAlphPartRepReg * SequenceAlphPartRepReg::load(ifstream & fp) {
		uint type = loadValue<uint>(fp);
		if(type!=ALPHPART_HDR)
			return NULL;
		SequenceAlphPartRepReg * ret = new SequenceAlphPartRepReg();
		ret->length = loadValue<size_t>(fp);
		ret->sigma = loadValue<uint>(fp);
		ret->origsigma = loadValue<uint>(fp);
		ret->maxLen = loadValue<uint>(fp);
		ret->cut = loadValue<uint>(fp);
		ret->revPermFreq = loadValue<uint>(fp,ret->sigma+1);
		ret->alphSortedByFreq = loadValue<uint>(fp,ret->sigma+1);
		ret->groupsIndex = Sequence::load(fp);
		ret->indexesByLength = new Sequence*[(ret->maxLen<=ret->cut)?0:(ret->maxLen-ret->cut)];
		for(uint i=0;ret->maxLen>ret->cut && i<ret->maxLen-ret->cut;i++)
			ret->indexesByLength[i] = Sequence::load(fp);
		return ret;
	}
	
};
