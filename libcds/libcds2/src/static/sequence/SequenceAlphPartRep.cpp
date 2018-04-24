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

#include <sequence/SequenceAlphPartRep.h>

#include <iostream>
#include <algorithm>
#include <grammar/Repair.h>

using namespace std;


namespace cds_static
{
	/**
	* We create a class with just one element for the first cut symbols. The
	* remaining are stored in clases as the verbatim algorithm says.
	* */
	inline uint SequenceAlphPartRep::group(uint pos, uint cut) const{
		uint ret = 0;
		if(pos<=cut)
			ret = pos;
		else
			ret = bits(pos)-bits(cut)+cut;
		return ret;
	}

	/**
	* Offset inside each class. If the class is <=cut the offset is 0 since
	* the class has only one element.
	* */
	inline uint SequenceAlphPartRep::offset(uint pos, uint cut, uint group) const{
		uint ret = 0;
		if(pos<=cut)
			ret = 0;
		else
			ret = pos-((1<<(group-cut+bits(cut)-1)));
		return ret;
	}

	SequenceAlphPartRep::SequenceAlphPartRep(uint * seq, size_t n, uint _cut, uint cutOffsets, SequenceBuilder * lenIndexBuilder,
			SequenceBuilder * seqsBuilder, SequenceBuilder * seqFirstClasses) : SequenceAlphPart(n) {
		int depth = 0;
		build(seq,n,_cut,cutOffsets,lenIndexBuilder,seqsBuilder,seqFirstClasses,nullptr,depth);
	}

	SequenceAlphPartRep::SequenceAlphPartRep(uint * seq, size_t n, uint _cut, uint cutOffsets, SequenceBuilder * lenIndexBuilder,
			SequenceBuilder * seqsBuilder, SequenceBuilder * seqFirstClasses, SequenceBuilder *recBuilder, int depth) : SequenceAlphPart(n) {

		build(seq,n,_cut,cutOffsets,lenIndexBuilder,seqsBuilder,seqFirstClasses,recBuilder,depth);
	}

	void SequenceAlphPartRep::build(uint * seq, size_t n, uint _cut, uint cutOffsets, SequenceBuilder *lenIndexBuilder,
			SequenceBuilder *seqsBuilder, SequenceBuilder *seqFirstClasses, SequenceBuilder *recBuilder, int depth){
		// Say we are using the builders
		lenIndexBuilder->use();
		seqsBuilder->use();
		seqFirstClasses->use();
		if (recBuilder) recBuilder->use();

		this->cut = (1<<_cut)-1;

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
		ulong acumSize = 0UL;
		// Now we build lengthsIndex
		uint * tmpSeq = new uint[n];
		for(uint i=0;i<n;i++)
			tmpSeq[i] = groupForSymb[seq[i]];
		groupsIndex = lenIndexBuilder->build(tmpSeq,n);
		acumSize+=groupsIndex->getSize();
		//cerr << "Acum size: " << acumSize << " (bps: " << acumSize*8.0/length << ")." << endl;
		delete [] tmpSeq;

		if ((int)(maxLen-cut+1)<=0){
			cerr << "Warning: AlphabetPartRep contains just one sequence" << endl;
		}
		// Now we build the other sequences
		uint ** seqs = NULL;
		if ((int)(maxLen-cut+1)>0){
			seqs = new uint*[maxLen-cut+1];
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
//                cout << "Level: " << i << " of " << maxLen-cut<<"in SeqAlphPartRep"<< endl;
				if (i<cutOffsets){
                    if (lenLength[i+cut+1]>100) {
                        indexesByLength[i] = seqFirstClasses->build(seqs[i], lenLength[i + cut + 1]);
                        acumSize += indexesByLength[i]->getSize();
                    }else{
                        SequenceBuilderWaveletMatrix sb(new BitSequenceBuilderRG(8),new MapperNone());
                        indexesByLength[i] = sb.build(seqs[i], lenLength[i + cut + 1]);
                        acumSize += indexesByLength[i]->getSize();
                    }
					//cerr << "Acum size: " << acumSize << " (bps: " << acumSize*8.0/length << ")." << endl;
				}else{
					if (depth>0 && recBuilder){
						indexesByLength[i] = new SequenceAlphPartRep(seqs[i], lenLength[i + cut + 1], cut, cutOffsets,
									lenIndexBuilder,seqsBuilder, seqFirstClasses, recBuilder, depth-1);
					}else {
                        if (lenLength[i+cut+1]>100) {
                            indexesByLength[i] = seqsBuilder->build(seqs[i], lenLength[i + cut + 1]);
                            acumSize += indexesByLength[i]->getSize();
                        } else{
                            SequenceBuilderWaveletMatrix sb(new BitSequenceBuilderRG(8),new MapperNone());
                            indexesByLength[i] = sb.build(seqs[i], lenLength[i + cut + 1]);
                            acumSize += indexesByLength[i]->getSize();
                        }
					//	cerr << "Acum size: " << acumSize << " (bps: " << acumSize*8.0/length << ")." << endl;
					}
				}
				//~ for (uint j=0;j<lenLength[i+cut+1];j++){
				//~ if(indexesByLength[i]->access(j)!=seqs[i][j]){
				//~ cerr << indexesByLength[i]->access(j) << endl;
				//~ }
				//~ assert(indexesByLength[i]->access(j)==seqs[i][j]);
				//~ }
				delete [] seqs[i];
			}
		}
		delete [] seqs;
		delete [] groupForSymb;
		delete [] lenLength;
		delete [] pairs;

