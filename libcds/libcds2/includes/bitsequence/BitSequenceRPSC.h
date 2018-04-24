/* BitSequenceRPSC.h
 * Copyright (C) 2014-curren_year, Alberto Ordóñez, all rights reserved.
 *
 * BitSequence that compresses the bitmap using Repair. It samples the resulting 
 * sequence at regular intervals on that sequence, not at a regular intervals 
 * of the original sequence. (basically, it is a wrapper for sequence/SequenceRepairSC).
 * Note you only need counters for 0 or 1, not for both since you have to store also the 
 * length of the segments/rules sampled. 
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

#ifndef _STATIC_BITSEQUENCE_RP_SC_H
#define _STATIC_BITSEQUENCE_RP_SC_H

#include <utils/libcdsBasics.h>
#include <bitsequence/BitSequence.h>
#include <grammar/DictionaryRP.h>


using namespace cds_utils;

namespace cds_static
{
	//forward declarations to avoid compilation errors due to circular references
	class BitSequenceBuilder;
	class SequenceRepairSC;
	class PermutationBuilder;
	class Repair;
    class DictionaryRPBuilder;
	class BitSequenceRPSC : public BitSequence
	{
		
		public:

			/**
				Builds a grammar compressed representation of the bitvector stored at "symbols".
				symbols: bitvector
				n: number of bits of the bitvector
				bsb: bitSequenceBuilder that is not currently used (TODO: remove it in the next version)
				pb: permutation builder to represent the dictionary (it is explained in the SPIRE's paper)
				samplingC: sampling step of the sequence C (the result of applying repair)
				delta: sampling for rules
				ss_rate: super sample step for C's samples. 

			*/
			BitSequenceRPSC(uint * symbols, size_t n, BitSequenceBuilder *bsb, PermutationBuilder *pb, uint samplingC, uint delta, uint ss_rate, DictionaryRPBuilder *dic=nullptr, bool deleteSymbols=false);
			BitSequenceRPSC(uint * symbols, size_t n, Repair *rp, BitSequenceBuilder *bsb, PermutationBuilder *pb, uint samplingC, uint delta, uint ss_rate, DictionaryRPBuilder *dic=nullptr, bool deleteSymbols=false);
			//BitSequenceRPSC(const BitString & bs, uint sample_rate=DEFAULT_SAMPLING);

			virtual ~BitSequenceRPSC();

			virtual size_t rank0(const size_t i) const;

			virtual size_t rank1(const size_t i) const;

			virtual size_t select0(const size_t i) const;

			virtual size_t select1(const size_t i) const;

			virtual bool access(const size_t i) const;

			virtual bool access(const size_t i, size_t &r) const;

			virtual size_t getSize() const;

			virtual void save(ofstream & fp) const;

			/** Reads the bitmap from a file pointer, returns NULL in case of error */
			static BitSequenceRPSC * load(ifstream & fp);


		protected:
			/**
			 * We can use SequenceRepairSC to compress a sequence with sigma==2
			 * */
			SequenceRepairSC *seq; 
			
			BitSequenceRPSC();

	};

	//~ class SequenceRepairSC;
}

#endif	

