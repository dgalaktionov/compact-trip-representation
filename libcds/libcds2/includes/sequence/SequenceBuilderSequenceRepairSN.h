/*  SequenceBuilderWaveletTreeNoptrs.h
 * Copyright (C) 2014-current_year, Alberto Ordóñez, all rights reserved.
 *
 * Alberto Ordóñez <alberto.ordonez@udc.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <utils/libcdsBasics.h>
#include <mapper/Mapper.h>
#include <sequence/SequenceBuilder.h>
#include <bitsequence/BitSequenceBuilder.h>
#include <permutation/PermutationBuilder.h>

#ifndef SEQUENCEBUILDERSEQUENCEREPAIRSN_H
#define SEQUENCEBUILDERSEQUENCEREPAIRSN_H

namespace cds_static
{
	class SequenceBuilderSequenceRepairSN : public SequenceBuilder
	{
		public:
			SequenceBuilderSequenceRepairSN(BitSequenceBuilder * bsb, Mapper * am,
								PermutationBuilder *, uint samplingC, uint delta, uint ss_rate, DictionaryRPBuilder *dic=nullptr);
			//This constructor binds the the object to the collection compressed with the rp given
			SequenceBuilderSequenceRepairSN(BitSequenceBuilder * bsb, Repair *rp, Mapper * am,
								PermutationBuilder *, uint samplingC, uint delta, uint ss_rate, DictionaryRPBuilder *dic=nullptr);
			virtual ~SequenceBuilderSequenceRepairSN();
			virtual Sequence * build(uint * seq, size_t len);
			virtual Sequence * build(const Array & seq);

		protected:
			BitSequenceBuilder * bsb;
			Mapper * am;
			PermutationBuilder *pb;
			uint samplingC;
			uint delta;
			uint ss_rate;
			Repair *rp;
			DictionaryRPBuilder *dic;
	};
};
#endif
