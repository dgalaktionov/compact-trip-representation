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

#ifndef _SEQUENCE_REPAIRSN_H
#define _SEQUENCE_REPAIRSN_H
#include <sequence/SequenceRepair.h>
#define TESTS_SN 1

using namespace std;

namespace cds_static
{	
	class SequenceRepairSN : public SequenceRepair
	{
		/**
		 * By defining the macro SB_COUNTERS_REPAIR_SC, the counters of symbols for each 
		 * sample of C are stored into two layers (blocks + superblocks). It should take
		 * less space but requires access to two DACs instead of one.
		 * */
		public:

			/** Builds a Repair sequence enhanced with access/rank/select capabilities. 
			 *  It uses counters for each symbol thus the voc. size must be low.
			 *  It differs from SequenceRepair that now the sampling of C is carried out at
			 *  regular intevals of C, not of n. This should reduce the space overhead in cases
			 *  in which the redundancy of the samples is also high (very rep. collections). 
			 * @param bmb builder for the bitmaps in each level.
			 * @param am alphabet mapper (we need all symbols to be used).
			 * @param pb builder for the permutation thata uses the Repair dictionary
			 * @param samplingC logarithm (base 2) of the sampling period for the sequence C. That is, 
			 * to store a sample every 1024 symbols, samplingC must be 10.
			 * @param delta sampling rate of the rules
			 * @param dicBuilder is a DictionaryRPBuilder you can set to manage the RePair dictionary. Is optional so in
			 * case you don't specify one, the default DictionaryRPBuilderTabei is used (a compact representation of the dictionary).
			 * */
			SequenceRepairSN(uint * symbols, size_t n, BitSequenceBuilder * bmb, Mapper * am, 
						PermutationBuilder *, uint samplingC, uint delta, uint ss_rate, DictionaryRPBuilder *dicBuilder = nullptr, bool deleteSymbols = false);
			
			SequenceRepairSN(uint * symbols, size_t n, Repair *rp, BitSequenceBuilder * bmb, Mapper * am, 
						PermutationBuilder *, uint samplingC, uint delta, uint ss_rate, DictionaryRPBuilder *dicBuilder = nullptr, bool deleteSymbols = false);
						
			SequenceRepairSN(const Array & seq, BitSequenceBuilder * bmb, Mapper * am, 
						PermutationBuilder *, uint samplingC, uint delta, uint ss_rate,DictionaryRPBuilder *dicBuilder = nullptr, bool deleteSymbols = false);
						
			/** Destroys the Wavelet Tree */
			virtual ~SequenceRepairSN();

			virtual size_t rank(uint symbol, size_t pos) const;
			virtual size_t select(uint symbol, size_t j) const;
			virtual uint access(size_t pos) const;
			virtual uint access(size_t pos, uint &r) const;
			virtual ulong extract(ulong i, ulong j, vector<uint> &res) const;
			virtual size_t getSize() const;
            virtual size_t getSizeAccess() const;
			virtual void save(ofstream & fp) const;
			static SequenceRepairSN * load(ifstream & fp);
			virtual string GetInfo() const;

		protected:
			SequenceRepairSN();
			size_t rankInternal(uint symbol, size_t pos, uint &r) const;
			virtual void superSampleCountersC(uint **counters, uint sigma, uint nSamples);
			virtual void sampleC(uint *input, uint sigma, uint delta, uint *lengths);
			virtual bool testSampleC(uint *input, uint *sampleC_o, uint *sampleC_p, uint **sampleC_c, uint nSamples, uint nrules, uint sigma);
			virtual bool testSuperSampleCounters(uint **counters, uint **newCounters, uint **ss_counters, uint nSamples, uint nSSamples, uint sigma);
			virtual uint searchBlock(uint symbol, uint j, uint &cnt) const;
			virtual void printSizes(size_t total) const;
			
	};
};

#endif
