/* SequenceLZEend.cpp
 * Copyright (C) 2014-current_year, Alberto Ordóñez, all rights reserved.
 *
 * Alberto Ordóñez <alberto.ordonez@udc.es>
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

#include <sequence/SequenceLZEnd.h>
 
namespace cds_static
{	
	
	SequenceLZEnd::SequenceLZEnd():Sequence(0){}

	SequenceLZEnd::~SequenceLZEnd(){
		delete endings;
		delete refs;
		delete [] trailing;
	}


	/*the input string $ terminated. The length of that string, including the $, is n -->s
	the length of the original string is n-1. The alphabet must contain contiguous 
	symbols in [0,sigma)*/
	SequenceLZEnd::SequenceLZEnd(uint *_input, size_t n, uint *_dic, bool del):Sequence(n-1){
		ss_rate = 1;
		build(_input,n,_dic,del,nullptr);
	}

	void SequenceLZEnd::buildFmRMQ(uint *input, size_t n, fmi **fm, RMQ **rmq){
		SequenceBuilder *seq_buidler = new SequenceBuilderWaveletMatrix(new BitSequenceBuilderRG(32),new MapperNone());
		(*fm) = new fmi(input,n,seq_buidler,false);
		(*fm)->set_samplepos(32)->set_samplesuff(32)->build_index();
		*rmq = new RMQ((int*)(*fm)->getSA(),n+1, RMQ_MAX);
		delete seq_buidler;
	}

	void SequenceLZEnd::build(uint *_input, size_t n, uint* _dic, bool del, Array *base){	
		z = 0;
		uint *input;
		if (!del){
			input = new uint[n];
			for (uint i=0;i<n;i++){
				input[i] = _input[i];
			}
		}else{
			input = _input;
		}
		this->dicSize=256;
		this->dic = _dic;
		this->invDic = new uint[dicSize];
		for (uint i=0;i<dicSize;i++){
			this->invDic[dic[i]]=i;
		}
		fmi *fm=nullptr;
		RMQ *rmq=nullptr;
		//printInput(input,fm,30);
		reverseStringAsUint(input, length);
		if (base){
			uint *b = new uint[base->getLength()];
			for (size_t i=0;i<base->getLength();i++)
				b[i]=base->getField(i);
			buildFmRMQ(b, base->getLength()-1, &fm, &rmq);
			delete [] b;
			delete base;
		}else{
			buildFmRMQ(input, length,	&fm, &rmq);
		}
		//the fm is adding a symbol at the end (the $), but we have already done it. 
		std::tuple<uint,uint,uint> *triplets;
		uint *bmpEndings;
		parse(input,fm,rmq,fm->getSA(),(uint*)fm->getInvSA(),triplets,&bmpEndings);
		assert(z!=0);
		// fm->freeSpace();
		
		reverseStringAsUint(input, length);
		//printInput(input,fm);
		compactDictionary(input,triplets,bmpEndings);

		//testParsing(input);
	
		uint **samples;
		sampleDictionary(input, endings, samples);
	
		compressDictionary(input, samples, ss_rate);
	
		addImprovements(input);

		delete [] bmpEndings;
		delete [] triplets;
		delete rmq;
		delete fm;
	}


	void SequenceLZEnd::build(uint *_input, size_t n, SequenceLZEnd *parsed, bool del ){	
		assert((n-1)==parsed->length);
		z = 0;
		uint *input;
		if (!del){
			input = new uint[n];
			for (uint i=0;i<n;i++){
				input[i] = _input[i];
			}
		}else{
			input = _input;
		}
		
		this->z = parsed->z;
		this->endings = parsed->endings;
		this->refs = parsed->refs;
		this->trailing = parsed->trailing;
		this->max_v = parsed->max_v;
		this->dicSize = parsed->dicSize;
		this->dic = parsed->dic;
		this->invDic = parsed->invDic;
		this->phraseLengths = nullptr;

		uint *ends = new uint[z];
		for (uint i=0;i<z;i++){
			ends[i] = parsed->getEndings()->select1(i+1);
		}
		BitSequence *new_end = new BitSequenceDelta(ends,z,128);
		cerr << "Delta endings: " << new_end->getSize()*8.0/length << endl;

		
		computePhraseLengths(this->endings);
		uint **samples;
		sampleDictionary(input, endings, samples);
	
		compressDictionary(input, samples, ss_rate);
	
		addImprovements(input);
	}

	void SequenceLZEnd::parse(uint *input, fmi *fm, RMQ* rmq, uint *A, uint *invA, 
			std::tuple<uint,uint,uint> *&triplets, uint **bmpEndings){
		assert(length>0);
		size_t i = 0;
		size_t iprime=0;
		uint ntriplets = length/bits(length);
		BinarySearchTree<uint> *bst = new BinarySearchTree<uint>();
		triplets = new std::tuple<uint,uint,uint>[ntriplets];
		uint ptriplets=0;
		
		createEmptyBitmap(bmpEndings,length+1);
		uint q=0;
		uint testJ=-1;
		while(i<length){
			iprime = i;
			uint j = i;
			q=0;
			size_t sp=0,ep=length-1;
			while(iprime<length){	
				uint nsp,nep;
				//input is inverted -> T[iprime] is at input[n-iprime-1]
				fm->BWS(sp,ep,input[length-iprime-1],nsp, nep);
				sp=nsp;
				ep=nep;
				uint mpos = rmq->getPos((int)nsp,nep);
				if (A[mpos]<=length-i-1) break;
				iprime++;
				BinarySearchTree<uint> *f = bst->successor(sp);
				if (f && f->getValue()<=ep){
					j=iprime;
					q = f->getValue();
				}
			}
			uint posA;
			uint trailing_char;
			if (length==j){
				posA = (length==j)?0:length-1-j;	
				trailing_char=0;
			}else{
				posA=length-1-j;
				trailing_char=input[posA]-1;
			}
			
				bst->insertKey(invA[posA]);
				if (ptriplets==ntriplets){
					ntriplets*=2;
					std::tuple<uint,uint,uint> *copy = new std::tuple<uint,uint,uint>[ntriplets];
					for (uint k=0;k<ptriplets;k++){
						copy[k]=triplets[k];
					}
					delete [] triplets;
					triplets=copy;
				}
				//why input[pos]-1. If we remove the $, we have to substract -1 to each symbol so that 
				//all the useful ones start at 0 not at 1. 

				triplets[ptriplets++] = std::make_tuple(max((int)0,(int)(length-A[q]-1)),j-i,trailing_char);
				assert(j!=testJ);
				testJ=j;
				bitset(*bmpEndings,j);
				if (i<40) cerr << "(p,l,c): (" << std::get<0>(triplets[ptriplets-1]) << "," << std::get<1>(triplets[ptriplets-1]) << "," << (char) invDic[std::get<2>(triplets[ptriplets-1])+1] << ")" << endl;
			
			i=j+1;

		}
		z = ptriplets;
	}


	size_t SequenceLZEnd::rank(uint symbol, size_t pos) const {
		return -1;
	}
	size_t SequenceLZEnd::select(uint symbol, size_t j) const{
		return -1;
	}
	uint SequenceLZEnd::access(size_t pos) const{
		return -1;
	}
	/**
	 * In case of sigma==2, r will contain the rank0(pos).
	 * */
	uint SequenceLZEnd::access(size_t pos, uint &r) const{
		return -1;
	}

	uint SequenceLZEnd::extract(uint start, uint l, uint *&buffer){
		assert(start+l<=length);
		if (start+l>length) return -1;
		if (l<=0) return 0;
		uint end = start+l-1;
		uint p = endings->rank1(end-1);
		if (endings->access(end)){
			uint r = extract(start,l-1,buffer);
			buffer[0]=trailing[p];
			buffer++;
			return 0;
			//output trailing[p];
		}else{
			uint pos = endings->select1(p)+1;
			if (start<pos){
				extract(start,pos-start,buffer);
				start = pos;
				l = end-start+1;
			}
			return extract(endings->select1((*refs)[p]+1)-l+1,l,buffer);
		}
		return 0;
	}

	size_t SequenceLZEnd::getSize() const{
		size_t total = sizeof(SequenceLZEnd);
		total+=endings->getSize();
		total+=z*sizeof(uchar);
		total+=refs->getSize();
		return total;
	}

	void SequenceLZEnd::save(ofstream & fp) const{
		uint r = SEQUENCE_LZEND_HDR;
		saveValue(fp, r);
		r = SEQUENCE_LZEND_BASIC_HDR;
		saveValue(fp,r);
		saveValue(fp,length);
		saveValue(fp, max_v);
		saveValue(fp, z);
		endings->save(fp);
		refs->save(fp);
		saveValue(fp, trailing, z);
		saveValue(fp, dicSize);
		saveValue(fp, dic, dicSize);
		saveValue(fp, invDic, dicSize);
	}
	
	Sequence * SequenceLZEnd::load(ifstream & fp){
		uint rd = loadValue<uint>(fp);
		if(rd!=SEQUENCE_LZEND_HDR) return NULL;
		rd = loadValue<uint>(fp);
		size_t pos = fp.tellg();
		fp.seekg(pos-sizeof(uint),ios::beg);
		switch(rd){
			case SEQUENCE_LZEND_BASIC_HDR: break;
			case SEQUENCE_LZEND_BASE_HDR: return SequenceLZEndBase::load(fp);
			return nullptr;
		}
		//load lzend basic
		rd = loadValue<uint>(fp);
		SequenceLZEnd *ret = new SequenceLZEnd();
		ret->length = loadValue<size_t>(fp);
		ret->max_v = loadValue<uint>(fp);
		ret->z = loadValue<uint>(fp);
		cerr << "z: " << ret->z << endl;
		ret->endings = BitSequence::load(fp);
		ret->refs = new Array(fp);
		ret->trailing = loadValue<uchar>(fp,ret->z);
		ret->dicSize = loadValue<uint>(fp);
		ret->dic = loadValue<uint>(fp,ret->dicSize);
		ret->invDic = loadValue<uint>(fp,ret->dicSize);
		return ret;

	}

	void SequenceLZEnd::printInput(uint *input, fmi* fm, uint limit){
		
		for (uint i=0;i<limit-1;i++){
			cerr << (char)invDic[input[i]]; 
		}
		cerr << (char)invDic[input[limit-1]]<<endl;
		if (!fm) return;
		if (fm->getSA()){
			for (uint i=0;i<limit;i++){
				cerr << "A["<<i<<"]="<<fm->getSA()[i] << ","; 
			}
			cerr << "A["<<limit<<"]="<<fm->getSA()[limit] << endl; 
		}
		if (fm->getInvSA()){
			for (uint i=0;i<limit;i++){
				cerr << "iA["<<i<<"]=" <<fm->getInvSA()[i] << ","; 
			}
			cerr << "iA["<<limit<<"]=" <<fm->getInvSA()[limit] << endl; 
		}
		
	}

	void SequenceLZEnd::compactDictionary(uint *input, std::tuple<uint,uint,uint> *dic, uint *bmpEndings){
		/*
			Why length and not length-1 since createEmptyBitmap(bmpEndings,length+1)?
			The last phrase that is marked in the bitmap is at position length (that's
			the reason for lenght+1 en createEmptyBitmap). However, that phrase is not going
			to be reference by nobody since it is the last one --> We do not net to consider 
			that mark (that we know that is at position length). Thus, we simply index the 
			bitmap but until pos length (that is, from 0 to length-1).
		*/

		endings = new BitSequenceRRR(bmpEndings,length+1,32);
		assert(endings->rank1(endings->getLength())==z);

		phraseLengths = new uint[z];
		uint *references = new uint[z];
		trailing = new uchar[z];
		for (uint i=0;i<z;i++){
			references[i]=endings->rank1(std::get<0>(dic[i]))-1;
			phraseLengths[i]	=std::get<1>(dic[i]);
			trailing[i]	=std::get<2>(dic[i]);
		}
		refs = new Array(references,z);
		assert(testParsing(input));
		delete [] references;
	}

	bool SequenceLZEnd::testParsing(uint *input){

		uint nphrases = endings->rank1(endings->getLength()-1)-1;
		for (uint i=0;i<nphrases;i++){
			uint posPhrase = endings->select1(i+1);
			assert(trailing[i]==(input[posPhrase]-1));
			uint posRef = endings->select1((*refs)[i]+1);
			for (uint j=0;j<phraseLengths[i];j++){
				if (posPhrase==0)
					cerr << endl;
				if (!(input[posPhrase-1]==input[posRef]))
					cerr << endl;
				assert(input[posPhrase-1]==input[posRef]);
				posRef--;
				posPhrase--;
			}
		}
		return true;
	}
	
	void SequenceLZEnd::computePhraseLengths(BitSequence *bitmap){
		phraseLengths = new uint[z];
		uint posPrev=0;
		for (uint i=0;i<z;i++){
			uint pos = bitmap->select1(i+1);
			//pos-posPrev+1 but we are skipping the trailing sequence --> add -1 --> pos-posPrev
			phraseLengths[i] = pos-posPrev;
			posPrev=pos+1;
		}
	}

	uint SequenceLZEnd::sampleDictionary(uint *input, BitSequence *marks, uint **&_samples){
		_samples=NULL;
		return 0;
	}


	uint SequenceLZEnd::compressDictionary(uint *input, uint ** samples, uint ss){
		return 0;
	}

	uint SequenceLZEnd::addImprovements(uint *input){
		return 0;
	}

	bool SequenceLZEnd::testCompressDictionary(uint nsamples, uint nssamples, uint **samplesOriginal, uint **_samples, uint **_ssamples, uint ss){

		for (uint i=0;i<nsamples;i++){
			for (uint j=0;j<=max_v;j++){
				assert(samplesOriginal[j][i]==(_samples[j][i]+_ssamples[j][i/ss]));
			}
		}
		return true;
	}
};
