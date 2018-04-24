/* BitSequenceRPSN.h
 * Copyright (C) 2014-curren_year, Alberto Ordóñez, all rights reserved.
 *
 * BitSequence that compress the bitmap using Repair. It samples the resulting 
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

#ifndef _STATIC_BITSEQUENCE_RP_SN_H
#define _STATIC_BITSEQUENCE_RP_SN_H

#include <utils/libcdsBasics.h>
#include <bitsequence/BitSequence.h>

using namespace cds_utils;

namespace cds_static
{
	//forward declarations to avoid compilation errors due to circular references
	class BitSequenceBuilder;
	class SequenceRepairSN;
	class PermutationBuilder;
	class Repair;
    class DictionaryRPBuilder;
	class BitSequenceRPSN : public BitSequence
	{
		
		public:
			BitSequenceRPSN(uint * symbols, size_t n, BitSequenceBuilder *bsb, PermutationBuilder *pb, uint samplingC, uint delta, uint ss_rate, DictionaryRPBuilder *dic=nullptr, bool deleteSymbols=false);
			BitSequenceRPSN(uint * symbols, size_t n, Repair *rp, BitSequenceBuilder *bsb, PermutationBuilder *pb, uint samplingC, uint delta, uint ss_rate, DictionaryRPBuilder *dic=nullptr, bool deleteSymbols=false);
			//BitSequenceRPSN(const BitString & bs, uint sample_rate=DEFAULT_SAMPLING);

			virtual ~BitSequenceRPSN();

			virtual size_t rank0(const size_t i) const;

			virtual size_t rank1(const size_t i) const;

			virtual size_t select0(const size_t i) const;

			virtual size_t select1(const size_t i) const;

			virtual bool access(const size_t i) const;

			virtual bool access(const size_t i, size_t &r) const;

			virtual size_t getSize() const;

			virtual void save(ofstream & fp) const;

			/** Reads the bitmap from a file pointer, returns NULL in case of error */
			static BitSequenceRPSN * load(ifstream & fp);


		protected:
			/**
			 * We can use SequenceRepairSC to compress a sequence with sigma==2
			 * */
			SequenceRepairSN *seq; 
			
			BitSequenceRPSN();

	};

	//~ class SequenceRepairSC;
}

#endif	

