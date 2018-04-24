
/* static_sequence_builder_wvtree.cpp
 * Copyright (C) 2012, Alberto Ordóñez, all rights reserved.
 *
 * Sequence builder wavelet tree
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

#include <sequence/SequenceBuilderCanonicalWTNoptrs.h>

namespace cds_static{

SequenceBuilderCanonicalWTNoptrs::SequenceBuilderCanonicalWTNoptrs(vector<BitSequenceBuilder *> &_bsbs, Mapper * am, MappingBuilder *msb) {
  this->bsbs = _bsbs;
  this->wc = nullptr;
  this->am = am;
  this->msb = msb;
}

SequenceBuilderCanonicalWTNoptrs::~SequenceBuilderCanonicalWTNoptrs() {

}
Sequence * SequenceBuilderCanonicalWTNoptrs::build(const Array & seq){
  return nullptr;
}

Sequence * SequenceBuilderCanonicalWTNoptrs::build(uint * seq, size_t len) {

	return new CanonicalWTNoptrs(seq,len,bsbs,am,msb);
}

Sequence * SequenceBuilderCanonicalWTNoptrs::build(uint * seq, size_t len, wt_coder_huff *coder) {

	return new CanonicalWTNoptrs(seq,len,coder,bsbs,am,msb);
}

};

