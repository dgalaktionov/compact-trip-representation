/* SequenceRepair.cpp
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


#include <assert.h>
#include <sequence/SequenceRepair.h>
#include <stack>
namespace cds_static
{	
	
	SequenceRepair::SequenceRepair():Sequence(0),sample_r(0){}
	SequenceRepair::SequenceRepair(size_t n):Sequence(n),sample_r(0){}


	void SequenceRepair::build(uint * symbols, size_t n, BitSequenceBuilder * bmb, Mapper * am,  PermutationBuilder *pb, 
						uint samplingC, uint delta, uint _ss_rate, bool deleteSymbols, DictionaryRPBuilder *dicBuilder){
		//bmb->use();
		this->n=n;
		this->am=am;
        sample_r=delta;
		am->use();
		
		for(uint i=0;i<n;i++)
			symbols[i] = am->map(symbols[i]);

		std::cout << "Mapped symbols" << endl;
		uint *PERM,*INVPERM;
		PERM=INVPERM=NULL;
		Repair *rp = new Repair((int*)symbols,n);
		//builds the dictionary using the technique of Tabei et al.
        uint *left, *right;
        int R = rp->getNRules()+rp->getTermsSize();
        left = new uint[rp->getNRules()];
        right = new uint[rp->getNRules()];
        for (long i=0L; i<rp->getNRules();i++){
            left[i] = rp->getLeft(i);
            right[i] = rp->getRight(i);
        }
        std::cout << "Dic bullshit" << endl;
//		dictionary = new DictionaryRP(rp,PERM,pb);
        if (dicBuilder==nullptr) {
            dictionary = new DictionaryRPTabei(left, right, rp->getTermsSize(), R, PERM);
        }else{
            dictionary = dicBuilder->build(left,right,rp->getTermsSize(),R,PERM);
        }
        delete [] left;
        delete [] right;
		//~ cerr << "Dic size: " << dictionary->getSize() << endl;
		//~ cerr << "Nc: " << rp->getNC()*rp->getNTerm() << endl;
		//~ cerr << "Total: " << rp->getNC()*rp->getNTerm()+dictionary->getSize() << endl;
		alph = dictionary->getAlph();
		c = rp->getNC();
		//the dictionary generates a permutation of the symbols -> that permutation is stored in PERM
		//we need store the inverse to restore the original symbols back in case we cannot delete them
		std::cout << "Good so far?" << endl;
		if (!deleteSymbols){
			INVPERM = new uint[dictionary->getNRules()];
			for (uint i=0;i<dictionary->getNRules();i++){
				INVPERM[PERM[i]]=i;
			}
		}
		for (size_t i=0;i<n;i++)
			symbols[i]=PERM[symbols[i]];
			
		max_v=max_value(symbols,n)+1;
		
		//copy the sequence C into an Array of ceil(log(max(C)))
		//we are adding a superfluous rule at the end of the vector Cs
		//this is to avoid to check (during the searches) if we are dealing with the last rule or not
		uint *Cseq = new uint[rp->getNC()+1];
		for (int i=0;i<rp->getNC();i++){
			Cseq[i]=PERM[rp->getC(i)];
		}
		Cseq[rp->getNC()]=PERM[rp->getC(rp->getNC()-1)];
		Cs = new Array(Cseq,rp->getNC()+1UL);
		delete [] Cseq;
		//We have already mapped the rules and the sequence C --> we do not need PERM anymore
		delete [] PERM;
		
		blockSizeC=(1U<<samplingC);
		blogBlockSizeC=bits(blockSizeC-1);
		log_ss_rate = _ss_rate;
		ss_rate = 1U<<log_ss_rate;
		
		uint *rule_lengths;
		std::cout << "Let's go" << endl;
		sampleRules(symbols,n,max_v,delta,rule_lengths);
		std::cout << "Got Sampled rules!" << endl;
		sampleC(symbols,max_v,delta,rule_lengths);
		
		if (!deleteSymbols){
			for(uint i=0;i<n;i++){
				symbols[i] = am->unmap(INVPERM[symbols[i]]);
			}
			delete [] INVPERM;
		}
		//~ delete rp;
		//bmb->unuse();
		this->length = n;						
							
	}

	//TODO factorize this code with the previous
	void SequenceRepair::build(uint * symbols, size_t n, Repair *rp, BitSequenceBuilder * bmb, Mapper * am,  PermutationBuilder *pb, 
						uint samplingC, uint delta, uint _ss_rate, bool deleteSymbols, DictionaryRPBuilder *dicBuilder){
		//bmb->use();
		this->n=n;
		this->am=am;
        this->sample_r = delta;
		am->use();
		
		for(uint i=0;i<n;i++)
			symbols[i] = am->map(symbols[i]);
		
		uint *PERM,*INVPERM;
		PERM=INVPERM=NULL;
        uint *left, *right;
        int R = rp->getNRules()+rp->getTermsSize();
        left = new uint[rp->getNRules()];
        right = new uint[rp->getNRules()];
        for (long i=0L; i<rp->getNRules();i++){
            left[i] = rp->getLeft(i);
            right[i] = rp->getRight(i);
        }
        if (dicBuilder==nullptr) {
            dictionary = new DictionaryRPTabei(left, right, rp->getTermsSize(), R, PERM);
        }else{
            dictionary = dicBuilder->build(left,right,rp->getTermsSize(),R,PERM);
        }
        delete [] left;
        delete [] right;
		// Repair *rp = new Repair((int*)symbols,n);
		//builds the dictionary using the technique of Tabei et al.
//		dictionary = new DictionaryRP(rp,PERM,pb);
		//~ cerr << "Dic size: " << dictionary->getSize() << endl;
		//~ cerr << "Nc: " << rp->getNC()*rp->getNTerm() << endl;
		//~ cerr << "Total: " << rp->getNC()*rp->getNTerm()+dictionary->getSize() << endl;
		alph = dictionary->getAlph();
		c = static_cast<size_t>(rp->getNC());
		//the dictionary generates a permutation of the symbols -> that permutation is stored in PERM
		//we need store the inverse to restore the original symbols back in case we cannot delete them
		
		if (!deleteSymbols){
			INVPERM = new uint[dictionary->getNRules()];
			for (uint i=0;i<dictionary->getNRules();i++){
				INVPERM[PERM[i]]=i;
			}
		}
		for (size_t i=0;i<n;i++)
			symbols[i]=PERM[symbols[i]];
			
		max_v=max_value(symbols,n)+1;
		
		//copy the sequence C into an Array of ceil(log(max(C)))
		//we are adding a superfluous rule at the end of the vector Cs
		//this is to avoid to check (during the searches) if we are dealing with the last rule or not
		uint *Cseq = new uint[rp->getNC()+1];
		for (int i=0;i<rp->getNC();i++){
			Cseq[i]=PERM[rp->getC(i)];
		}
		Cseq[rp->getNC()]=PERM[rp->getC(rp->getNC()-1)];
		Cs = new Array(Cseq, static_cast<size_t>(rp->getNC()+1L));
		delete [] Cseq;
		//We have already mapped the rules and the sequence C --> we do not need PERM anymore
		delete [] PERM;
		
		blockSizeC=(1U<<samplingC);
		blogBlockSizeC=bits(blockSizeC-1);
		log_ss_rate = _ss_rate;
		ss_rate = 1U<<log_ss_rate;
		
		uint *rule_lengths;
		sampleRules(symbols,n,max_v,delta,rule_lengths);
		sampleC(symbols,max_v,delta,rule_lengths);
		
		if (!deleteSymbols){
			for(uint i=0;i<n;i++){
				symbols[i] = am->unmap(INVPERM[symbols[i]]);
			}
			delete [] INVPERM;
		}
		//~ delete rp;
		//bmb->unuse();
		this->length = n;						
							
	}
	uint SequenceRepair::max_value(uint *symbols, size_t n) {
		uint max_v = 0;
		for(size_t i=0;i<n;i++)
			max_v = max(symbols[i],max_v);
		return max_v;
	}

	void SequenceRepair::sampleRules(uint *input, size_t len, uint sigma, uint delta, uint *&lengths) {
		 //we first sample the rules
		auto nRules = dictionary->getNRules();
        lengths = new uint[nRules];
        for (uint i=0;i<nRules;i++)lengths[i]=0;
     
        uint **countersRules = new uint*[sigma];
        for (uint i=0;i<sigma;i++)
			countersRules[i] = new uint[nRules];
		for (uint i=0;i<sigma;i++)
			for (uint j=0;j<nRules;j++)
				countersRules[i][j]=0;
				
        //compute the length and counters for each rule
        for (uint i=0;i<nRules;i++)
			computeRuleParams(i,lengths,countersRules,sigma);

		
		sampleRules(delta,nRules,dictionary->getAlph(),sigma,lengths,countersRules);

	}
	
    /**
     * Returns the length of a rule and stores that length in lengths[rule]. Besides it computes 
     * the counters of symbols of each rule
     * 
     * */
	uint SequenceRepair::computeRuleParams(uint rule, uint *lengths, uint **counters, uint sigma){
		
		if (lengths[rule]) return lengths[rule];
		
		if (rule<alph){
			counters[rule][rule]=1;
			lengths[rule]=1;			
		}else{
			uint left,right,total=0;
			left = dictionary->getLeft(rule);
			right = dictionary->getRight(rule);
			
			if (lengths[left]){
				total=lengths[left];
			}else{
				total=computeRuleParams(left,lengths,counters,sigma);
			}
			for (uint i=0;i<sigma;i++){
				counters[i][rule]=counters[i][left];
			}
			
			if (lengths[right]){
				total+=lengths[right];
			}else{
				total+=computeRuleParams(right,lengths,counters,sigma);
			}
			for (uint i=0;i<sigma;i++){
				counters[i][rule]+=counters[i][right];
			}
			lengths[rule]=total;
		}
		return lengths[rule];
	}
	
	void SequenceRepair::compressRules(uint totalSamples, uint sigma, uint *sampledLengths, uint **sampledCounters){
		bool overwriteVectors=false;
		ruleLengths = compressWithDirectAccess(sampledLengths,totalSamples,overwriteVectors);
		ruleCounters = new DirectAccess*[sigma];
		for (uint i=0;i<sigma;i++){
			//TODO
			/**
			 * If we build a DAC for a vector with all the values equals, the DAC constructor fails
			 * To solve it, by now, I opted for adding a new element at the end of the vector
			 * */
			bool allEqual=true;
			for (uint j=1;j<totalSamples;j++)
				if (sampledCounters[i][j]!=sampledCounters[i][j-1]){
					allEqual=false;
					break;
				}
			if (allEqual){
				uint *newc = new uint[totalSamples+1];
				for (uint j=0;j<totalSamples;j++)
					newc[j] = sampledCounters[i][j];
				newc[totalSamples]=newc[totalSamples-1]+1;
				ruleCounters[i] = compressWithDirectAccess(newc,totalSamples+1,overwriteVectors);
			}else{
//				size_t diff=1;
//				for (;diff<totalSamples;diff++){
//					if (sampledCounters[i][diff-1]!=sampledCounters[i][diff]){
//						break;
//					}
//				}
////				if (diff!=totalSamples){
////					ruleCounters[i] = new ArrayDA(sampledCounters[i], totalSamples);
////				}else {
					ruleCounters[i] = compressWithDirectAccess(sampledCounters[i], totalSamples, overwriteVectors);
//				}
			}
		}
		
	}
	
	void SequenceRepair::sampleRules(uint sample, uint n, uint alph, uint sigma, uint *lengths, uint **counters){
		uint bmp_len = n-alph+1;
		assert(uint_len(bmp_len,1));
		auto n_uints_bmp = uint_len(bmp_len,1);
		uint *bitVector = new uint[n_uints_bmp];
		for (uint i=0;i<n_uints_bmp;i++) bitVector[i]=0;
		if (sample>0){
			for (uint i=0;i<n-alph;i++)
				markBitmapRules(i+alph,1,sample,bitVector);
		}else{
			for (uint i=0;i<n_uints_bmp;i++)
				bitVector[i]=~0U;
		}
		bd = new BitSequenceRG(bitVector,bmp_len,21);
		//bitVector is not needed anymore
		delete [] bitVector;
		sampledRules = bd->rank1(n-alph);
		
		uint *sampledLengths = new uint[sampledRules+1];
		for (uint i=0;i<=sampledRules;i++){
			sampledLengths[i]=0;
		}
		
		uint **sampledCounters = new uint*[sigma];
		for (uint i=0;i<sigma;i++){
			sampledCounters[i] = new uint[sampledRules+1];
			for (uint j=0;j<=sampledRules;j++)
				sampledCounters[i][j]=0;
		}
		
		//I add a superfluous rule at the end of the vector of lengths (util in rmq)
		for (size_t i=0;i<sampledRules;i++){
			//assert((bd->select1(i+1)+alph)<n);
			auto sel = bd->select1(i+1)+alph;
			//this si a "fix" because i'm getting unexpectly high values at some points of the
			//lengths array (generally at the last position). //TODO check why this is happening
			sampledLengths[i]=lengths[sel];//(lengths[sel]>=sampledRules)?0:lengths[sel];
			for (uint j=0;j<sigma;j++){
                //TODO: This check shouldn't be necessary. It ought to be >=0 always
				if(counters[j][sel]==(uint)-1)
                    counters[j][sel]=0;
				sampledCounters[j][i]=counters[j][sel];
			}
		}
		
		compressRules(sampledRules,sigma,sampledLengths,sampledCounters);
		for (uint i=0;i<sampledRules;i++){
			assert(sampledLengths[i]==ruleLengths->access(i));
		}
		assert(testRules(lengths,counters,n,sigma));
		for (uint i=0;i<sigma;i++)
			delete [] sampledCounters[i];
		delete [] sampledCounters;
		delete [] sampledLengths;
		
	}


	void SequenceRepair::markBitmapRules(uint rule, uint depth, uint sample, uint *bitVector){
		uint alph = dictionary->getAlph();
		if (rule>=alph){
			rule-=alph;
			bool left,right;
			left=right=true;
			
			if (!(depth%sample)){
				if (dictionary->getLeft(rule+alph)>=alph){
					left = !bitget(bitVector,dictionary->getLeft(rule+alph)-alph);
					if (left){
						bit_set(bitVector, dictionary->getLeft(rule + alph) - alph);
					}
				}else{
					left=false;
				}
				
				if (dictionary->getRight(rule+alph)>=alph){
					right = !bitget(bitVector,dictionary->getRight(rule+alph)-alph);
					if (right){
						bit_set(bitVector, dictionary->getRight(rule + alph) - alph);
					}
				}else{
					right=false;
				}
			}
			if (left) markBitmapRules(dictionary->getLeft(rule+alph),depth+1,sample,bitVector);
				
			if (right) markBitmapRules(dictionary->getRight(rule+alph),depth+1,sample,bitVector);
		}
	}
	
	uint SequenceRepair::getLengthCounterRule(uint rule, uint &len, uint &cnt, uint sigma)const{
		if (rule<alph){ 
			//TODO remplece this by a table
			if (rule==sigma)cnt++;
			len++;
		}else{
			 size_t t1;
			 if (bd->access(t1=(rule-alph))) {
				rule=t1;
				auto r = bd->rank1(rule)-1;
				cnt+=ruleCounters[sigma]->access(r);
				len+=ruleLengths->access(r);
			}else{ 				
				getLengthCounterRule(dictionary->getLeft(rule),len,cnt,sigma);
				getLengthCounterRule(dictionary->getRight(rule),len,cnt,sigma);
			}
		}
		return len;
	}
	
	uint SequenceRepair::getLengthRule(uint rule)const{
		if (rule<alph){ 
			return 1;
		}else{
			 size_t t1;
			 if (bd->access(t1=(rule-alph))) {
				rule=t1;
				uint r = bd->rank1(rule)-1;
				return ruleLengths->access(r);
			}else{ 				
				return getLengthRule(dictionary->getLeft(rule))
						+ getLengthRule(dictionary->getRight(rule));
			}
		}
	}
	
	uint SequenceRepair::getCounterRule(uint rule, uint sigma)const{
		if (rule<alph){ 
			//TODO remplece this by a table
			if (rule==sigma) return 1;
			return 0;
		}else{
			 uint t1;
			 if (bd->access(t1=(rule-alph))) {
				rule=t1;
				size_t r = bd->rank1(rule)-1;
				return ruleCounters[sigma]->access(r);
			}else{ 				
				return getCounterRule(dictionary->getLeft(rule),sigma)
						+ getCounterRule(dictionary->getRight(rule),sigma);
			}
		}
	}
	
	inline uint SequenceRepair::getC_o(uint pos)const{
		return C_o->access(pos);
	}
	
	inline uint SequenceRepair::getC_p(uint pos)const{
		return C_p->access(pos);
	}
	
	inline uint SequenceRepair::getC_c(uint pos, uint symbol)const{
		#if SB_COUNTERS_REPAIR
			return SC_counter[symbol]->access(pos>>log_ss_rate)+C_counter[symbol]->access(pos);
		#else
			return C_counter[symbol]->access(pos);
		#endif
	}
	
	inline uint SequenceRepair::getC(uint pos) const{
		return (*Cs)[pos];
	}


	ulong SequenceRepair::extractFromRule(ulong i, ulong j, uint posInC, ulong sumL, uint rule, vector<uint> &res) const{
		ulong current = i;
		stack<uint> stack;
		stack.push(rule);
		while(current<j){
			rule = stack.top();
			stack.pop();
			//exhaust the current rule checking not to go beyond j
			while(current<j) {
				if (rule >= alph) {
					size_t left;
					uint l;
					left = getLengthRule((l = dictionary->getLeft(rule)));
					size_t t1;
					if ((t1 = sumL + left) <= current) {
						sumL = t1;
						rule = dictionary->getRight(rule);
					} else {
						//if we move to the left, we store in a stack the right rule to continue
						//processing it when we exhaust the left sub-tree
						stack.push(dictionary->getRight(rule));
						rule = l;
					}
				}else{
					res.push_back(rule);
					current++;
					sumL++;
					if (stack.empty()) break;
					rule = stack.top();
					stack.pop();
				}

			}
			//iterate on C while don't reach j
			stack.push(getC(++posInC));
		}
		return j-i;
	}
	
	bool SequenceRepair::testRules(uint *lengths, uint **counters, uint nrules, uint sigma){

		for (uint i=0;i<dictionary->getNRules();i++){
			if (i<alph)
				assert(lengths[i]==1);
			else
				assert(lengths[i]==lengths[dictionary->getLeft(i)]+lengths[dictionary->getRight(i)]);
		}
		for (uint i=0;i<nrules;i++){
			assert(lengths[i]==getLengthRule(i));
		}
		for (uint i=0;i<sigma;i++){
			for (uint j=0;j<nrules;j++){
				assert(counters[i][j]==getCounterRule(j,i));
			}
		}
		for (uint i=0;i<sigma;i++){
			for (uint j=0;j<nrules;j++){
				uint len=0,cnt=0;
				getLengthCounterRule(j,len,cnt,i);
				assert(counters[i][j]==cnt);
				assert(lengths[j]==len);
			}
		}
		cerr << "Test rules OK!" << endl;

		return true;
	}

	DirectAccess* SequenceRepair::compressWithDirectAccess(uint *list,size_t totalSamples,bool overwriteVectors){
		//TODO: this is a patch...
		for (size_t i=0;i<totalSamples;i++){
			if (list[i]==(uint)-1)
				list[i]=0U;
		}
		if (totalSamples<20){
            //TODO: solve this using  a global DAC (using pointers for small blocks)
			return new ArrayDA(list,totalSamples);
		}else{
			return new DAC(list,static_cast<uint>(totalSamples),overwriteVectors);
		}
	}

    DictionaryRP *SequenceRepair::getDictionary() {
        return dictionary;
    }

    void SequenceRepair::setDictionary(DictionaryRP *dic) {
        dictionary = dic;
    }
};
