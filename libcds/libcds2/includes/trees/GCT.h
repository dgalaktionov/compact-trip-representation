/*  
 * Copyright (C) 2014-current_year, Alberto Ordonez, all rights reserved.
 *
   Alberto Ordonez <alberto.ordonez@udc.es>
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

#ifndef _GCT_H
#define _GCT_H

#include <utils/Array.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>
#include <sys/times.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <utils/HashMap.h>
#include <grammar/DictionaryRP.h>
#include <direct_access/DAC.h>
#include <utils/Array.h>
#include <bitsequence/BitSequence.h>
#include <grammar/Repair.h>
#include <trees/Parentheses.h>

#ifndef MAX_DEPTH
#define MAX_DEPTH 1024
#endif

#ifndef MIN_EXCESS
#define MIN_EXCESS 1
#endif

using namespace cds_static;

namespace cds_static{

    class GCT:public Parentheses{

     public:
                    //aordonez
                    GCT(uint _delta, uint samplingC, uint *input, char *inputGrammar);
//                    GCT(uint delta, uint samplingC, uint *bmp_input, ulong _len);
                    static GCT* newGCTFromRevBitmap(uint _delta, uint _samplingC, uint *bmp_input, ulong _len, bool candel=false);
                    static GCT* GCTFromBitmap(uint _delta, uint _samplingC, uint *_input, ulong _len);
                    size_t rank0(size_t pos);
                    size_t rank0(size_t pos,size_t &excess);
               
                    size_t rank1(size_t pos);
                    size_t rank1(size_t pos,size_t &excess);
                    
                    size_t select0(size_t i,size_t &excess);
                    size_t select1(size_t i,size_t &excess);
                    
                    size_t access(size_t pos, int &excess);
                   
                    size_t access_and_leaf_rank(size_t pos, uint &leaf_rank);
                    
                    // size_t brLeafRank(size_t pos);
                    // size_t brLeafSelect(size_t i);
                    size_t getExcess(size_t pos);
                    // size_t fwd_search(size_t i, int e);
                    // size_t bwd_search(size_t i, int e);
                    size_t rmqi( size_t s, size_t t, size_t &m);
                    
                   
                    // size_t inorder(size_t v);
                    
                    // bool isLeaf(size_t pos, uint &leaf_rank);


                    virtual ulong fwdExcess(uint s, uint e);
                    virtual ulong bwdExcess( uint s, int e);
                    virtual ulong rmq( uint s, int t, int &m);
                    virtual ulong depth( uint s);
                    virtual ulong rankOpen( uint s);
                    virtual ulong rankClose( uint s);
                    virtual ulong selectOpen( uint s);
                    virtual ulong selectClose( uint s);
                    virtual ulong root_node();
                    virtual ulong inspect(uint s);
                    virtual ulong findClose(uint s);
                    virtual ulong findOpen(uint s);
                    virtual ulong parent(uint s);
                    virtual ulong parentClose(uint s);
                    virtual ulong enclose(uint s);
                    virtual ulong levelAncestor(uint s, uint d);
                    virtual ulong lca(uint a, uint b);
                    virtual ulong preorderRank(uint s);
                    virtual ulong preorderSelect(uint s);
                    virtual ulong postorderRank(uint s);
                    virtual ulong isLeaf(uint s);
                    virtual ulong subtreeSize(uint s);
                    virtual ulong firstChild(uint s);
                    virtual ulong nextSibling(uint s);
                    virtual ulong isAncestor(uint a, uint b);
                    virtual ulong prevSibling(uint x);
                    virtual ulong bwd_search(uint v, int e);
                    virtual ulong fwd_search(uint v, int e);
                    virtual bool isLeaf(uint s, uint &leaf_rank);
                    virtual ulong inorder(uint v);
                    virtual ulong brLeafSelect(uint v);
                    virtual ulong brLeafRank(uint v);
                    virtual ulong nextNode(ulong ith);
                    virtual ulong nextNodeBottomUp(ulong pos);
                    virtual ulong access(uint s);
                    virtual ulong excessAt(uint s);
                    virtual ulong leftmostLeaf(ulong pos);
                    virtual ulong lb(ulong pos);
                    virtual ulong rb(ulong pos);
                    virtual ulong id(ulong pos);
                    virtual ~GCT();
                    virtual  ulong getSize() const;
                    virtual  size_t getSizeVerbose() const;
                    virtual void save(ofstream &fp) const;
                    static GCT * load(ifstream &fp);
                    
            protected:
				void build(uint *, ulong len);

                    //int n; //Parentheses contains n (length of the original sequence)     
                    int c;  // real |C|
                    uint nb_C; //number of elements of C
                    uint delta; //sampling of the rules
                    int alph; // max used terminal symbol
                    uint *C; // compressed text
                    uint blogBlockSizeC;
                    uint blogBlockSizeCPlus1;
                    uint blockSizeC;//sampling pariod of C
                    uint b; //==log(R)
                    uint heapDepth;
                    uint *lengthsHeapLevels;
                    DictionaryRP *dic;
                    DAC* compressedSampleC_p;
                    DAC* compressedSampleC_o;
                    DAC* heapSampleC_e;
                    Array *heapSampleC_minE_array;
                    DAC *compressedRulesMinExcess;
                    DAC *compressedSampledLengths;
                    DAC *compressedSampledTotalExcess;
                    DAC *compressedRuleLeafs;
                    DAC* compressedSampleC_leafs;
                    uint *ruleFstBit;//contains the first bit of each rule
                    uint *ruleLstBit;//contains the last bit of each rule
                    BitSequence *bd;//used to mark the sampled rules
                    uint OVERLAP_LEAFS[4];//used for leafRank
                    int *EXCESS;//EXCESS[0]=-1 AND EXCESS[1]=1. 
                    
                    size_t stackRulePath[MAX_DEPTH];
                    int stackRuleValues[MAX_DEPTH];
                    
                    //from here, the data structures defined are only used on building time

                    float factor; // 1/extra space overhead; set closer to 1 for smaller and
                                          // slower execution
                    
                    
                    
                    ulong len; // Text length
                    
                    GCT();
               
				int getC(size_t i) const ;
                    
                    BitSequence *bsLeafsTest;
                    //aordonez
						//building
				size_t searchRuleDown_access(size_t rule, size_t acum_pos, size_t &acum_e, size_t target);
					
				//int computeExcessRankChunk_rmqi(unsigned char v,uint offset, uint lim, int *TABLE, int &mine, int &pos, int &acum_pos);
                    //int getTotalExcessRankTerminals_rmqi(uint rule, int *TABLE, uint lim, int &mine, int &pos, int &acum_pos);
                    
                    void countOccurrences(int rule, uint *original);
                    void printRule(int rule, int alpha);
                    int my_trim_rule(uint *&symbPerm,uint &newn, uint *rule_lengths, uint *newC);
                    int recursiveDelete(int rule, size_t index, HashMap *deleted, HashMap *trimed,uint &ndel, uint *);
                    void sampleC(uint *,uint *input);
                    void sampleLeafs(uint *,uint *input);
                    void sampleCExcess();
                    void countOnes(size_t rule, size_t subrule, size_t &ones);
                    void computeExcess(size_t rule, size_t subrule, int &minE, int &maxE, int &totalE, int &excess, int *, int*,uint *, uint &drule);
						//util queries
                    size_t getSampleR(size_t pos);
                    size_t getSampleL(size_t pos);
                    size_t getLength(uint rule);
                    
                    int getTotalExcess(uint rule);
                    int getMinExcess(uint rule,int &pos);
				int getMinTotalExcessAndLength(uint rule, int &mine, int &acum_excess,size_t &length);
				int getRuleAll(uint rule,int &mine,int &acum_excess,size_t &length,uint &leafs,uchar &fstb,uchar &lstb);
				int getRuleAllButMin(uint rule,int &acum_excess,size_t &length,uint &leafs,uchar &fstb,uchar &lstb);
				int getTotalExcessAndLength(uint rule,int &acum_excess,size_t &length);
				int getMinTotalExcess(uint rule, int &mine, int &acum_excess);
                    int getMinExcess(uint rule);
                    int getMaxExcess(uint rule);
                    int getSampleC_e(uint pos);
                    int getSampleC_leaf(uint pos);
                    int getSampleC_p(uint pos);
                    int getSampleC_o(uint pos);
                    uint getRuleLeaf(size_t rule, uint &leaf, uchar &fstb, uchar &lstb);
                    int getRuleLengthAndLeaf(uint rule,size_t &length,uint &leaves,uchar &fstb,uchar &lstb);
                    size_t getLeafFromPos(size_t pos, uint start, uint end);
                    int getSampleC_minE(uint pos);
                    
                    void getLeafsRec(int rule, int alpha, uint *rule_leafs, uint *first_bit, uint *last_bit, bool &fstb, bool &lstb, uint &leafs);
                    bool isBetterThanTrivialWay(char *, size_t compressed_size, uint *input, uint n);
                    size_t searchRuleDown_fwd(size_t rule, size_t e, int acum_excess, size_t acum_pos);
                    size_t searchRuleDown_bwd(size_t rule, size_t e, int acum_excess, size_t acum_pos);
                    size_t searchRMM_fwd(size_t sumL, int index, int &excess, size_t e);
                    size_t searchRMM_bwd(size_t sumL, int index, int &excess, size_t e);
                    size_t findPosMin(size_t rule);
                    size_t findPos_fwd(size_t i, size_t &sumL, int &p, int &excess, size_t &rule, size_t &posInC);
                    size_t findPos_fwd_with_e(size_t i, size_t &sumL, int &p, int &excess, size_t &rule, size_t &posInC, int e,size_t &);
                    size_t findPos_bwd(size_t i, size_t &sumL, int &p, int &excess, size_t &rule, size_t &posInC);
                    size_t findPos_bwd_with_e(size_t i, size_t &sumL, int &p, int &excess, size_t &rule, size_t &posInC,int e,size_t&);
                    size_t searchRMM_rmq(size_t p, int index, int &excess, int &minAcumE,size_t &acum_pos,size_t j, size_t &minE,size_t &posMin, size_t &ruleMin);
                    size_t findPosInCByPos(size_t pos, size_t &sumZeroes, size_t &sumLengths);
                    size_t findPosInCByZeroPos(size_t , size_t &sumZeroes, size_t &sumLengths);
                    size_t findPosInCByOnePos(size_t , size_t &sumZeroes, size_t &sumLengths);
                   
				size_t searchRuleDown_rmq(size_t rule, size_t e, int acum_excess, size_t acum_pos, size_t &minPos, size_t &minE, size_t &minRule);    
				size_t searchPartialRule_rmq(size_t rule, size_t j, int acum_excess, size_t acum_pos, size_t &minPos, size_t &minE, size_t &minRule,int &minAcumE);                
				
				size_t findPosMax(size_t rule);
				size_t iterate_fwd(int posInC, size_t acum_pos, int excess, size_t lim, int &minAcumE, size_t &minE, size_t &ruleMin, size_t &posMin);
				void iterate_EQUAL_fwd(int posInC, size_t acum_pos, int excess, size_t lim, int &minAcumE, size_t &minE, size_t &ruleMin, size_t &posMin);
				int getMinExcess(uint rule, int &mine, int &acum_excess,int&);
				//int getMinExcessTerminal(uint rule, int &mine, int &acum_excess,int&,int);
				int getMinExcessTerminalTest(uint rule, int &mine, int &acum_excess, int &posMin, int lim);
				size_t sampleOverheads(uint sample, uint *);
				size_t sampleOverheadsRec(uint rule, uint depth, uint sample, uint *bitVector);
				//in terminals vector we store the verbatim rules
				size_t searchRMM_rmq_linear(size_t p, int index, int &excess, int &minAcumE, size_t &acum_pos,size_t j, size_t &minE,size_t &posMin, size_t &ruleMin);
				size_t searchRuleDown_s0(size_t rule, size_t acum_pos, size_t &acum_e,size_t target);
				size_t searchRuleDown_s1(size_t rule, size_t acum_pos, size_t &acum_e, size_t target);
				int getRuleAllButLength(uint rule,int &mine,int &acum_excess,uint &leafs,uchar &fstb,uchar &lstb);
				void compressStructures();
				void testFWDBWDSEARCH_leafs(uint *input, size_t n);

                    uint samplingC;
                    uint maxRuleDepth;
                    uint *ruleLeafs;
                    uint *sampledRuleFstBit;
                    uint *sampledRuleLstBit;
                    size_t totalSampledRuleBitsInBits;
                    
                    uint *rule_lengths;
                    uint *newC;
				pair<uint,uint> *RULES;
				uint *RULES_LEFT;
				uint *RULES_RIGHT;
				uint *compressedLenTerms;
				uint verbatimLength;
				uint lenTermFieldWidth;
				
				
				DAC *compressedSampledTotalExcessDiff;
				
				
				
				DAC* heapSampleC_minE;
				
				
				
				uint *sampleC_leafs;
				uint heapLength;
				uint nSamplesC;
				uint *heapLevelSize;
				uint *sampledLengths;
				uint *sampledLeafs;
				uint *sampledZeroes;
				int *sampledMinExcess;
				int *sampledTotalExcess;
				
				BitSequence *bsExcessBuild;
				size_t getExcessBuild(size_t pos);
				
				void computeExcessRuleTest(int rule, int alpha,int &e, int*);
				
				unsigned short int *excessTest;
				uint nbits_excessTest;
				uint *terminals;
				uint *lenTerms;
                    uint *lengths;
                    uint *zeroes;
                    void updateLeaf(uint &leaf, uint &acumLeafs, uchar &prev, uchar fstb, uchar lstb);
                    void updateLeaf_bwd(uint &leaf, uint &acumLeafs, uchar &prev, uchar fstb, uchar lstb);
                    size_t posLastLeaf;
                  
                    int *minExcess;
                    int *maxExcess;
                    int *totalExcess;
                    uint sampleRateCE;
                    
                    uint blogBlockSizeC2;
                    uint blockSizeC2; 
                    uint estimationLeafsPerBlock;
                    //(o,p,e)--> p: max j {L(j)<=i*s}
                    //		     o: i*s - L(p)
                    //			 e: remplace r by total E up to p-1
                    size_t *sampleC_o;
                    size_t *sampleC_p;
					//sampleC_e and sampleC_minE are heaps (the leafs are at the beggining)
					//factorization_var *compressedSampleC_e;
                    size_t *sampleC_e; 
                    //factorization_var *compressedSampleC_minE;
                    int *sampleC_minE;
                    int *sampleC_maxE;
                    

				
                    pair<uint *,bool> * bitVectorsTrim;
                    uint *ruleOccs;
                    uint term;
                    size_t sampleRateC;
                    size_t nSamples;
                    size_t *samplesLen;
                    size_t *samplesZeroes;
                    int *samplesMinE;
                    int *samplesMaxE;
				
						//queries
                    void testFWDBWDSEARCH(uint  *input, size_t n);
				void testExcess(uint  *input, size_t n);
                    void testRankSelect(uint  *input);
			     void testRmq(uint  *input, size_t n);
				void testsAll(uint *input, size_t n);
					//operations	
                    //end-aordonez
		          uint buildVerbatimBitVector(HashMap* trimed, uint *symbPerm, uint *rules_length);
                    void sampling(int rate);
                    //void computeMinExcessTerminal(unsigned char v,uint offset, uint lim, int &acum_excess,int &mine, int &posMin, int &acum_pos);
		          void test (int * a)const;
                    void testAccess(uint *input);
				void testLeafRankSelect(uint *input);
				size_t internal_rank0(size_t pos, int &excess, uint &bit);
                    void initTables();

            
    };

};

#endif	/* _GCT_H */

