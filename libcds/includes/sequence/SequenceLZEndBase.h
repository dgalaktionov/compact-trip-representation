/* SequenceLZEndBase.h
 * Copyright (C) 2014-current_year, Alberto Ordóñez, all rights reserved.
 *
 * Sequence encoded with Repair which stores counters to solve rank/select/access operations
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

#ifndef _SEQUENCE_LZEND_BASE_
#define _SEQUENCE_LZEND_BASE_

#include <sequence/SequenceLZEnd.h>
#include <sequence/LZBaseStrategy.h>

#ifndef ENDING_BITMAP
#define ENDING_BITMAP 0
#endif

#ifndef LZ_SAMPLE_C
#define LZ_SAMPLE_C 1
#endif
using namespace std;

namespace cds_static
{
	class LZBaseStrategy;

	class SequenceLZEndBase : public SequenceLZEnd
	{
		friend class LZBaseLength;
		friend class LZBaseLimitHops;
		friend class LZBaseSampleOnN;
		friend class LZBaseCopyOptimal;
		public:

			/** Destroys the Wavelet Tree */
			virtual ~SequenceLZEndBase();
			SequenceLZEndBase(uint *input, size_t n, uint* dic, BitSequenceBuilder *bsb, SequenceBuilder *sb,  LZBaseStrategy *_strategy,  uint sampleC, uint superSampleC, bool del=false);
			SequenceLZEndBase(uint *input, size_t n, BitSequenceBuilder *bsb, SequenceBuilder *sb,  LZBaseStrategy *_strategy,  uint sampleC, uint superSampleC, SequenceLZEnd *parsed, bool del=false);
			SequenceLZEndBase(uint *_input, size_t n, uint *_dic,  Array *base, BitSequenceBuilder *bsb, SequenceBuilder *sb, uint _beta, uint _sampleC,  uint superSampleC, bool del);
			virtual size_t rank(uint symbol, size_t pos) const ;
			virtual size_t select(uint symbol, size_t j) const;
			virtual uint access(size_t pos) const;
			/**
			 * In case of sigma==2, r will contain the rank0(pos).
			 * */
			virtual uint access(size_t pos, uint &r) const;
			virtual size_t getSize() const;
			virtual void save(ofstream & fp) const;
			static Sequence * load(ifstream & fp);

			virtual uint extract(uint sp, uint len, uint *&buffer);
			
			LZBaseStrategy* getStrategy();
			LZBaseStrategy* setStreategy(LZBaseStrategy *strategy);

		protected:
			SequenceLZEndBase();
			size_t getPointerRule(size_t index) const;

			bool testExtractPhrases(uint *input);
			size_t interanlRank(uint symbol, size_t pos, uint &end, uint &to, uint &acum, uint phrase, bool verbose) const;
			size_t interanlRankOnN(uint symbol, size_t pos, uint &end, uint pos_end, uint &to, uint &acum, uint phrase_pos_index,bool first) const;
			virtual uint addImprovements(uint *input);
			/*All those phrases with length <=beta are added to the base sequence*/
			uint extractBaseByLength(uint *input);

			/*All those symbols that are copied more than beta times are added to the base sequence*/
			uint extractBaseByHeight(uint *input);

			/*Stores the rank for each symbol up to each phrase ending (included) */
			virtual uint sampleDictionary(uint *input, BitSequence *, uint **&);

			virtual uint compressDictionary(uint *input, uint **, uint ss);

			uint accessSample(size_t pos, uint symbol, uint &, uint &to_position, uint &index_to) const;
			uint accessSampleOnN(size_t pos, uint symbol, uint &s, uint &sample_position, uint &index_to, uint &index_end) const;
			size_t rulesUpTo(size_t index) const;

			size_t getSizeVerbatim() const;

			uint sampleDictionaryOnN(uint *input, BitSequence *marks, uint **&_samples);
			size_t analyzeSampling() const;
			LZBaseStrategy *strategy;

			//Dictionary
			SequenceBuilder *seqBuilder;
			BitSequenceBuilder *bitSequenceBuilder;
			Sequence *base;
			BitSequence *explicity;
			DirectAccess *pointersBase;
			DirectAccess **samples;
			uint **samplesVerb;
			uint nsamples;
			//uint nssamples;
			uint beta;//parameter to select what phrases are stored directly
			uint *input_seq;
			uint nRulesNotExplicit;
			bool verbose_test;
		
			uint sampleRate;
			//the marks of rule ends can be stored in a bitVector
			//or in an Array. We keep both an use what we want.
			//To decide there is an macro ENDING_BITMAP
			Array *endingsArray;
	};
};
#endif
