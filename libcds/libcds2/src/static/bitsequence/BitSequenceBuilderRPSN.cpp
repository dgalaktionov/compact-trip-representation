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

#include <bitsequence/BitSequenceBuilderRPSN.h>
#include <sequence/SequenceBuilder.h>
#include <permutation/PermutationBuilder.h>

namespace cds_static
{

	BitSequenceBuilderRPSN::BitSequenceBuilderRPSN(Repair *rp, BitSequenceBuilder * bsb, PermutationBuilder *pb, uint samplingC, uint delta, uint _ss_rate,DictionaryRPBuilder *_dicBuilder) {
		this->bsb = bsb;
		this->pb = pb;
		this->samplingC = samplingC;
		this->delta = delta;
		this->ss_rate = _ss_rate;
		this->rp = rp;
        this->dicBuilder = _dicBuilder;
		bsb->use();
	}
	BitSequenceBuilderRPSN::BitSequenceBuilderRPSN(BitSequenceBuilder * bsb, PermutationBuilder *pb, uint samplingC, uint delta, uint _ss_rate,DictionaryRPBuilder *_dicBuilder) {
		this->rp = nullptr;
		this->bsb = bsb;
		this->pb = pb;
		this->samplingC = samplingC;
		this->delta = delta;
		this->ss_rate = _ss_rate;
        this->dicBuilder = _dicBuilder;
		bsb->use();
	}
	BitSequenceBuilderRPSN::BitSequenceBuilderRPSN(BitSequenceBuilder * bsb, uint samplingC, uint delta, uint _ss_rate,DictionaryRPBuilder *_dicBuilder) {
		this->rp = nullptr;
		this->bsb = bsb;
		this->pb = new PermutationBuilderMRRR(5,new BitSequenceBuilderRG(10));
		this->samplingC = samplingC;
		this->delta = delta;
		this->ss_rate = _ss_rate;
        this->dicBuilder = _dicBuilder;
		bsb->use();
	}

	BitSequenceBuilderRPSN::~BitSequenceBuilderRPSN() {
		bsb->unuse();
		if (rp) delete rp;
	}

	BitSequence * BitSequenceBuilderRPSN::build(uint * bitseq, size_t len) const{
		if (this->rp == nullptr){
			return new BitSequenceRPSN(bitseq,len, bsb, pb, samplingC, delta, ss_rate, dicBuilder,true);
		}else{
			return new BitSequenceRPSN(bitseq,len, rp, bsb, pb, samplingC, delta, ss_rate, dicBuilder,true);
		}
	}

	BitSequence * BitSequenceBuilderRPSN::build(const BitString & bs) const{
		//~ return new SequenceRepairSC(seq, bsb, am, pb, samplingC, delta, ss_rate);
		return NULL;
	}
};
