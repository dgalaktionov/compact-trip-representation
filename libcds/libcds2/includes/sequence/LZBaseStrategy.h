/* LZBaseStrategy.h
 * Copyright (C) 2014-current_year, Alberto Ordóñez, all rights reserved.
 *
 * Streategies to obtain the base sequence from a LZ Parsing
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

#ifndef _LZ_BASE_STRATEGY_
#define _LZ_BASE_STRATEGY_

#include <sequence/SequenceLZEnd.h>

using namespace std;

namespace cds_static
{

	class SequenceLZEndBase;

	class LZBaseStrategy 
	{
		
		public:
			LZBaseStrategy();
			/** Destroys the Wavelet Tree */
			virtual ~LZBaseStrategy();
			
			virtual uint computeBase(uint *input, size_t len, SequenceLZEndBase *seq)=0;
	};
};

#include <sequence/LZBaseLength.h>
#include <sequence/LZBaseLimitHops.h>
#include <sequence/LZBaseSampleOnN.h>
#include <sequence/LZBaseCopyOptimal.h>
#endif
