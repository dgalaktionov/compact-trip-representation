/*  SequenceBuilderSequenceRepair.cpp
 * Copyright (C) 2014-current_year, Alberto Ordóñez, all rights reserved.
 *
 * Alberto Ordóñez <alberto.ordonez@udc.e
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

#include <bitsequence/BitSequenceBuilderRPSC.h>
#include <sequence/SequenceBuilder.h>
#include <permutation/PermutationBuilder.h>

namespace cds_static
{


	BitSequenceBuilderRPSC::BitSequenceBuilderRPSC(Repair *rp, BitSequenceBuilder * bsb, PermutationBuilder *pb, uint samplingC, uint delta, uint _ss_rate,DictionaryRPBuilder *dicBuilder) {
		this->rp = rp;
		this->bsb = bsb;
		this->pb = pb;
		this->samplingC = samplingC;
		this->delta = delta;
		this->ss_rate = _ss_rate;
        this->dicBuilder = dicBuilder;
		bsb->use();
	}

	BitSequenceBuilderRPSC::BitSequenceBuilderRPSC(BitSequenceBuilder * bsb, PermutationBuilder *pb, uint samplingC, uint delta, uint _ss_rate,DictionaryRPBuilder *dicBuilder) {
		this->rp = nullptr;
		this->bsb = bsb;
		this->pb = pb;
		this->samplingC = samplingC;
		this->delta = delta;
		this->ss_rate = _ss_rate;
        this->dicBuilder = dicBuilder;
		bsb->use();
	}

	BitSequenceBuilderRPSC::BitSequenceBuilderRPSC(BitSequenceBuilder * bsb, uint samplingC, uint delta, uint _ss_rate,DictionaryRPBuilder *dicBuilder) {
		this->bsb = bsb;
		this->pb = new PermutationBuilderMRRR(5,new BitSequenceBuilderRG(10));
		this->samplingC = samplingC;
		this->delta = delta;
		this->ss_rate = _ss_rate;
		this->rp = nullptr;
        this->dicBuilder = dicBuilder;
		bsb->use();
	}

	BitSequenceBuilderRPSC::~BitSequenceBuilderRPSC() {
		bsb->unuse();
		if (rp) delete rp;
	}

	BitSequence * BitSequenceBuilderRPSC::build(uint * bitseq, size_t len) const{
		if (this->rp== nullptr) {
			return new BitSequenceRPSC(bitseq, len, bsb, pb, samplingC, delta, ss_rate,dicBuilder);
		}else{
			return new BitSequenceRPSC(bitseq, len, rp, bsb, pb, samplingC, delta, ss_rate,dicBuilder);
		}
	}

	BitSequence * BitSequenceBuilderRPSC::build(const BitString & bs) const{
		//~ return new SequenceRepairSC(seq, bsb, am, pb, samplingC, delta, ss_rate);
		return NULL;
	}
};
