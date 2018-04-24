/* SequenceLZEendBase.cpp
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
#include <sequence/SequenceLZEndBase.h>


namespace cds_static
{	

	SequenceLZEndBase::SequenceLZEndBase():SequenceLZEnd(){}
	
	SequenceLZEndBase::~SequenceLZEndBase(){
		delete base;
		delete explicity;
		delete pointersBase;
		for (uint i=0;i<=max_v;i++)
			delete samples[i];
		delete [] samples;
	}

	/*the input string $ terminated. The length of that string, including the $, is n -->s
	the length of the original string is n-1. The alphabet must contain contiguous 
	symbols in [0,sigma)*/
	SequenceLZEndBase::SequenceLZEndBase(uint *_input, size_t n, uint *_dic,  BitSequenceBuilder *bsb, SequenceBuilder *sb, LZBaseStrategy *_strategy, uint _sampleRate,  uint supersampleRate, bool del):SequenceLZEnd(){
		length = n-1;
		sampleRate = _sampleRate;
		seqBuilder = sb;
		bitSequenceBuilder = bsb;
		input_seq = _input;
		this->strategy = _strategy;
		ss_rate = supersampleRate;
		build(_input,n,_dic,del,nullptr);
	}

	SequenceLZEndBase::SequenceLZEndBase(uint *_input, size_t n, uint *_dic,  Array *base, BitSequenceBuilder *bsb, SequenceBuilder *sb, uint _beta, uint _sampleRate,  uint supersampleRate,  bool del):SequenceLZEnd(){
		length = n-1;
		sampleRate = _sampleRate;
		beta = _beta;
		strategy=nullptr;
		seqBuilder = sb;
		bitSequenceBuilder = bsb;
		input_seq = _input;
		ss_rate = supersampleRate;
		build(_input,n,_dic,del,base);
	}

	SequenceLZEndBase::SequenceLZEndBase(uint *_input, size_t n, BitSequenceBuilder *bsb, SequenceBuilder *sb,  LZBaseStrategy *_strategy,  uint _sampleRate, uint _supersampleRate, SequenceLZEnd *parsed, bool del){
	
		length = n-1;
		sampleRate = _sampleRate;
		seqBuilder = sb;
		bitSequenceBuilder = bsb;
		input_seq = _input;
		this->strategy = _strategy;
		ss_rate = _supersampleRate;
		cerr << "z: " << parsed->getZ() << endl;
		build(_input,n,parsed,del);


		uint *ends = new uint[z];
		for (uint i=0;i<z;i++){
			ends[i] = endings->select1(i+1);
		}
		cerr << "writing endings to disk" << endl;
		ofstream of("tmp.deltas");
		of.write((char*)ends,sizeof(uint)*z);
		of.close();
		// BitSequence *new_end = bitSequenceBuilder->build(ends,z);
		// delete endings;
		// endings = new_end;
		// cer 
	}

	inline uint SequenceLZEndBase::accessSample(size_t pos, uint symbol, uint &s, uint &sample_position, uint &index_to) const{
		s = rulesUpTo(pos-1);
		uint samp;//,ss_pos;	
		samp = s/sampleRate;
		index_to = min(z,(samp+1)*sampleRate);
		//ss_pos = samp / ss_rate;
		sample_position = getPointerRule(index_to);
		index_to--;
		// return ssamples[symbol]->access(ss_pos)+samples[symbol]->access(samp);
		return samples[symbol]->access(samp);
	}

	inline uint SequenceLZEndBase::accessSampleOnN(size_t pos, uint symbol, uint &s, uint &pos_sample, uint &pos_end, uint &index_end) const{
		uint samp;//,ss_pos;
		s = rulesUpTo(pos-1);
		samp = pos / sampleRate;
		pos_sample = min((uint)length-1,(samp+1)*sampleRate-1);
		// ss_pos = samp / ss_rate;
		index_end = rulesUpTo(pos_sample-1);
		pos_end = getPointerRule(index_end+1);
		//return ssamples[symbol]->access(ss_pos)+samples[symbol]->access(samp);
		return samples[symbol]->access(samp);
	}


	size_t SequenceLZEndBase::rank(uint symbol, size_t pos) const {

		#if LZ_SAMPLE_C
			uint phrase_index;
			//TODO remove this +2
			uint end_limit_index;
			uint to, index_to;
			uint acum = accessSample(pos, symbol,phrase_index, to, index_to);
			//p is the position where the sample is
			//phrase_index: index of the frase that contains pos
			if (pos==length) return acum;
			return interanlRank(symbol, pos+1, index_to, to, acum, phrase_index, false);
		#else
			uint phrase_index;
			uint pos_sample,pos_end,index_end;

			uint acum = accessSampleOnN(pos, symbol,phrase_index, pos_sample, pos_end, index_end);
			//p is the position where the sample is
			//phrase_index: index of the frase that contains pos
			if (pos==length) return acum;
			return interanlRankOnN(symbol, pos+1, index_end, pos_end, pos_sample, acum, phrase_index,true);
		#endif
	}


	inline size_t SequenceLZEndBase::interanlRank(uint symbol, size_t pos, uint &end, uint &to, uint &acum, uint phrase_pos_index, bool verbose) const {
		if (pos>to) return acum;

		uint explicity_rank0_end =explicity->rank0(end);
		while(to>pos){
			if (explicity->access(end)){
				uint rule_limit_explicit_index = 0;
				uint limitRunExplicit = 0;
				if (explicity_rank0_end){
					rule_limit_explicit_index = explicity->select0(explicity_rank0_end);//at end there is a one -->explicit->rank0(end) is the number of zeros preceeding end. Select to obtain the rule number
					limitRunExplicit = getPointerRule(rule_limit_explicit_index+1);//from the rule number we obtain its position.
				}
				uint offset = to-(max(limitRunExplicit+1,(uint)pos))+1;//offset is how we have to substract to posBase to obtain the lowerLimit for the range query. Note the lower limit has to be a position before the inferior limit of the range.
				uint pBase = pointersBase->access(end-explicity_rank0_end);//ending position in the base sequence of this phrase (it is stored explicitly)
				uint pos_ref= pBase - offset;
				acum -= (base->rank(symbol,pBase)-base->rank(symbol,pos_ref));
				to -=offset;//update the upperLimit
				end = rule_limit_explicit_index;//we continue processing the previous rule to the left of end that is marked as not explicit.
			}else{
				explicity_rank0_end--;//convert explic_rank0_end into an index to access refs
				assert(endings->rank1(to)==(end+1));
				uint ref_index = (*refs)[explicity_rank0_end]; //access the refs vector (contains a pointer to the source of the phrase (the index of the 1 in endings))
				if (trailing[explicity_rank0_end]==symbol)acum--;
				uint newTo = getPointerRule(ref_index+1); //the upper limit of the search (+1 because ref_index is an index). Points to the ending point of the source (the 1's at ending mark the end of the source)
				uint prevEnding = getPointerRule(end);//pevEnding is end-1 --> since end is an index, to use it in select1 we have to add +1 --> end-1+1=end
				size_t offset = newTo - (to-1-max(prevEnding+1,(uint)pos)); //prevEnding+1 the position "prevEnding" belongs to the previous frase (is the select1 of the previous end)
				interanlRank(symbol, offset, ref_index, newTo, acum, end-1,verbose);
				to = prevEnding;//set the new upperLimit to the lowerLimit of the search carried out by the preceeding line
				end--;//we move a rule to the left since this one is not explicit and those not explict have to be processed one by one
			}
		}
		if (pos==to){
			//assert((end-explicity_rank0_end+1)==explicity->rank1(end));
			uint explicity_rank1_end_index = end-explicity_rank0_end;
			if (explicity->access(end)){
				if (base->access(pointersBase->access(explicity_rank1_end_index))==symbol) acum--;
			}else{
				if (trailing[explicity_rank0_end-1]==symbol)acum--;
			}
		}
		return acum;
}
//it is no currently working
inline size_t SequenceLZEndBase::interanlRankOnN(uint symbol, size_t pos, uint &end, uint pos_end, uint &to, uint &acum, uint phrase_pos_index,bool first) const {
		if (pos>to) return acum;
		
		uint explicity_rank0_end =explicity->rank0(end);
		while((first) || to>pos){
		
			if (explicity->access(end)){
				first = false;
				uint rule_limit_explicit_index = 0;
				uint limitRunExplicit = 0;
				if (explicity_rank0_end){
					rule_limit_explicit_index = explicity->select0(explicity_rank0_end);//at end there is a one -->explicit->rank0(end) is the number of zeros preceeding end. Select to obtain the rule number
					limitRunExplicit = getPointerRule(rule_limit_explicit_index+1);//from the rule number we obtain its position.
				}
				uint offset_to = pos_end-to;
				uint offset = to-(max(limitRunExplicit+1,(uint)pos))+1;//offset is how we have to substract to posBase to obtain the lowerLimit for the range query. Note the lower limit has to be a position before the inferior limit of the range.
				uint pBase = pointersBase->access(end-explicity_rank0_end)-offset_to;//ending position in the base sequence of this phrase (it is stored explicitly)
				uint pos_ref= pBase - offset;
				acum -= (base->rank(symbol,pBase)-base->rank(symbol,pos_ref));
				to -=offset;//update the upperLimit
				end = rule_limit_explicit_index;//we continue processing the previous rule to the left of end that is marked as not explicit.
				pos_end = to;
			}else{
				explicity_rank0_end--;//convert explic_rank0_end into an index to access refs
				
				uint ref_index = (*refs)[explicity_rank0_end]; //access the refs vector (contains a pointer to the source of the phrase (the index of the 1 in endings))
				uint offset_to=pos_end-to-1;
				uint adjust=0;
				first=true;
				if (to==pos_end){
					if (trailing[explicity_rank0_end]==symbol)acum--;
					adjust=offset_to=1;
					first=false;
				}
				
				pos_end =  getPointerRule(ref_index+1);
				uint newTo = pos_end-offset_to; //the upper limit of the search (+1 because ref_index is an index). Points to the ending point of the source (the 1's at ending mark the end of the source)
				uint prevEnding = getPointerRule(end);//pevEnding is end-1 --> since end is an index, to use it in select1 we have to add +1 --> end-1+1=end
				size_t offset = newTo - (to-max(prevEnding+1,(uint)pos))+adjust; //prevEnding+1 the position "prevEnding" belongs to the previous frase (is the select1 of the previous end)
				
				interanlRankOnN(symbol, offset, ref_index, pos_end, newTo, acum, end-1,first);
				
				to = prevEnding;//set the new upperLimit to the lowerLimit of the search carried out by the preceeding line
				end--;//we move a rule to the left since this one is not explicit and those not explict have to be processed one by one
				pos_end=to;
			}
		}
		if (pos==to){
			//assert((end-explicity_rank0_end+1)==explicity->rank1(end));
			uint explicity_rank1_end_index = end-explicity_rank0_end;
			if (explicity->access(end)){
				if (base->access(pointersBase->access(explicity_rank1_end_index))==symbol) acum--;
			}else{
				if (trailing[explicity_rank0_end-1]==symbol)acum--;
			}
		}
		return acum;
}

	uint SequenceLZEndBase::extract(uint start, uint l, uint *&buffer){
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


	size_t SequenceLZEndBase::select(uint symbol, size_t j) const{
		return -1;
	}

	uint SequenceLZEndBase::access(size_t pos) const{
		assert(pos<length);
		//access the rule index (on 0..nrules)
		uint rule = rulesUpTo(pos-1);
		//the accessed symbols is at the end of a block
		if (endings->access(pos)){
			//check if the rules is stored explicitly
			uint r = explicity->rank1(rule);//r is the r-th marked rule (starting at 0)
			if (explicity->access(rule)){
				return base->access(pointersBase->access(r-1));
			}else{
				//return the trailling char of the rule-r (explicity->rank0)
				uint r0 = rule-r; //do not add +1 because is an index to access trailing
				return trailing[r0];
				
			}
		}else{
			if (explicity->access(rule)){
				uint r = explicity->rank1(rule);
				pos = getPointerRule(rule+1)-pos;
				return base->access(pointersBase->access(r-1)-pos);
			}
			uint r0 = explicity->rank0(rule);
			pos = getPointerRule((*refs)[r0-1]+1) - (getPointerRule(rule+1)-1-pos);
			return access(pos);
			
		}
	}
	/**
	 * In case of sigma==2, r will contain the rank0(pos).
	 * */
	uint SequenceLZEndBase::access(size_t pos, uint &r) const{
		return -1;
	}


	size_t SequenceLZEndBase::getSize() const{
		size_t total = sizeof(SequenceLZEndBase);
		total+=base->getSize();
		#if ENDING_BITMAP
			total+=endings->getSize();
		#else
			total+=endingsArray->getSize();
		#endif
		total+=explicity->getSize();
		total+=refs->getSize();
		total+=pointersBase->getSize();
		total+=sizeof(uchar)*nRulesNotExplicit;
		total+=sizeof(DirectAccess)*(max_v+1);
		for (uint i=0;i<=max_v;i++)
			total+=samples[i]->getSize();
		
		return getSizeVerbatim();
	}


	size_t SequenceLZEndBase::analyzeSampling() const{
		double ten = length*1.0/10;
		size_t pos = (size_t) ten;
		cerr << "length: " << length << ", endings length: " << endings->getLength() << endl;
		size_t prev = 0;
		for (uint i=1;i<=10;i++){
			pos = min((size_t)pos,endings->getLength()-1);
			size_t next = endings->rank1(pos);
			cerr << i*10 << " % are " <<  next-prev<< " rules" << endl;
			prev=next;
			pos+=ten;
		}	
		return 0;
	}


	size_t SequenceLZEndBase::getSizeVerbatim() const{
		size_t total = sizeof(SequenceLZEndBase);
		analyzeSampling();
		cerr << "Base length: " << base->getLength() << endl;
		total+=base->getSize();
		size_t ends;
		cerr << "Bitmap: " << endings->getSize()*8.0 / length << endl;
		cerr << "Array: " << endingsArray->getSize()*8.0 / length << endl;
		#if ENDING_BITMAP
			ends = endings->getSize();
			total+=endings->getSize();
		#else
			ends = endingsArray->getSize();
			total+=endingsArray->getSize();
		#endif

		total+=explicity->getSize();
		total+=refs->getSize();
		total+=pointersBase->getSize();
		cerr << "nRulesNotExplicit: " << nRulesNotExplicit << endl;
		total+=bits(max_v+1)*nRulesNotExplicit/8;
		size_t s;
		s=sizeof(DAC)*(max_v+1);
		cerr << "max_v: " << max_v << endl;
		if (max_v==1){
			s+=samples[0]->getSize();
		}else{
			for (uint i=0;i<=max_v;i++)
				s+=samples[i]->getSize();
		}
		total+=s;

		const uint l = 8;
		string meanings[l] = {string("base:\t\t"),string("endings:\t"),string("explicity:\t"),string("refs:\t\t"),string("pBase:\t\t"),string("trail:\t\t"),string("samples:\t")};
		size_t parts[l]={base->getSize(), ends,explicity->getSize(),refs->getSize(),pointersBase->getSize(),bits(max_v+1)*nRulesNotExplicit/8, s};
		for (uint i=0;i<l;i++){
			double percent = parts[i]*1.0/total*100;
			uint stars = (uint)percent;
			cerr << meanings[i];
			for (uint j=0;j<stars;j++){
				cerr << "*";
			}	
			if (stars) cerr << "(" << parts[i]*8.0/length << ")";
			cerr <<endl;
		}
		return total;
	}

	void SequenceLZEndBase::save(ofstream & fp) const{
		uint r = SEQUENCE_LZEND_HDR;
		saveValue(fp, r);
		r = SEQUENCE_LZEND_BASE_HDR;
		saveValue(fp, r);
		saveValue(fp, length);
		saveValue(fp, z);
		saveValue(fp, beta);
		saveValue(fp, nsamples);
		saveValue(fp, nRulesNotExplicit);
		saveValue(fp, max_v);
		saveValue(fp, ss_rate);
		
		saveValue(fp, sampleRate);
		saveValue(fp, trailing, nRulesNotExplicit);
		base->save(fp);
		endings->save(fp);
		endingsArray->save(fp);
		explicity->save(fp);
		refs->save(fp);
		pointersBase->save(fp);
		for (uint i=0;i<=max_v;i++){
			samples[i]->save(fp);
		}
		
		// for (uint i=0;i<=max_v;i++){
		// 	ssamples[i]->save(fp);
		// }
	}
	
	Sequence * SequenceLZEndBase::load(ifstream & fp){
		uint rd = loadValue<uint>(fp);
		if(rd!=SEQUENCE_LZEND_BASE_HDR) return NULL;
		SequenceLZEndBase *ret = new SequenceLZEndBase();
		ret->length = loadValue<size_t>(fp);
		ret->z = loadValue<uint>(fp);
		ret->beta = loadValue<uint>(fp);
		ret->nsamples = loadValue<uint>(fp);
		
		ret->nRulesNotExplicit = loadValue<uint>(fp);
		ret->max_v = loadValue<uint>(fp);
		ret->ss_rate = loadValue<uint>(fp);
		
		ret->sampleRate = loadValue<uint>(fp);
		ret->trailing = loadValue<uchar>(fp,ret->nRulesNotExplicit);
		ret->base = Sequence::load(fp);
		ret->endings = BitSequence::load(fp);
		ret->endingsArray = new Array(fp);
		ret->explicity = BitSequence::load(fp);
		ret->refs = new Array(fp);
		ret->pointersBase = DAC::load(fp);
		ret->samples = new DirectAccess*[ret->max_v+1];
		for (uint i=0;i<=ret->max_v;i++){
			ret->samples[i] = DirectAccess::load(fp);
		}
		return ret;
	}


	inline size_t SequenceLZEndBase::getPointerRule(size_t rank_one) const{
		#ifdef ENDINGS_BITMAP
			return endings->select1(rank_one);
		#else
			return (*endingsArray)[rank_one-1];
		#endif
	
		// return endings->select1(index);
	}
	
	inline size_t SequenceLZEndBase::rulesUpTo(size_t pos) const{
		#ifdef ENDINGS_BITMAP
			return endings->rank1(pos);
		#else
			//uint proba[7] = {0,1,4,6,7,9,10};
			if (pos+1==0) return 0;
			uint l,r,p;
			l=0;r=z-1;
			uint v;
			while(l<=r){
				p=(l+r)/2;
				//v = proba[p];
				v = (*endingsArray)[p];
				if (v>pos){
					r = p-1;
				}else if (v<pos){
					l = p+1;
				}else{
					return p+1;
				}
			}
			if (v>pos) return p;
			return p+1;
		#endif
	}
	uint SequenceLZEndBase::addImprovements(uint *input){
		uint ebbl;
		if (strategy==nullptr){
			cerr << "The strategy to select the base sequence is null" << endl;
			abort();
		}
		strategy->computeBase(input, length, this);

		// if (ba==BASE_LENGTH){
		// 	ebbl = extractBaseByLength(input);
		// }else if (ba==BASE_C){
		// 	ebbl = extractBaseByHeight(input);
		// }else{cs unknown.\n\tProgram will close!"<<endl;
		// 	abort();
		// }

		nRulesNotExplicit = explicity->rank0(explicity->getLength()-1);
		uint *newRefs = new uint[nRulesNotExplicit];
		uchar *newTrailing = new uchar[nRulesNotExplicit+1];
		uchar maxTrailing=0;
		for (uint i=0;i<nRulesNotExplicit;i++){
			newRefs[i] = (*refs)[explicity->select0(i+1)];
			newTrailing[i] = trailing[explicity->select0(i+1)];
			maxTrailing = max(maxTrailing,newTrailing[i]);
		}

		delete refs;
		refs = new Array(newRefs,nRulesNotExplicit);
		delete [] trailing;
		trailing = newTrailing;
		return ebbl;
	}


	uint SequenceLZEndBase::extractBaseByLength(uint *input){
		uint *sequence = new uint[length];
		uint pointerBase=0;
		uint *bmpExplicits;
		createEmptyBitmap(&bmpExplicits,z+1);
		uint *pBase = new uint[z];
		uint nExplicit=0;
		for (uint i=0;i<z;i++){
			if (phraseLengths[i]>beta) continue;
			if (phraseLengths[i]==0){
				bitset(bmpExplicits,i);
				sequence[pointerBase]=trailing[i];
				pBase[nExplicit++]=pointerBase;
				pointerBase++; 
			}else{
				uint start, end;
				end = endings->select1((*refs)[i]+1);
				start = end-phraseLengths[i]+1;
				
				bitset(bmpExplicits,i);
				for (uint j=start;j<=end;j++){
					sequence[pointerBase++]=input[j]-1;
				}
				sequence[pointerBase]=trailing[i];
				pBase[nExplicit++]=pointerBase;
				pointerBase++;
			}
		}
		//TODO...the symbols must be contiguous...
		bitset(bmpExplicits,z);
		explicity = new BitSequenceRRR(bmpExplicits,z+1,32);
		pBase[nExplicit]=pointerBase;
		// for (uint i=0;i<=nExplicit;i++){
		// 	pBase[i]=pBase[i+1]-1;//make it to point to the tail of the sequence, not to the head
		// }
		pointersBase = new DAC(pBase,nExplicit);
		uint ngaps,*gaps=nullptr;
		getGaps(sequence,(size_t)pointerBase,ngaps,gaps);
		if (gaps){
			//padding the sequence
			for (uint i=0;i<ngaps;i++)
				sequence[pointerBase++]=gaps[i];
		}
		base = seqBuilder->build(sequence,pointerBase);

		// Sequence *seq_base = new SequenceLZEnd(sequence,pointerBase,dic, false);
		// cerr << "seq_base: " << seq_base->getSize()*8.0 / pointerBase << endl;
		pointerBase-=ngaps;
		assert(testExtractPhrases(input));
		delete [] gaps;
		delete [] bmpExplicits;
		delete [] sequence;
		delete [] pBase;
		return 0;
	}


	uint SequenceLZEndBase::extractBaseByHeight(uint *input){
		uint *C = new uint[length+1];
		uint pointerBase=0;
		uint *bmpExplicits;
		createEmptyBitmap(&bmpExplicits,z+1);
		uint *pBase = new uint[z+1];
		uint nExplicit=0;
		for (uint i=0;i<length;i++) C[i]=0;
		
		uint prevPos = 0;
		uint nrules = z;
		
		uint *maxC_rule = new uint[nrules];

		uint pos=0;
		for (uint i=1;i<=nrules;i++){
			if (phraseLengths[i-1]==0){
				maxC_rule[i-1]=0;
				assert(pos<length);
				C[pos]=0;
				pos++;
			}else{
				pos = getPointerRule(i);
				assert(pos<=length);
				C[pos]=0;
				maxC_rule[i-1]=0;
				int j= pos-1;
				uint ref = getPointerRule((*refs)[i-1]+1);
				for (int d=0;j>prevPos;j--,d++){
					C[--pos] = C[ref-d]+1;
					maxC_rule[i-1]=max(maxC_rule[i-1],C[pos]);
					if (maxC_rule[i-1]>=beta) break;
				}
				if (maxC_rule[i-1]>=beta){
					pos = getPointerRule(i);
					for (int j=pos;j>prevPos;j--)
						C[j]=0;
				}

			}
			prevPos = getPointerRule(i);
		}
		uint toExplicit=0;
		uint lengthExplicit = 0;
		double avgLenght=0.0;
		for (uint i=0;i<nrules;i++){
			if (maxC_rule[i]>=beta ){
				toExplicit++;
				lengthExplicit+=(phraseLengths[i]+1);
			}
		}
		cerr << "avgLength: " << 1.0*lengthExplicit/toExplicit << endl;
		cerr << "toExplicit: " << toExplicit << endl;
		cerr << "length base: " << lengthExplicit << endl;
		cerr << "%base out of total: " << 1.0 * lengthExplicit / length << endl;
		uint *sequence = new uint[lengthExplicit+255];
		for (uint i=0;i<nrules;i++){

			if (phraseLengths[i]>0 && maxC_rule[i]<beta) continue;
			if (phraseLengths[i]==0){
				bitset(bmpExplicits,i);
				sequence[pointerBase]=trailing[i];
				pBase[nExplicit++]=pointerBase;
				pointerBase++; 
			}else{
				uint start, end;
				end = getPointerRule((*refs)[i]+1);
				start = end-phraseLengths[i]+1;
				
				bitset(bmpExplicits,i);
				for (uint j=start;j<=end;j++){
					sequence[pointerBase++]=input[j]-1;
				}
				sequence[pointerBase]=trailing[i];
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
		cerr << "nExplicit: " << nExplicit << ", z: " << z << endl;
		explicity = new BitSequenceRG(bmpExplicits,z,32);
		assert(getPointerRule(endings->rank1(endings->getLength()-1))==length);
		assert(nExplicit<=z);

		pointersBase = new DAC(pBase,nExplicit);
		uint ngaps,*gaps=nullptr;
		getGaps(sequence,(size_t)pointerBase,ngaps,gaps);
		if (gaps){
			//padding the sequence
			for (uint i=0;i<ngaps;i++)
				sequence[pointerBase++]=gaps[i];
		}

		// Sequence *seq_base = new SequenceLZEnd(sequence,pointerBase,dic, false);
		// cerr << "seq_base: " << seq_base->getSize()*8.0 / pointerBase << endl;

		base = seqBuilder->build(sequence,pointerBase);

		pointerBase-=ngaps;
		assert(testExtractPhrases(input));
		delete [] gaps;
		delete [] bmpExplicits;
		delete [] sequence;
		delete [] pBase;
		return 0;
	}



	LZBaseStrategy* SequenceLZEndBase::getStrategy(){
		return strategy;
	}

	LZBaseStrategy* SequenceLZEndBase::setStreategy(LZBaseStrategy *strategy){
		this->strategy = strategy;
		return this->strategy;
	}

	bool SequenceLZEndBase::testExtractPhrases(uint *input){
		uint nphrase = endings->rank1(endings->getLength()-1)-1;
		for (uint i=0;i<nphrase;i++){
			
			if (explicity->access(i)){
				uint finPos = getPointerRule(i+1);
				uint indexBase = explicity->rank1(i)-1;
				uint pBase = pointersBase->access(indexBase);
				for (uint j=0;j<=phraseLengths[i];j++){
					assert((input[finPos-j]-1)==base->access(pBase-j));
				}
			}else{
				uint finPos = getPointerRule(i+1);
				assert((input[finPos]-1)==trailing[i]);
				uint redirect = getPointerRule((*refs)[i]+1);
				for (uint j=0;j<phraseLengths[i];j++){
					assert( (input[redirect-j])==input[finPos-j-1]);
				}
				//cerr << "rule: " << i << endl;
				uint counter = phraseLengths[i];
				uint rule_index = (*refs)[i];
				uint pos = redirect;
				if (!explicity->access(endings->rank1(pos)-1)) continue;
				uint pBase = pointersBase->access(explicity->rank1(rule_index)-1);
				int j=0;
				while(counter>0){
					if (endings->access(pos) && !explicity->access(endings->rank1(pos)-1))
						break;
					// if (i==135){
					// 	cerr << "pos: " << pos << ", inp: " <<input[pos]-1 << endl;
					// }
					if(!((input[pos]-1)==base->access(pBase-j)))
						cerr << endl;
					assert((input[pos]-1)==base->access(pBase-j));
					j++;
					pos--;
					counter--;
				}
				
			}
		}
		return true;
	}

	uint SequenceLZEndBase::sampleDictionary(uint *input, BitSequence *marks, uint **&_samples){

		//if NOT def LZ_SAMPLE_C --> SAMPLE ON N
		#if LZ_SAMPLE_C == 0
			return sampleDictionaryOnN(input, marks, _samples);
		#endif
		//why -1? That max_v is including the $, for which we do not want to store _samples obviously. 
		//Thus, the actual identifier of input[i] is input[i-1]. 
		max_v = getMaxV(input,length)-1;
		uint nmarks = z;
		assert(marks->rank1(marks->getLength()-1)==z);
		nsamples = z/sampleRate+((z%sampleRate)?1:0);
		_samples = new uint*[nsamples];



		uint *counters = new uint[max_v+1];
		for (uint i=0;i<=max_v;i++){
			counters[i]=0;
		}
		for (uint i=0;i<nsamples;i++){
			_samples[i] = new uint[max_v+1];
		}
		uint pointerSamples=0;
		for (uint i=0;i<length;i++){
			//-1? already explained
			counters[input[i]-1]++;
			if(marks->access(i) && (marks->rank1(i)%sampleRate)==0){
				assert(pointerSamples<nsamples);
				for (uint j=0;j<=max_v;j++){
					_samples[pointerSamples][j]=counters[j];
				}
				pointerSamples++;
				assert((marks->rank1(i)/sampleRate)==pointerSamples);
			}
		}
		for (uint j=0;j<=max_v;j++){
			_samples[pointerSamples][j]=counters[j];
		}
		
		_samples[pointerSamples][0]++;
		pointerSamples++;

		assert(pointerSamples==nsamples);
		//copy the endings in an array
		uint *ending_positions = new uint[z];
		for (uint i=0;i<z;i++){
			ending_positions[i]=endings->select1(i+1);
		}
		endingsArray = new Array(ending_positions,z);

		// for (uint i=0;i<10;i++){
		// 	cerr << "ending: " << rulesUpTo(i) << endl;
		// 	//assert(endings->rank1(i)==rulesUpTo(i));
		// }
		for (uint i=0;i<z;i++){
			// cerr << "ending: " << rulesUpTo(i) << endl;
			assert(endings->rank1(i)==rulesUpTo(i));
		}
		delete [] counters;
		return nsamples;
	}


	uint SequenceLZEndBase::sampleDictionaryOnN(uint *input, BitSequence *marks, uint **&_samples){
		//why -1? That max_v is including the $, for which we do not want to store _samples obviously. 
		//Thus, the actual identifier of input[i] is input[i-1]. 
		max_v = getMaxV(input,length)-1;
		nsamples = length/sampleRate+((length%sampleRate)?1:0);
		_samples = new uint*[nsamples];

		uint *counters = new uint[max_v+1];
		for (uint i=0;i<=max_v;i++){
			counters[i]=0;
		}
		for (uint i=0;i<nsamples;i++){
			_samples[i] = new uint[max_v+1];
		}
		uint pointerSamples=0;
		//-1? already explained
		// counters[input[0]-1]++;
		counters[input[0]-1]++;
		for (uint i=1;i<length;i++){
			//sampleo hasta i-1 cuando es multiplo cd #sampleC (así van sampleC en cada bloque)
			if((i%sampleRate)==0){
				assert(pointerSamples<nsamples);
				for (uint j=0;j<=max_v;j++){
					_samples[pointerSamples][j]=counters[j];
				}
				pointerSamples++;
			}
			counters[input[i]-1]++;
		}
		for (uint j=0;j<=max_v;j++){
			_samples[pointerSamples][j]=counters[j];
		}
		
		_samples[pointerSamples][0]++;
		pointerSamples++;



		//copy the endings in an array
		uint *ending_positions = new uint[z];
		for (uint i=0;i<z;i++){
			ending_positions[i]=marks->select1(i+1);
		}

		endingsArray = new Array(ending_positions,z);
		for (uint i=0;i<z;i++){
			assert(marks->rank1(i)==rulesUpTo(i));
		}
		assert(pointerSamples==nsamples);
		delete [] counters;
		return nsamples;
	}


	uint SequenceLZEndBase::compressDictionary(uint *input, uint **_samples,uint ss){

		samplesVerb = _samples;		
		_samples = new uint*[max_v+1];
		
		for (uint i=0;i<=max_v;i++){
			_samples[i] = new uint[nsamples];
		}
		
		for (uint i=0;i<nsamples;i++){
			for (uint j=0;j<=max_v;j++){
				_samples[j][i]=samplesVerb[i][j];
			}
		}

		
		for (uint i=0;i<nsamples;i++){
			delete [] samplesVerb[i];
		}
		delete [] samplesVerb;
		
		samplesVerb  = _samples;
		

		//DACS only work for large sequences
		samples = new DirectAccess*[max_v+1];
		for (uint i=0;i<=max_v;i++){
			cerr << "i: " << i << endl;
			
			samples[i] = new DACInc(_samples[i],nsamples,ss);
		}

		#ifdef DEBUG
		for (uint i=0;i<nsamples;i++){
			for (uint j=0;j<=max_v;j++){
				assert(_samples[j][i]==samples[j]->access(i));
			}
		}		
		#endif

		return 0;
	}

	// 	inline void  SequenceLZEndBase::solveExplicit(uint symbol, size_t offset, uint phrase_pos_index, uint to, uint phrase_to_index, uint &acum, uint rank1_index_phrase_to_index, bool verbose) const{
	// 	//uint limitRunExplicit = (explicity_rank0_end==0)?0:getPointerRule(explicity->select0(explicity_rank0_end)+1);
	// 	//uint rule_index = explicity->rank1(phrase_to_index);//end-explicity_rank0_end; //number of explicit rules up to end // TODO must come as a parameter!!!!!
	// 	//assert(rank1_index==(rule_index-1));
	// 	uint pBase = pointersBase->access(rank1_index_phrase_to_index);
	// 	uint pos_ref= pBase - offset;
	// 	acum -= (base->rank(symbol,pBase)-base->rank(symbol,pos_ref));
	// 	if (verbose){
	// 		cerr << "\tE"<<endl;
	// 		cerr << "\t";
	// 		for (uint i=pBase;i>pos_ref;i--){
	// 			cerr << base->access(i) << ",";
	// 		}	
	// 		cerr << "\t(Resta):"<<(base->rank(symbol,pBase)-base->rank(symbol,pos_ref))<<endl;
	// 		cerr << endl << "******"<< endl;
	// 		cerr << endl;
	// 	}
	// }

};
