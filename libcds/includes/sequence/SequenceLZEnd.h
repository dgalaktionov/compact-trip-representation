/* SequenceLZEnd.h
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

#ifndef _SEQUENCE_LZEND_
#define _SEQUENCE_LZEND_

#include <iostream>
#include <cassert>
#include <utils/libcdsBasics.h>
#include <bitsequence/BitSequenceBuilder.h>
#include <sequence/Sequence.h>
#include <mapper/Mapper.h>
#include <direct_access/DAC.h>
#include <permutation/PermutationBuilder.h>
#include <text_indexes/fmi.h>
#include <rmq/RMQ.h>
#include <utils/BinarySearchTree.h>
#include <utils/StringUtils.h>
#include <assert.h>

using namespace std;

namespace cds_static
{
	#define SEQUENCE_LZEND_BASIC_HDR 1
	#define SEQUENCE_LZEND_BASE_HDR 2

	class fmi;

	class SequenceLZEnd : public Sequence
	{

//		friend class LZBaseLength;
//        friend class LZBaseLimitHops;
//        friend class LZBaseSampleOnN;
//        friend class LZBaseCopyOptimal;
		public:

			/** Destroys the Wavelet Tree */
			virtual ~SequenceLZEnd();
			SequenceLZEnd(uint *input, size_t n, uint* dic, bool del=false);
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

			virtual uint extract(uint sp, uint ep, uint *&buffer);

			uint getZ(){return z;}

		protected:
			
			void build(uint *input, size_t n, uint* dic, bool del, Array *base);
			void build(uint *_input, size_t n, SequenceLZEnd *parsed, bool del);
			void buildFmRMQ(uint *input, size_t n, fmi **fm, RMQ **rmq);
			SequenceLZEnd();
			void parse(uint *input, fmi *fm, RMQ* rmq, uint *A, uint *invA, std::tuple<uint,uint,uint> *&, uint **);
			
			void printInput(uint *input, fmi *, uint limit);
			/*Receives a sequence of triplets (p,l,c) and build a compressed repres.
			 for LZEnd (a bitmap indication the frase endings...)*/
			void compactDictionary(uint *input, std::tuple<uint,uint,uint> *, uint *);
			/*Stores the rank for each symbol up to each phrase ending (included) */
			virtual uint sampleDictionary(uint *input, BitSequence *, uint **&);
			/*Defined a super sampling factor (ss), super-sample the vector os samples 
				and stores the samples in a DAC*/
			virtual uint compressDictionary(uint *input, uint **, uint ss);

			bool testCompressDictionary(uint nsamples, uint nssamples, uint **samplesOriginal, uint **samples, uint **, uint ss);

			bool testParsing(uint *input);
			virtual uint addImprovements(uint *input);

			void computePhraseLengths(BitSequence *bitmap);

			BitSequence* getEndings(){return endings;}

			//Dictionary
				uint z;//number of rules
				BitSequence *endings;
				Array *refs;
				uint *phraseLengths;
				uchar *trailing;
				uint max_v;
				uint ss_rate;
				uint dicSize;
				uint *dic;
				uint *invDic;
				
	};
};

#include <sequence/SequenceLZEndBase.h>

#endif
