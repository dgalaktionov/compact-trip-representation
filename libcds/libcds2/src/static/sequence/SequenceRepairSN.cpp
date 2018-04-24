/* SequenceRepairSN.cpp
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

#include <sequence/SequenceRepairSN.h>
#include <assert.h>
#include <stack>

namespace cds_static
{

	SequenceRepairSN::SequenceRepairSN(uint * symbols, size_t n, BitSequenceBuilder * bmb, Mapper * am,  PermutationBuilder *pb, 
						uint samplingC, uint delta, uint ss_rate, DictionaryRPBuilder *dicBuilder, bool deleteSymbols):SequenceRepair(n) {
		
		build(symbols,n,bmb,am,pb,samplingC,delta,ss_rate,deleteSymbols,dicBuilder);
		length = n;
	}
	SequenceRepairSN::SequenceRepairSN(uint * symbols, size_t n, Repair *rp, BitSequenceBuilder * bmb, Mapper * am,  PermutationBuilder *pb, 
						uint samplingC, uint delta, uint ss_rate,DictionaryRPBuilder *dicBuilder, bool deleteSymbols):SequenceRepair(n) {
		
		build(symbols,n,rp,bmb,am,pb,samplingC,delta,ss_rate,deleteSymbols,dicBuilder);
		length = n;
	}
	SequenceRepairSN::SequenceRepairSN(const Array & seq, BitSequenceBuilder * bmb, Mapper * am,  PermutationBuilder *pb, 
						uint samplingC, uint delta, uint ss_rate, DictionaryRPBuilder *dicBuilder, bool deleteSymbols):SequenceRepair(seq.getLength()){
		uint *symbols = new uint[seq.getLength()];
		for (size_t i=0;i<seq.getLength();i++){
			symbols[i] = seq[i];
		}
		build(symbols,n,bmb,am,pb,samplingC,delta,ss_rate,deleteSymbols,dicBuilder);
		delete [] symbols;
		length = n;
	}
	
	SequenceRepairSN::SequenceRepairSN():SequenceRepair(0) {	}

	SequenceRepairSN::~SequenceRepairSN() {
		#if 0
		delete dictionary;
		delete Cs;
		delete bd;
		delete ruleLengths;
		for (uint i=0;i<max_v;i++)
			delete ruleCounters[i];
		delete [] ruleCounters;
		delete C_o;
		delete C_p;
		for (uint i=0;i<max_v;i++)
			delete C_counter[i];
		delete [] C_counter;
		for (uint i=0;i<max_v;i++)
			delete SC_counter[i];
		delete [] SC_counter;
		#endif
	}

	string SequenceRepairSN::GetInfo() const{
		return "";
	}

	void SequenceRepairSN::save(ofstream & fp) const
	{
		uint wr = SEQUENCE_REPAIRSN_HDR;
		saveValue(fp,wr);
		saveValue(fp, n);
		saveValue(fp,alph);
		saveValue(fp, max_v);
		saveValue(fp, c);
		dictionary->save(fp);
		Cs->save(fp);
		saveValue(fp, blockSizeC);
		saveValue(fp,blogBlockSizeC);
		saveValue(fp, nSamplesC);
		saveValue(fp,sampledRules);
		saveValue(fp, nSSamplesC);
		saveValue(fp, ss_rate);
		saveValue(fp,log_ss_rate);
		bd->save(fp);
		ruleLengths->save(fp);
		for (uint i=0;i<max_v;i++)
			ruleCounters[i]->save(fp);
		C_p->save(fp);
		C_o->save(fp);
		
		for (uint i=0U;i<max_v;i++)
			C_counter[i]->save(fp);
		for (uint i=0U;i<max_v;i++)
			SC_counter[i]->save(fp);
        uint dummy = -1;
        saveValue(fp,dummy);
        saveValue(fp,sample_r);
	}

	SequenceRepairSN * SequenceRepairSN::load(ifstream & fp) {
		SequenceRepairSN *ret = NULL;
		try{

			uint rd = loadValue<uint>(fp);
			if(rd!=SEQUENCE_REPAIRSN_HDR) return NULL;
			ret = new SequenceRepairSN();
			ret->am = nullptr;
			ret->n = loadValue<size_t>(fp);
			ret->length = ret->n;
			ret->alph = loadValue<uint>(fp);
			ret->max_v = loadValue<uint>(fp);
			ret->c = loadValue<size_t>(fp);
			ret->dictionary = DictionaryRP::load(fp);
			ret->Cs = new Array(fp);
			ret->blockSizeC = loadValue<uint>(fp);
			ret->blogBlockSizeC = loadValue<uint>(fp);
			ret->nSamplesC = loadValue<uint>(fp);
			ret->sampledRules = loadValue<uint>(fp);
			ret->nSSamplesC = loadValue<uint>(fp);
			ret->ss_rate = loadValue<uint>(fp);
			ret->log_ss_rate = loadValue<uint>(fp);
			ret->bd = BitSequence::load(fp);
			ret->ruleLengths = DirectAccess::load(fp);
			ret->ruleCounters = new DirectAccess*[ret->max_v+1];
			for (uint i=0;i<ret->max_v;i++)
				ret->ruleCounters[i]= DirectAccess::load(fp);
			ret->C_p = DirectAccess::load(fp);
			ret->C_o = DirectAccess::load(fp);
			ret->C_counter = new DirectAccess*[ret->max_v+1];
			for (uint i=0;i<ret->max_v;i++){
				ret->C_counter[i] = DirectAccess::load(fp);
			}
			ret->SC_counter = new DirectAccess*[ret->max_v+1];
			for (uint i=0;i<ret->max_v;i++)
				ret->SC_counter[i] = DirectAccess::load(fp);
            auto pos = fp.tellg();
            uint dummy = loadValue<uint>(fp);
            if (dummy==-1){
                ret->sample_r = loadValue<uint>(fp);
            }else{
                fp.seekg(pos);
            }

		}catch(...){
			return NULL;
		}
		return ret;
	}

	ulong SequenceRepairSN::extract(ulong i, ulong j, vector<uint> &res) const{

		//locate the rule that contains the S[i]
		j = min(j,length);
		assert(i<=j);
		uint posInC;
		size_t p,sumL,lenr;
		p = i >> blogBlockSizeC;
		posInC=getC_p(p);
		sumL = (p<<blogBlockSizeC)-getC_o(p);
		uint rule = getC(posInC);
		while((lenr=sumL+getLengthRule(rule)) <= i){
			sumL =lenr;
			rule=getC(++posInC);
		}
		return extractFromRule(i,j,posInC, sumL, rule,res);
	}

	uint SequenceRepairSN::access(size_t pos) const
	{

//        uint acc;
//        rankInternal(0,pos,acc);
//        return acc;
		uint posInC;
		size_t p,sumL,lenr;
		p = pos >> blogBlockSizeC;
		posInC=getC_p(p);
		sumL = (p<<blogBlockSizeC)-getC_o(p);
		uint rule = getC(posInC);
		while((lenr=sumL+getLengthRule(rule)) <= pos){
			sumL =lenr;
			rule=getC(++posInC);
		}

		while(rule>=alph){
			size_t left,l;
			left = getLengthRule((l=dictionary->getLeft(rule)));
			size_t t1;
			if ((t1=sumL+left)<=pos){
				sumL=t1;
				rule=dictionary->getRight(rule);
			}else{
				rule=l;
			}
		}
		return rule;
	}
	
	uint SequenceRepairSN::access(size_t pos, uint &res) const
	{
		uint acc;
		res = rankInternal(0,pos,acc);
		//rank internal carries out the rank of 0's since the
		//first argument given is 0. However, what we have to return
		//is the rank of 0's/1's depending on the bit at position
		//pos, which is acc.
		if (acc){
			res = pos - res + 1;
		}
		return acc;
	}
	
	size_t SequenceRepairSN::rank(uint symbol, size_t pos) const
	{
		uint res=0;
		if (pos+1==0) return 0;
		return rankInternal(symbol,pos,res);
	}

	inline size_t SequenceRepairSN::rankInternal(uint symbol, size_t pos, uint &res) const
	{
		//Find the block where position is contained in O(1)
		size_t sumZ,sumL,posInC,p;
		p = pos >> blogBlockSizeC;
		posInC=getC_p(p);
		sumL = (p<<blogBlockSizeC)-getC_o(p);
		//we acumulate the excess instead of the number of 0's (the excess is what
		//we have stored). 
		sumZ = getC_c(p,symbol);
		size_t rule = getC(posInC);	
		uint lenr=sumL;uint cnt=sumZ;
		//TODO length and counter in a call
		//search forward until reach a rule that overtakes the searched position
		
		while(getLengthCounterRule(rule,lenr,cnt,symbol) <= pos){
			sumL  =lenr;
			sumZ  =cnt;
			rule=getC(++posInC);
		}
		
		//search the rule down moving left/right depending on the acumulated position
		while(rule>=alph){
			size_t left,l;
			left = getLengthRule((l=dictionary->getLeft(rule)));
			size_t t1;
			if ((t1=sumL+left)<=pos){
				sumL=t1;
				sumZ+=getCounterRule(l,symbol);//getTotalExcess(l);
				rule=dictionary->getRight(rule);
			}else{
				rule=l;
			}
		}
		assert(rule>=0 && rule<alph);
		//TODO table here
		if (rule==symbol)sumZ++;
		res = rule;
		return sumZ;
	}	
	uint SequenceRepairSN::searchBlock(uint symbol, uint j, uint &cnt) const{
		uint l,r,block;
		l=0;r=nSamplesC-1;
		while(l<=r){
			block=(l+r)>>1;
			cnt = getC_c(block,symbol);
			if (cnt>j){
				r=block-1;
			}else if (cnt<j){
				l=block+1;
			}else{
				break;
			}
		}
		while(cnt>=j)cnt=getC_c(--block,symbol);
		assert(cnt<=j);
		return block;
	}
	
	size_t SequenceRepairSN::select(uint symbol, size_t j) const
	{
	
		size_t sumL,posInC,p;
		uint sumZ;
		p = searchBlock(symbol,j,sumZ);
		posInC=getC_p(p);
		sumL = (p<<blogBlockSizeC)-getC_o(p);
		size_t rule = getC(posInC);	
		uint lenr=sumL;uint cnt=sumZ;
		//TODO length and counter in a call
		//search forward until reach a rule that overtakes the searched position
		getLengthCounterRule(rule,lenr,cnt,symbol);
		while(cnt < j && (++posInC<c)){
			sumL  =lenr;
			sumZ  =cnt;
			rule=getC(posInC);
			getLengthCounterRule(rule,lenr,cnt,symbol);	
		}
		if (posInC==c) return -1;
		lenr=sumL,cnt=sumZ;
		while(rule>=alph){
			uint lrule;
			getLengthCounterRule((lrule=dictionary->getLeft(rule)),lenr,cnt,symbol);
			if (cnt < j){
				sumL =lenr;
				sumZ =cnt;
				rule=dictionary->getRight(rule);
			}else{
				cnt  =sumZ;
				lenr =sumL;
				rule=lrule;
			}
		}
		assert(((rule==symbol)?sumZ+1:sumZ)==j);
		return sumL;
	}

	void SequenceRepairSN::printSizes(size_t total_size) const{
		cerr << "*****SN******" << endl;
		cerr << "n:" << n << endl;
		cerr << "Dictionary:" << dictionary->getSize() << endl;
		cerr << "C:"<<Cs->getSize() << endl;
		cerr << "bd:"<< bd->getSize()<< endl;
		cerr << "Rule_lengths:" << ruleLengths->getSize()<< endl;
		size_t total = sizeof(DirectAccess)*max_v;
		for (uint i=0;i<max_v;i++)
			total+= ruleCounters[i]->getSize();
		cerr << "rule_counters:"<<total<<endl;
		total=0;
		cerr << "C_p:"<<C_p->getSize()<<endl;
		cerr << "C_o:"<<C_o->getSize()<<endl;
		total +=sizeof(DirectAccess)*max_v;
		for (uint i=0;i<max_v;i++)
			total+=C_counter[i]->getSize();
		cerr << "C_counter:"<<total<<endl;
		total=0;
		#if SB_COUNTERS_REPAIR
			total +=sizeof(DirectAccess)*max_v;
			for (uint i=0;i<max_v;i++)
				total+=SC_counter[i]->getSize();
			cerr << "SC_counterSC:" << total << endl;
		#endif
		cerr << "total:"<< total_size << endl;
		cerr << "+++++++++++" << endl;
	}

	size_t SequenceRepairSN::getSize() const{
		uint bin_max_v = (max_v==2)?1:max_v;
		/**
		 * in case of binary alphabets, we do not have to 
		 * store counters for 0 and 1. It is enough to store
		 * one of them since the other can be computed if we know
		 * the length of the segment. 
		 * */
		size_t total=sizeof(SequenceRepairSN);
		if (am) total += am->getSize();
		total += dictionary->getSize();
		total += Cs->getSize();
		//TODO add a parameter to know when all rules are sampled
		//if (bd->getLength()==bd->rank1(bd->getLength()-1)) total += bd->getSize();
		total += ruleLengths->getSize();
		total += sizeof(DirectAccess)*bin_max_v;
		for (uint i=0;i<bin_max_v;i++)
			total+= ruleCounters[i]->getSize();
		total += C_p->getSize();
		total += C_o->getSize();
		total +=sizeof(DirectAccess)*bin_max_v;
		for (uint i=0;i<bin_max_v;i++)
			total+=C_counter[i]->getSize();
		#if SB_COUNTERS_REPAIR
			total +=sizeof(DirectAccess)*bin_max_v;
			for (uint i=0;i<bin_max_v;i++)
				total+=SC_counter[i]->getSize();
		#endif
		//~ printSizes(total);
		return total;
	}

    size_t SequenceRepairSN::getSizeAccess() const{

        size_t total=sizeof(SequenceRepairSN);
        if (am) total += am->getSize();
        total += dictionary->getSize();
        total += Cs->getSize();
        if (sample_r)
            total+=bd->getSize();
        total += ruleLengths->getSize();
        total += C_p->getSize();
        total += C_o->getSize();
        return total;
    }
	
	
	//sampleC at regular intervals in the length of the original sequence
	void SequenceRepairSN::sampleC(uint *input, uint sigma, uint delta, uint *lengths) {
		//cerr << "SN sampleC" << endl;
		//now we sample C at regular intevals of C (not in n)
		//length-1 because we add a superflous element at the very end of C
		//~ len = Cs->getLength()-1;
		uint len = n;
       //now we sample C 
        nSamplesC =   (len/blockSizeC)+3;
        
        uint *sampleC_o  = new uint[nSamplesC];
        uint *sampleC_p  = new uint[nSamplesC];
        //allocate space for the counters of each symbol
		uint **counters = new uint*[sigma];

		for (uint i=0;i<sigma;i++) {
			counters[i] = new uint[nSamplesC];
			for (uint j = 0; j < nSamplesC; j++)
				counters[i][j] = 0U;
		}
        for (uint i=0;i<nSamplesC;i++){
			sampleC_p[i]=sampleC_o[i]=0U;
		}
		uint j		= 1U;
		uint r		= 0U;
		uint sumL	= 0U;
		
		while(sumL<len){

			while(sumL<len && sumL+lengths[(*Cs)[r]]<=j*blockSizeC){
				uint k 	 = sumL;
				sumL	+= lengths[(*Cs)[r++]];
				for (;k<sumL;k++){
					counters[input[k]][j]++;
				}
				assert(k==sumL);
			}
			sampleC_o[j]=j*blockSizeC-sumL;
			sampleC_p[j]=r;
			assert((int)sampleC_o[j]>=0);
			assert((int)sampleC_p[j]>=0);
			j++;
			//initialize the counter with the values from the previous sampling
			for (uint s=0;s<sigma;s++){
				counters[s][j]=counters[s][j-1];
			}
		}
		
		if (sumL%blockSizeC){
			sampleC_p[j] = sampleC_p[j-1];
			sampleC_o[j] = (j*blockSizeC) - ((j-1)*blockSizeC - sampleC_o[j-1]);
			assert((int)sampleC_o[j]>=0);
			assert((int)sampleC_p[j]>=0);
			j++;
		}
		
		assert(j>=(n/blockSizeC));
		nSamplesC=j;
		C_o = compressWithDirectAccess(sampleC_o,j,false);
		C_p = compressWithDirectAccess(sampleC_p,j,false);
		
		std::cout << "Going for SS" << endl;
		//sample the counters stored for c
		superSampleCountersC(counters,sigma,j);
		
		assert(testSampleC(input,sampleC_o,sampleC_p,counters,j,dictionary->getNRules(),sigma));
		
    }
    
    void SequenceRepairSN::superSampleCountersC(uint **counters, uint sigma, uint nSamples){
		#if SB_COUNTERS_REPAIR
			if ((nSamples / ss_rate) == 0U){
				ss_rate = 1U<<bits(nSamples / 2);
				log_ss_rate = bits(nSamples/2);
			}
			nSSamplesC = nSamples/ss_rate;
			if (nSamples%ss_rate) nSSamplesC++;
			
			uint **ss_counter = new uint*[sigma];
			for (uint i=0;i<sigma;i++){
				ss_counter[i] = new uint[nSSamplesC];
			}
			for (uint i=0;i<sigma;i++){
				for (uint j=0;j<nSSamplesC;j++){
					ss_counter[i][j] = counters[i][j*ss_rate];
					assert((int)ss_counter[i][j]>=0);
				}
			}
			uint **newCounters = new uint*[sigma];
			//nSamples+1...we add an spurious value at the end to avoid all of them could be equal
			for (uint i=0;i<sigma;i++)
				newCounters[i] = new uint[nSamples+1];
			for (uint i=0;i<sigma;i++){
				for (uint j=0;j<nSamples;j++){
					assert(counters[i][j]>=counters[i][j/ss_rate]);
					newCounters[i][j]=counters[i][j]-ss_counter[i][j/ss_rate];
				}
				newCounters[i][nSamples]=newCounters[i][nSamples-1]+1;
			}
			//build DACs for the superblocks
			SC_counter = new DirectAccess*[sigma];
			for (uint i=0;i<sigma;i++){
				SC_counter[i] = compressWithDirectAccess(ss_counter[i],nSSamplesC,false);
			}
			//build DACs for the blockstestSampleC
			C_counter = new DirectAccess*[sigma];
			for (uint i=0;i<sigma;i++){
				//+1 due to the spurious
				C_counter[i] = compressWithDirectAccess(newCounters[i],nSamples+1,false);
			}
			assert(testSuperSampleCounters(counters, newCounters, ss_counter, nSamples, nSSamplesC, sigma));
		#else
			C_counter = new DAC*[sigma];
			for (uint i=0;i<sigma;i++){
				bool allEqual=true;
				for (uint j=1;j<nSamples;j++){
					if (counters[i][j]!=counters[i][j-1]){
						allEqual=false;
						break;
					}
				}
				assert(!allEqual);
				C_counter[i] = new DAC(counters[i],nSamples,false);
			}
			assert(testSuperSampleCounters(counters, (uint**)NULL, (uint**)NULL, nSamples, nSSamplesC, sigma));
		#endif
		//teset all
		
	}
	
	bool SequenceRepairSN::testSuperSampleCounters(uint **counters, uint **newCounters, uint **ss_counters, uint nSamples, uint nSSamples, uint sigma){
		#if TESTS_SN
		#if SB_COUNTERS_REPAIR
		for (uint i=0;i<sigma;i++){
			for (uint j=0;j<nSSamples;j++){
				assert(ss_counters[i][j]==SC_counter[i]->access(j));
				assert(SC_counter[i]->access(j)==counters[i][j*ss_rate]);
			}
		}
		for (uint i=0;i<sigma;i++){
			for (uint j=0;j<nSamples;j++){
				assert(newCounters[i][j]==counters[i][j]-ss_counters[i][j/ss_rate]);
				assert(C_counter[i]->access(j)==newCounters[i][j]);
				assert(counters[i][j]==SC_counter[i]->access(j/ss_rate)+C_counter[i]->access(j));
			}
		}
		#else
		for (uint i=0;i<sigma;i++){
			for (uint j=0;j<nSamples;j++){
				assert(counters[i][j]==C_counter[i]->access(j));
			}
		}
		#endif
		cerr << "Test sampling counters C OK!" << endl;
		#endif
		return true;
	}
    
	
	bool SequenceRepairSN::testSampleC(uint *input, uint *sampleC_o, uint *sampleC_p, uint **sampleC_c,uint nSamples, uint nrules, uint sigma){
		#if TESTS_SN
		//the wt is not answering well when sigma==1 -> we skip the test
		if (sigma==1) return true;
		for (uint i=0;i<nSamples;i++){
			assert(sampleC_o[i]==getC_o(i));
			assert(sampleC_p[i]==getC_p(i));
			for (uint j=0;j<sigma;j++){
				assert(sampleC_c[j][i]==getC_c(i,j));
			}
		}
		WaveletTreeNoptrs *wt = new WaveletTreeNoptrs(input,n,new BitSequenceBuilderRG(11),new MapperNone());
		for (uint i=1;i<nSamples;i++){
			uint pos = i*blockSizeC - getC_o(i);
			for (uint j=0;j<sigma;j++){
				if (pos==0) 
					assert(getC_c(i,j)==0);
				else 
					assert(wt->rank(j,pos-1)==getC_c(i,j));
			}
		}
		uint pos=-1U;
		vector<uint> counter(sigma,0U);
		for (uint i=0;i<sigma;i++) counter[i]=0;
		for (uint i=0;i<c;i++){
			uint rule = (*Cs)[i];
			pos+=getLengthRule(rule);
			for (uint j=0;j<sigma;j++){
				counter[j] += getCounterRule(rule,j);
				assert(wt->rank(j,pos)==counter[j]);
			}
		}
		assert((uint)pos+1==n);
		delete wt;
		cerr << "Test sample C OK!" << endl;
		#endif
		return true;
	}
};
