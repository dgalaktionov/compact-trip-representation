/*  SequenceBuilderSequenceRepair.h
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

#include <bitsequence/BitSequenceBuilder.h>


#ifndef BIT_SEQUENCEBUILDERSEQUENCEREPAIR_SN_H
#define BIT_SEQUENCEBUILDERSEQUENCEREPAIR_SN_H

namespace cds_static
{
	class SequenceBuilder;
	class PermutationBuilder;
	class DictionaryRPBuilder;
	class BitSequenceBuilderRPSN : public BitSequenceBuilder
	{
		public:
			BitSequenceBuilderRPSN(BitSequenceBuilder * bsb,
								PermutationBuilder *, uint samplingC, uint delta, uint ss_rate,DictionaryRPBuilder *dicBuilder=nullptr);
			//this constructor binds the object it can create to the rp given
			BitSequenceBuilderRPSN(Repair *rp, BitSequenceBuilder * bsb,
								PermutationBuilder *, uint samplingC, uint delta, uint ss_rate,DictionaryRPBuilder *dicBuilder=nullptr);
			BitSequenceBuilderRPSN(BitSequenceBuilder * bsb,
									uint samplingC, uint delta, uint ss_rate,DictionaryRPBuilder *dicBuilder=nullptr);
			virtual ~BitSequenceBuilderRPSN();
			virtual BitSequence * build(uint * seq, size_t len) const;
			virtual BitSequence * build(const BitString & bs) const;
		protected:
			BitSequenceBuilder * bsb;
			PermutationBuilder *pb;
			uint samplingC;
			uint delta;
			uint ss_rate;
			Repair *rp;
			DictionaryRPBuilder *dicBuilder;
	};
};
#endif
