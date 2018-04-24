/*  SequenceBuilderWaveletTreeNoptrs.cpp
 * Copyright (C) 2010, Francisco Claude, all rights reserved.
 *
 * Francisco Claude <fclaude@cs.uwaterloo.ca>
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

#include <sequence/SequenceBuilderWaveletTreeNoptrs.h>
#include <stdexcept>

namespace cds_static
{

	SequenceBuilderWaveletTreeNoptrs::SequenceBuilderWaveletTreeNoptrs(BitSequenceBuilder * _bsb, Mapper * _am) {
		this->bsbs.push_back(_bsb);
		this->am = _am;
		am->use();
	}
	
	SequenceBuilderWaveletTreeNoptrs::SequenceBuilderWaveletTreeNoptrs(vector<BitSequenceBuilder*> & _bsbs,Mapper * _am) {
		this->bsbs = _bsbs;
		for (auto it:this->bsbs)
			it->use();
		this->am = _am;
		am->use();
	}

	SequenceBuilderWaveletTreeNoptrs::~SequenceBuilderWaveletTreeNoptrs() {
		for (auto it:this->bsbs)
			it->unuse();
		am->unuse();
	}

	Sequence * SequenceBuilderWaveletTreeNoptrs::build(uint * sequence, size_t len) {
		if (this->bsbs.size()==1){
			return new WaveletTreeNoptrs(sequence, len, bsbs[0], am);
		}else{
			return new WaveletTreeNoptrs(sequence, len, bsbs, am);
		}
	}

	Sequence * SequenceBuilderWaveletTreeNoptrs::build(const Array & seq) {
		if (bsbs.size()==1){
			return new WaveletTreeNoptrs(seq, bsbs[0], am);
		}else{
			throw runtime_error("Building a WaveletMatrix using several bitmaps per level (>1) is only implemented when the input sequence is an array of uints");
		}
	}
};
