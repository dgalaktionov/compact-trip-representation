/*  SequenceBuilderSequenceWTRP.h
 * Copyright (C) 2014, Alberto Oróñez, all rights reserved.
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

#include <sequence/SequenceBuilderSequenceWTRP.h>

namespace cds_static
{

	SequenceBuilderSequenceWTRP::SequenceBuilderSequenceWTRP(SequenceBuilder * groupIndexBuilder, 
			SequenceBuilder * indexBuilder, 
			SequenceBuilder * firstClasses, 
			uint cut, uint cutOffsets) {
		this->groupIndexBuilder = groupIndexBuilder;
		this->indexBuilder = indexBuilder;
		this->firstClassesBuilder = firstClasses;
		this->cut = cut;
		this->cutOffsets = cutOffsets;
		groupIndexBuilder->use();
		indexBuilder->use();
	}

	SequenceBuilderSequenceWTRP::~SequenceBuilderSequenceWTRP() {
		groupIndexBuilder->unuse();
		indexBuilder->unuse();
	}

	Sequence * SequenceBuilderSequenceWTRP::build(uint * sequence, size_t len) {
		cerr << "TODO SeqBuildSeqRPwt"<<endl;
		return NULL;
	}

	Sequence * SequenceBuilderSequenceWTRP::build(const Array & seq) {
		cerr << "TODO SeqBuildSeqRPwt"<<endl;
		return NULL;
	}
};
