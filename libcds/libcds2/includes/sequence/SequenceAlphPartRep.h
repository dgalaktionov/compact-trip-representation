/* SequenceAlphPartRep.h
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

#ifndef _SEQUENCEALPHPARTREP_H
#define _SEQUENCEALPHPARTREP_H

#include <sequence/SequenceAlphPart.h>
#include <sequence/SequenceBuilder.h>
#include <permutation/PermutationBuilder.h>
#include <grammar/Repair.h>

using namespace cds_utils;
using namespace std;

namespace cds_static
{

	class DictionaryRP;
	/** Sequence Alphabet Partitioning tailored to repetitive sequences
	 *
	 *  @author Alberto Ordóñez
	 */
	class SequenceAlphPartRep : public SequenceAlphPart
	{

		public:
			/**
			 * The first cut classes are stored directly on C. Then, the first cutOffsets
			 * sequence of offsets are encoded using seqFirstClasses
			 * 
			 * */
			SequenceAlphPartRep(uint * seq, size_t n, uint cut, uint cutOffsets,
						SequenceBuilder * lenIndexBuilder, 
						SequenceBuilder * seqsBuilder, 
						SequenceBuilder * seqFirstClasses);

			SequenceAlphPartRep(uint * seq, size_t n, uint _cut, uint cutOffsets,
						SequenceBuilder *lenIndexBuilder,
						SequenceBuilder *seqsBuilder,
						SequenceBuilder *seqFirstClasses,
						SequenceBuilder *recBuilder,
						int depth);

//			Sequence *decideCompressUnCompress(uint *seq, size_t len, SequenceBuilder *comp, SequenceBuilder *unComp, SequenceBuilder **seqb);

			SequenceAlphPartRep();

			virtual ~SequenceAlphPartRep();

			virtual void save(ofstream & fp) const;
			
			static SequenceAlphPartRep * load(ifstream & fp);

		protected:

			void build(uint * seq, size_t n, uint _cut, uint cutOffsets, SequenceBuilder *lenIndexBuilder,
				SequenceBuilder *seqsBuilder, SequenceBuilder *seqFirstClasses, SequenceBuilder *recBuilder, int depth);
			/**
			 * c: length of the sequence C after applying Repair
			 * */
			uint c;
			
			DictionaryRP *dictionary;
			
			void sortSymbols(uint *seq, uint n, uint sigma, uint *occ, pair<float,uint> *&, Repair *rp=NULL);

			virtual inline uint group(uint pos, uint cut) const;
			virtual inline uint offset(uint pos, uint cut, uint group) const;

	};

};
#include <grammar/DictionaryRP.h>
#endif							 /* _SEQUENCEALPHPARTREP_H */
