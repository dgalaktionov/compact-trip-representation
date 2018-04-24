/* WaveletTreeNoptrs.h
 * Copyright (C) 2008, Francisco Claude, all rights reserved.
 *
 * WaveletTreeNoptrs definition
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

#ifndef _SEQUENCE_WT_RP_H
#define _SEQUENCE_WT_RP_H

#ifndef MAX_CODE_LEN_TERMINALS
#define MAX_CODE_LEN_TERMINALS 8192
#endif
#include <iostream>
#include <cassert>
#include <utils/libcdsBasics.h>
#include <bitsequence/BitSequence.h>
#include <bitsequence/BitSequenceBuilder.h>
#include <sequence/Sequence.h>
#include <mapper/Mapper.h>
#include <grammar/Repair.h>
#include <utils/DisjointSet2.h>
#include <utils/DisjointSet.h>
#include <sequence/Tuple.h>

using namespace std;
using namespace cds_utils;

namespace cds_static
{

/**
			 * Class that represents the info. about each terminal symbol of
			 * the original sequence.
			 * */
	class terminal{
	public:
		terminal(){id=-1;code=NULL;len=0;noccs=0;}
		terminal(uint _id,uint _noccs){
			id=_id;
			code=new uint[MAX_CODE_LEN_TERMINALS/(sizeof(uint)*8)];
			for (uint i=0;i<MAX_CODE_LEN_TERMINALS/(sizeof(uint)*8);i++)
				code[i]=0;
			len=0;
			noccs=_noccs;
		}
		terminal(uint _id,uint _len, uint _noccs){
			id=_id;
			code=new uint[MAX_CODE_LEN_TERMINALS/(sizeof(uint)*8)];
			for (uint i=0;i<MAX_CODE_LEN_TERMINALS/(sizeof(uint)*8);i++)
				code[i]=0;
			len=_len;
			noccs=_noccs;
		};
		terminal(uint _id, uint *_code, uint _len, uint _noccs){
			id=_id;
			code = _code;
			len=_len;
			noccs=_noccs;
		};
		friend ostream & operator<<(ostream &os, const terminal& p);
		~terminal(){};
		uint id;
		uint *code;
		uint len;
		uint noccs;

	private:
	};

	class SequenceWTRP : public Sequence
	{
		public:

			/** Builds a Wavelet Tree for the string
			 * pointed by symbols assuming its length
			 * equals n and uses bmb to build the bitsequence
			 * @param bmb builder for the bitmaps in each level.
			 * @param am alphabet mapper (we need all symbols to be used).
			 * */
			SequenceWTRP(uint * symbols, size_t n, int limit, BitSequenceBuilder * bmb, Mapper * am, bool deleteSymbols = false);
			
			/** Destroys the Wavelet Tree */
			virtual ~SequenceWTRP();

			virtual size_t rank(uint symbol, size_t pos) const;
			virtual size_t select(uint symbol, size_t j) const;
			virtual uint access(size_t pos) const;
			virtual size_t getSize() const;
			virtual void save(ofstream & fp) const;
			static SequenceWTRP * load(ifstream & fp);

		protected:
			SequenceWTRP();

			Mapper * am;

			BitSequence **bitstring;

			/** Length of the string. */
			size_t n;

			/** Height of the Wavelet Tree. */
			uint height, max_v;
			uint *C, *OCC;
			

			/** Obtains the maximum value from the string
			 * symbols of length n */
			uint max_value(uint *symbols, size_t n);

			/** How many bits are needed to represent val */
			uint bits(uint val);

			/** Returns true if val has its ind-th bit set
			 * to one. */
			bool is_set(uint val, uint ind) const;

			/** Sets the ind-th bit in val */
			uint set(uint val, uint ind) const;

			/** Recursive function for building the Wavelet Tree. */
			void build_level(uint **bm, uint *symbols, uint length, uint level, uint *sizeBitmaps, tuple *);
			
			


			/**
			 * Internal class that represents a UNION-FIND-NODE
			 * */
			class ufn {
				public:
				ufn(uint _id){
					term=NULL;
					nterm=0;
					id = _id;
				}
				ufn(terminal *_term){
					term  = _term;
					nterm = 1;
					id    = term->id;
				}
				ufn(uint _id, uint _nterm){
					id = _id;
					nterm = _nterm;
					term = new terminal[nterm];
				}
				ufn(uint _id, uint _nterm, uint _nsymb){
					id = _id;
					nterm = _nterm;
					term = new terminal[nterm];
				}
				~ufn(){
					if (term) delete [] term;
				};			
				/**
				 * ID of the non-terminal or of the terminal symbol
				 * */
				uint id;
				/**
				 * number of terminal symbols stored in term
				 * */
				uint nterm;
				
				terminal *term;
			
				private:
			};
		//for bottom up
			terminal** terms;
			uint maxCodeLen;
			uint *nCodesLevel;
			uint newSets;
			static const int maxLevelTest = 7;
			void build_level(uint **bm, uint *symbols, uint length, uint level, uint *sizeBitmaps, double);
			void buildCodes(uint total, uint *lrs, DisjointSet<ufn*> *ufds, uint alpha, uint *occs, uint grammarDepth, uint &order);//, uint, uint &, uint *, uint *);
			void buildCodesBottomUp(uint total, uint *lrs, DisjointSet2<ufn*> *ufds, uint alpha);
			void buildCodesTD(uint total, uint *lrs, Repair *rp,uint *input, uint inputLen,vector<bool> &codes);
			void buildCodesTD(uint *rules, uint total,Repair *rp, uint *input, uint inputLen, vector<bool> &codes, double factor);
			void buildRecTopDown(uint *symbols, size_t n, BitSequenceBuilder * bmb,uint **bmps, uint *levelPointer, uint level, double factor);
			void buildRecTopDown(uint *symbols, size_t n, BitSequenceBuilder * bmb, uint **bmps, uint *pointerLevel, uint level, double factor, pair<uint,uint> *my_codes, uint nc);
			bool testMarkTerms(uint *input, uint ini, uint fin, uint *bmp, uint nbits, Repair *rp, uint *voc);
			uint markTerms(uint *input, uint ini, uint fin, uint *bmp, uint nbits, Repair *rp, uint *voc);
			uint markTerms(uint *input, uint ini, uint fin, Repair *rp, uint *voc);
			uint markTerms(uint rule, Repair *rp, uint *voc);
			uint intersection(uint *voc1, uint *voc2, uint n1, uint n2);
			bool testIntesection();
			uint mergeLists(uint *list1, uint *list2, uint &n1, uint n2);
			bool testMerge();
			void countOccsRules(uint rule, Repair *rp, uint *occs_rules);
			bool statistics(DisjointSet2<ufn*> *ufds, Repair *rp, uint sigma, uint limitSets,uint *occs);
			uint regularMerge(uint set1, uint set2,DisjointSet2<ufn*> *ufds,ufn *new_repr);
			uint recInsert(uint node, uint new_set, bool new_set_left, bool head, DisjointSet2<ufn*> *ufds,ufn *new_repr);
			uint finishRecursion(uint node, uint smallerSet, bool new_set_left, bool head, DisjointSet2<ufn*> *ufds, ufn *new_repr);
			void copyTerminals(uint n1,uint n2,uint dest,DisjointSet2<ufn*> *ufds);
			bool testCodeLengths(uint set_repr, DisjointSet2<ufn*> *);
			uint testConsistencyNTerms(uint set_repr, DisjointSet2<ufn*> *);
			bool balanceCondition(uint nt1, uint nt2);
			uint getValidSet(uint ,DisjointSet2<ufn*> *);
			void printRule(uint rule, DisjointSet2<ufn*>*,uint);
			uint getMaxCodeLength(uint rule, DisjointSet2<ufn*>*ufds);
			void getListCodesDFS(uint rule, DisjointSet2<ufn*>*ufds, uint *&list);
			uint testCountLeaves(uint rule, DisjointSet2<ufn*>*);
			void getListCodes(uint rule, DisjointSet2<ufn*>*ufds, tuple *&list, uint &code, uint level);
			void getSymbols(uint rule, Repair *rp, uint *symbols, uint &nsymb);
	};


};
#endif
