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

#include <sequence/SequenceBuilderSequenceRepairSC.h>

namespace cds_static
{

	SequenceBuilderSequenceRepairSC::SequenceBuilderSequenceRepairSC(BitSequenceBuilder * bsb, 
					Mapper * am,PermutationBuilder *pb, uint samplingC, uint delta, uint _ss_rate, DictionaryRPBuilder *dic) {
		this->rp = nullptr;
		this->bsb = bsb;
		this->am = am;
		this->pb = pb;
		this->samplingC = samplingC;
		this->delta = delta;
		this->ss_rate = _ss_rate;
        this->dic = dic;
		bsb->use();
		am->use();
	}

	SequenceBuilderSequenceRepairSC::SequenceBuilderSequenceRepairSC(BitSequenceBuilder * bsb, Repair *_rp,
																	 Mapper * am,PermutationBuilder *pb, uint samplingC, uint delta, uint _ss_rate, DictionaryRPBuilder *dic) {
		this->rp = rp;
		this->bsb = bsb;
		this->am = am;
		this->pb = pb;
		this->samplingC = samplingC;
		this->delta = delta;
		this->ss_rate = _ss_rate;
        this->dic = dic;
		bsb->use();
		am->use();
	}

	SequenceBuilderSequenceRepairSC::~SequenceBuilderSequenceRepairSC() {
		bsb->unuse();
		am->unuse();
		if (rp) delete rp;
	}

	Sequence * SequenceBuilderSequenceRepairSC::build(uint * sequence, size_t len) {
		if (rp){
			return new SequenceRepairSC(sequence, len, rp,bsb, am, pb, samplingC, delta, ss_rate,dic);
		}else {
			return new SequenceRepairSC(sequence, len, bsb, am, pb, samplingC, delta, ss_rate,dic);
		}
	}

	Sequence * SequenceBuilderSequenceRepairSC::build(const Array & seq) {
		return new SequenceRepairSC(seq, bsb, am, pb, samplingC, delta, ss_rate,dic);
	}
};
