/* SequenceAlphPartRepReg.h
 * Copyright (C) 2014, Alberto Oróñez, all rights reserved.
 *
 * Alphabet Partition enhanced with grammar compression
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

#ifndef _SEQUENCE_ALPH_PART_REP_REG_
#define _SEQUENCE_ALPH_PART_REP_REG_

#include <sequence/SequenceAlphPart.h>
#include <sequence/SequenceBuilder.h>
#include <permutation/PermutationBuilder.h>
#include <grammar/Repair.h>

using namespace cds_utils;
using namespace std;

namespace cds_static
{

	class DictionaryRP;
	/** 
		Sequence Alphabet Partitioning tailored to repetitive sequences
		The size of the first groups is constant. The rest is as on SequenceAlphPart
	 
	    @author Alberto Ordóñez
	 */
	class SequenceAlphPartRepReg : public SequenceAlphPart
	{

		public:
			/**
			 * The first cut classes are stored directly on C. Then, the first cutOffsets
			 * sequence of offsets are encoded using seqFirstClasses
			 * 
			 * */
			SequenceAlphPartRepReg(uint * seq, size_t n, uint cut, uint cutOffsets,
						uint firstGroupSizes,
						SequenceBuilder * lenIndexBuilder, 
						SequenceBuilder * seqsBuilder, 
						SequenceBuilder * seqFirstClasses, 
						PermutationBuilder *pb);


			SequenceAlphPartRepReg();

			virtual ~SequenceAlphPartRepReg();

			virtual void save(ofstream & fp) const;
			
			static SequenceAlphPartRepReg * load(ifstream & fp);

		protected:
			
			/**
			 * c: length of the sequence C after applying Repair
			 * */
			uint c;
			/**
				Size of the first cutOffsets clases (number of elements of each class). Thus, the 
				total number of elements stored in the first cutOffsets classes (excluding those directly 
				handled in C -cut-) is cutOffsets*firstGroupSizes.
			*/
			uint secondGroupSize;
			uint nSecondGroups;
			//it has a 1 at blogThirdGroupFirstClassSize (if the first class of the third section has size 16, then it maskThirGroup must be 16 10000)
			uint maskThirdGroup;
			uint maskThirdGroupOffset;
			uint minBitThirdGroup;
			uint cteGroupSizes;
			
			DictionaryRP *dictionary;
			
			virtual inline uint group(uint pos, uint cut) const;
			virtual inline uint offset(uint pos, uint cut, uint group) const;
			void initParameters(uint ,uint, uint);
			bool testGroupAndOffset() ;

	};

};
#include <grammar/DictionaryRP.h>
#endif							 /* _SEQUENCE_ALPH_PART_REP_REG_ */
