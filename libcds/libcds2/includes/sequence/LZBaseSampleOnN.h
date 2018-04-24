/* LZBaseSampleOnN.h
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

#ifndef _LZ_BASE_SAMPLE_ON_N_H
#define _LZ_BASE_SAMPLE_ON_N_H

#include <sequence/LZBaseStrategy.h>

using namespace std;

namespace cds_static
{

	class LZBaseSampleOnN : public LZBaseStrategy
	{
		
		public:
			LZBaseSampleOnN(uint samplingStep, uint phraseLength);
			
			virtual ~LZBaseSampleOnN();
		
			virtual uint computeBase(uint *input, size_t len, SequenceLZEndBase *seq);

		protected: 
			uint samplingStep;//every z positions (moving from phrase to phrase, we take a sample). == maxLength
			uint phraseLength;
	};
};

#endif
