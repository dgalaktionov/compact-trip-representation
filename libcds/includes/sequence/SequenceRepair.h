/* SequenceRepair.h
 * Copyright (C) 2014-current_year, Alberto Ordóñez, all rights reserved.
 *
 * Sequence encoded with Repair which stores counters to solve rank/select/access operations
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

#ifndef _SEQUENCE_REPAIR_H
#define _SEQUENCE_REPAIR_H

#include <iostream>
#include <cassert>
#include <utils/libcdsBasics.h>
#include <bitsequence/BitSequence.h>
#include <bitsequence/BitSequenceBuilder.h>
#include <sequence/Sequence.h>
#include <mapper/Mapper.h>
#include <direct_access/DAC.h>
#include <permutation/PermutationBuilder.h>
#include <grammar/DictionaryRPBuilder.h>

#define SB_COUNTERS_REPAIR 1
using namespace std;

namespace cds_static
{
	class DictionaryRPBuilder;

	class SequenceRepair : public Sequence
	{
		public:

			/** Destroys the Wavelet Tree */
			virtual ~SequenceRepair(){}
			SequenceRepair(size_t n);
            DictionaryRP *getDictionary();
            void setDictionary(DictionaryRP *);
			virtual size_t rank(uint symbol, size_t pos) const =0;
			virtual size_t select(uint symbol, size_t j) const=0;
			virtual uint access(size_t pos) const=0;
			virtual ulong extract(ulong i, ulong j, vector<uint> &res) const = 0;
			/**
			 * In case of sigma==2, r will contain the rank0(pos).
			 * */
			virtual uint access(size_t pos, uint &r) const=0;
			virtual size_t getSize() const=0;
            virtual size_t getSizeAccess() const=0;
			virtual void save(ofstream & fp) const=0;
			static SequenceRepair * load(ifstream & fp);

		protected:
			SequenceRepair();

			Mapper * am;

			DictionaryRP *dictionary;

			Array *Cs;

			uint blockSizeC;

			uint blogBlockSizeC;

			uint nSamplesC;

			uint sampledRules;

			/**
			 * number of super samples of C (used to store the counters)
			 * */
			uint nSSamplesC;

			uint ss_rate;

			uint log_ss_rate;

			BitSequence *bd;

			DirectAccess *ruleLengths;

			DirectAccess **ruleCounters;

			DirectAccess *C_p;

			DirectAccess *C_o;

			DirectAccess **SC_counter;

			DirectAccess **C_counter;
			
			
			uint alph;
			
			/** Length of the input string. */
			size_t n;
			
			/** C length (sequence of grammar symbols that generates the original sequence). */
			size_t c;
		
			/** max_v = sigma = voc. size. */
			uint max_v;

            uint sample_r;

			/** Obtains the maximum value from the string
			 * symbols of length n */
			uint max_value(uint *symbols, size_t n);

			void build(uint * symbols, size_t n, BitSequenceBuilder * bmb, Mapper * am,  PermutationBuilder *pb, 
						uint samplingC, uint delta, uint _ss_rate, bool deleteSymbols, DictionaryRPBuilder *dicBuilder=nullptr);
			void build(uint * symbols, size_t n, Repair *rp, BitSequenceBuilder * bmb, Mapper * am,  PermutationBuilder *pb, 
						uint samplingC, uint delta, uint _ss_rate, bool deleteSymbols, DictionaryRPBuilder *dicBuilder=nullptr);
			virtual void sampleC(uint *input, uint sigma, uint delta, uint *lengths)=0;
			virtual void superSampleCountersC(uint **counters, uint sigma, uint nSamples)=0;
			void sampleRules(uint *input, size_t len, uint sigma, uint delta, uint *&lengths);
			void sampleRules(uint sample, uint n, uint alph, uint sigma, uint *lengths, uint **counters);
			uint computeRuleParams(uint rule, uint *lengths, uint **counters,uint sigma);
			void markBitmapRules(uint rule, uint depth, uint sample, uint *bitVector);
			void compressRules(uint totalSamples, uint sigma,uint *sampledLengths, uint **sampledCounters);
			uint getLengthCounterRule(uint rule, uint &len, uint &cnt, uint sigma)const;
			uint getLengthRule(uint rule) const;
			uint getCounterRule(uint rule,uint sigma)const;
			virtual uint getC_o(uint pos)const;
			virtual uint getC_p(uint pos)const;
			virtual uint getC_c(uint pos, uint symbol)const;
			virtual uint getC  (uint pos)const;
			bool testRules(uint *lengths, uint **counters, uint nrules, uint sigma);
			virtual bool testSampleC(uint *input, uint *sampleC_o, uint *sampleC_p, uint **sampleC_c, uint nSamples, uint nrules, uint sigma)=0;
			virtual bool testSuperSampleCounters(uint **counters, uint **newCounters, uint **ss_counters, uint nSamples, uint nSSamples, uint sigma)=0;
			virtual uint searchBlock(uint symbol, uint j, uint &cnt) const=0;
			virtual void printSizes(size_t total) const=0;
			DirectAccess* compressWithDirectAccess(uint *sampledLengths,size_t totalSamples,bool overwriteVectors);
			//helper function for extract
			ulong extractFromRule(ulong i, ulong j, uint posInC, ulong sumL, uint rule, vector<uint> &res) const;
	};
};
#include <sequence/SequenceRepairSC.h>
#include <sequence/SequenceRepairSN.h>
#include <grammar/DictionaryRP.h>
#endif