		// We are done with the builders
		lenIndexBuilder->unuse();
		seqsBuilder->unuse();
		seqFirstClasses->unuse();
		if (recBuilder) recBuilder->unuse();
		this->maxLen = maxLen;
		this->sigma = sigma;
	}

	SequenceAlphPartRep::SequenceAlphPartRep() : SequenceAlphPart(0) {
		length = 0;
		sigma = 0;
	}

	SequenceAlphPartRep::~SequenceAlphPartRep() {
		for(uint i=0;maxLen>cut && i<maxLen-cut;i++)
			delete indexesByLength[i];
		delete [] indexesByLength;
		delete groupsIndex;
		delete [] revPermFreq;
		delete [] alphSortedByFreq;
	}

	void SequenceAlphPartRep::save(ofstream & fp) const
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

	SequenceAlphPartRep * SequenceAlphPartRep::load(ifstream & fp) {
		cerr << "loading" << endl;
		uint type = loadValue<uint>(fp);
		if(type!=ALPHPARTREP_HDR)
			return NULL;
		SequenceAlphPartRep * ret = new SequenceAlphPartRep();
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

	void SequenceAlphPartRep::sortSymbols(uint *seq, uint n, uint sigma, uint *occ, pair<float,uint> *&pairs, Repair *rp){
		if (!rp) rp = new Repair((int*)seq,n);
		//cout << "Repair size: " << rp->getSize() << endl;
		// Create pairs (frequency,symbol) and then sort by frequency
		pairs = new pair<float,uint>[sigma+2];
		/**
		* The frequency of a symbol is the number of times the symbol appears in C +
		* the number of times it can be found as a leaf in any of the gramma rules.
		* */
		for(uint i=0;i<=sigma;i++){
			pairs[i].first=0;
			pairs[i].second=i;
		}
		for (int i=0;i<rp->getNC();i++){
			if (rp->getC(i)<rp->getAlpha()){
				pairs[rp->getC(i)].first++;
			}
		}
		for (int i=0;i<rp->getNTerm();i++){
			if (rp->getLeft(i)<rp->getAlpha())
				pairs[rp->getLeft(i)].first++;
			if (rp->getRight(i)<rp->getAlpha())
				pairs[rp->getRight(i)].first++;
		}
		for (uint i=0;i<=sigma;i++)
			pairs[i].first /= occ[i];

		pairs[sigma+1]=pair<float,uint>(n,sigma+1);
		sort(pairs,pairs+sigma+2,less<pair<float,uint> >());
		for (uint i=0;i<sigma;i++)
			assert(pairs[i].first<=pairs[i+1].first);
		delete rp;
	}

};
