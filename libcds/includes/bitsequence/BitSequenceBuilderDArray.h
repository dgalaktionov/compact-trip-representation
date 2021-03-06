/*  BitSequenceBuilderDArray.h
 * Copyright (C) 2008, Francisco Claude, all rights reserved.
 *
 * Rodrigo Canovas <rcanovas@dcc.uchile.cl>
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

#ifndef BITSEQUENCEBUILDERDARRAY_H
#define BITSEQUENCEBUILDERDARRAY_H

#include <utils/libcdsBasics.h>
#include <bitsequence/BitSequenceDArray.h>
#include <bitsequence/BitSequenceBuilder.h>

namespace cds_static
{
	/** Base class for BitSequence builders, it defines the build function
	 * that takes only a bitmap. The parameters for construction are can
	 * be set in any way by the builder, but none are received when
	 * the actual building takes place.
	 *
	 * @author Rodrigo Canovas
	 */
	class BitSequenceBuilderDArray : public BitSequenceBuilder
	{
		public:
			BitSequenceBuilderDArray();
			virtual ~BitSequenceBuilderDArray();
			virtual BitSequence * build(uint * bitseq, size_t len) const;
			virtual BitSequence * build(const BitString & bs) const;
	};
};
#endif
