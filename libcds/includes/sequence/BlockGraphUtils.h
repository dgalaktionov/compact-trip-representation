/* BlockGraphUtils.h
 * Copyright (C) 2015, Alberto Ordóñez, all rights reserved.
 *
 * e-mail: alberto.ordonez@udc.es
 *
 * Utilities for the BlockGraph.
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

#ifndef _BLOCK_GRAPH_UTILS_H_
#define _BLOCK_GRAPH_UTILS_H_

#include <vector>
#include <string>
#include <utils/cppUtils.h>
#include <utils/GenericArray.h>

using namespace cds_utils;

namespace cds_static{
#define STRATEGY_SP_TOP_DOWN 0
#define STRATEGY_SP_BOTTOM_UP 1
#define STRATEGY_SP_BOTTOM_UP_WITH_ARITY 2
/**
* Implements different strategies to obtain the block length for each level of the block graph (Strategy Pattern).
*/

    class StrategySplitBlock{
    public:
        virtual ~StrategySplitBlock(){};
        virtual void computeBlockLength(uint level, vector<ulong> &blockLength, vector<ulong> &degree, ulong n)=0;
        static StrategySplitBlock* load(ifstream &fp);
        virtual void save(ofstream &fp)=0;
        virtual bool stop(uint level)=0 ;
        virtual uint getNLevels()=0;
    protected:
        StrategySplitBlock(){}
    };

/**
* StrategySplitBlock is a Top-Down approach in which all nodes have the same arity (or very close). It was
* the strategy used in the DCC'15 paper.
*/
    class StrategySplitBlockTopDown: public StrategySplitBlock{
    public:
        StrategySplitBlockTopDown(uint nLevels, uint arity):nLevels(nLevels),arity(arity){};
        virtual ~StrategySplitBlockTopDown(){};
        virtual void computeBlockLength(uint level, vector<ulong> &blockLength, vector<ulong> &degree, ulong n);
        static StrategySplitBlockTopDown* load(ifstream &fp);
        virtual void save(ofstream &fp);
        virtual bool stop(uint level);
        uint getNLevels(){return nLevels;}
        uint getArity()  {return arity;}
    protected:
        StrategySplitBlockTopDown(): StrategySplitBlock(){}
        uint nLevels;
        uint arity;
    };

/**
* StrategySplitBlockBottomUp implements a bottom-up strategy. It is given the number of levels of the block graph
* and the length of each leaf of the block graph. Thus, the block length at level nLevels (actually nLevels-1 since the
* first is the 0) is blockLength. At each level nLevels-i, with i \in [1,nLevels] the block graph has block length equals
* to blockLength^i. Note that all blocks have the same arity (equal to blockLength) but the root, which can have higher arity.
* This is the strategy we must use in case of using a block graph as an index, since we must guarantee the length of the leaves.
*/

    class StrategySplitBlockBottomUp: public StrategySplitBlock{
    public:
        StrategySplitBlockBottomUp(uint nLevels, uint blockLength):StrategySplitBlock(),nLevels(nLevels),blockLengthAtLeaves(blockLength){};
        virtual ~StrategySplitBlockBottomUp(){};
        virtual void computeBlockLength(uint level, vector<ulong> &blockLength, vector<ulong> &degree, ulong n);
        static StrategySplitBlockBottomUp* load(ifstream &fp);
        virtual void save(ofstream &fp);
        virtual bool stop(uint level);
        uint getNLevels(){return nLevels;}
        uint getEll()  {return blockLengthAtLeaves;}
    protected:
        StrategySplitBlockBottomUp():StrategySplitBlock(){}
        uint nLevels;
        ulong blockLengthAtLeaves;
    };


/**
* StrategySplitBlockBottomUp implements a bottom-up strategy. It is given the number of levels of the block graph,
* the length of each leaf of the block graph, and the arity (in how many blocks each block is split).
*/

    class StrategySplitBlockBottomUpWithArity: public StrategySplitBlock{
    public:
        StrategySplitBlockBottomUpWithArity(uint nLevels, uint blockLength, uint arity=0U):StrategySplitBlock(),nLevels(nLevels),blockLengthAtLeaves(blockLength),arity(arity){};
        virtual ~StrategySplitBlockBottomUpWithArity(){};
        virtual void computeBlockLength(uint level, vector<ulong> &blockLength, vector<ulong> &degree, ulong n);
        static StrategySplitBlockBottomUpWithArity* load(ifstream &fp);
        virtual void save(ofstream &fp);
        virtual bool stop(uint level);
        uint getNLevels(){return nLevels;}
        uint getEll()  {return blockLengthAtLeaves;}
    protected:
        StrategySplitBlockBottomUpWithArity():StrategySplitBlock(){}
        uint nLevels;
        ulong blockLengthAtLeaves;
        uint arity;
    };

    inline StrategySplitBlock* StrategySplitBlock::load(ifstream &fp){
        uint wr;
        fp.read((char*)&wr, sizeof(uint));
        size_t pos = fp.tellg();
        fp.seekg(pos-sizeof(uint), ios::beg);
        switch (wr){
            case STRATEGY_SP_TOP_DOWN:
                return StrategySplitBlockTopDown::load(fp);
            case STRATEGY_SP_BOTTOM_UP:
                return StrategySplitBlockBottomUp::load(fp);
            case STRATEGY_SP_BOTTOM_UP_WITH_ARITY:
                return StrategySplitBlockBottomUpWithArity::load(fp);
        }
        return nullptr;
    }

    /***********************    TOP-DOWN IMPLEMENTATION *************************************/
    inline StrategySplitBlockTopDown* StrategySplitBlockTopDown::load(ifstream &fp){
        uint wr;
        fp.read((char*)&wr, sizeof(uint));
        if (wr!= STRATEGY_SP_TOP_DOWN)
            return nullptr;
        StrategySplitBlockTopDown *td = new StrategySplitBlockTopDown();
        td->nLevels = loadValue<uint>(fp);
        td->arity = loadValue<uint>(fp);
        return td;
    }
    inline void StrategySplitBlockTopDown::save(ofstream &fp){
        uint wr = STRATEGY_SP_TOP_DOWN;
        saveValue(fp, wr);
        saveValue(fp,nLevels);
        saveValue(fp,arity);
    }

    inline void StrategySplitBlockTopDown::computeBlockLength(uint level, vector<ulong> &blockLength, vector<ulong> &degree, ulong n){
        if (level==0){
            blockLength[level] = n/arity + arity-((n/arity)%arity);
            degree[level] = n / blockLength[level] +((n%blockLength[level])?1:0);
        }else{
            blockLength[level] = blockLength[level-1]/arity + arity-((blockLength[level-1]/arity)%arity);
            degree[level] = blockLength[level-1]/blockLength[level] +((blockLength[level-1]%blockLength[level])?1:0);;
            if (blockLength[level] <=1){
                cerr << "blockLength at level " << level << " is "<< blockLength[level] << endl;
                throw runtime_error("BlockLength at level "+to_string(level)+" is "+to_string(blockLength[level])+"(<=1) at computeBlockLength");
            }
        }
    }
    inline bool StrategySplitBlockTopDown::stop(uint level){
        return (level+1== nLevels);
    }


    /***********************    BOTTOM-UP IMPLEMENTATION *************************************/
    inline void StrategySplitBlockBottomUp::computeBlockLength(uint level, vector<ulong> &blockLength, vector<ulong> &degree, ulong n){
        if (level==0){
            blockLength[0] = 1;
            for (auto i=1u;i<this->nLevels;i++)
                blockLength[0]*=blockLengthAtLeaves;
            if (blockLength[0]>n){
                string message = "The given configuration is incorrect. The block length at the root level is " + to_string(blockLength[level])+" while the collection is of length "+to_string(n)+" (at computeBlockLength)";
                throw  runtime_error(message);
            }
            degree[0] = n / blockLength[level] + ((n / blockLength[level])?1:0);
        }else{
            blockLength[level] = blockLength[level-1]/blockLengthAtLeaves;
            degree[level] = blockLengthAtLeaves;
        }
    }

    inline StrategySplitBlockBottomUp* StrategySplitBlockBottomUp::load(ifstream &fp){
        uint wr;
        fp.read((char*)&wr, sizeof(uint));
        if (wr!= STRATEGY_SP_BOTTOM_UP)
            return nullptr;
        StrategySplitBlockBottomUp *td = new StrategySplitBlockBottomUp();
        td->nLevels = loadValue<uint>(fp);
        td->blockLengthAtLeaves = loadValue<ulong>(fp);
        return td;
    }

    inline void StrategySplitBlockBottomUp::save(ofstream &fp){
        uint wr = STRATEGY_SP_BOTTOM_UP;
        saveValue(fp, wr);
        saveValue(fp,nLevels);
        saveValue(fp,blockLengthAtLeaves);
    }

    inline bool StrategySplitBlockBottomUp::stop(uint level){
        return (level+1==nLevels);
    }

    /***********************    BOTTOM-UP-WITH-ARITY IMPLEMENTATION *************************************/

    inline void StrategySplitBlockBottomUpWithArity::computeBlockLength(uint level, vector<ulong> &blockLength,
                                                                        vector<ulong> &degree, ulong n){
        if (level==0){
            blockLength[0] = blockLengthAtLeaves;
            for (auto i=2u;i<this->nLevels;i++)
                blockLength[0]*=arity;
            if (blockLength[0]>n){
                string message = "The given configuration is incorrect. The block length at the root level is " +
                        to_string(blockLength[level])+" while the collection is of length "+
                        to_string(n)+" (at computeBlockLength)";
                throw  runtime_error(message);
            }
        }else{
            blockLength[level] = blockLength[level-1]/arity;
        }
        degree[level] = arity;
    }
    inline StrategySplitBlockBottomUpWithArity* StrategySplitBlockBottomUpWithArity::load(ifstream &fp){
        uint wr;
        fp.read((char*)&wr, sizeof(uint));
        if (wr!= STRATEGY_SP_BOTTOM_UP_WITH_ARITY)
            return nullptr;
        StrategySplitBlockBottomUpWithArity *td = new StrategySplitBlockBottomUpWithArity();
        td->nLevels = loadValue<uint>(fp);
        td->blockLengthAtLeaves = loadValue<ulong>(fp);
        td->arity = loadValue<uint>(fp);
        return td;
    }

    inline void StrategySplitBlockBottomUpWithArity::save(ofstream &fp){
        uint wr = STRATEGY_SP_BOTTOM_UP_WITH_ARITY;
        saveValue(fp, wr);
        saveValue(fp,nLevels);
        saveValue(fp,blockLengthAtLeaves);
        saveValue(fp,arity);
    }

    inline bool StrategySplitBlockBottomUpWithArity::stop(uint level){
        return (level+1==nLevels);
    }

/**
* Object used for the Rabin-Karp parsing of the Block Graph. Basically,
* it encapsulates the concept of a block (in a RK-parsing): it has a blockId
* (which is the order of the block in the input sequence), and a first_occ
* which points the first occurrence of that block (if it is known or -1 otherwise).
*/
    class BlockHash{
    public:
        BlockHash(){
            this->block_id=-1;
            this->first_occ=-1;
        }
        BlockHash(long blockId){
            this->block_id=blockId;
            this->first_occ=-1;
        }
        ~BlockHash(){}

        BlockHash(long blockId, long firstOcc){
            this->first_occ=firstOcc;
            this->block_id = blockId;
        }
        long getFirstOcc(){
            return first_occ;
        }
        long getBlockId(){
            return block_id;
        }
        void setFirstOcc(long fo){
            first_occ = fo;
        }
        void setBlockId(long id){
            this->block_id = id;
        }
    protected:
        long first_occ;
        long block_id;

    };

    class BlockContainer{
    protected:
        ulong nBlocks;
        ulong undefValue;
    public:
        BlockContainer(ulong nBlocks):nBlocks(nBlocks){}
        virtual ~BlockContainer(){}
        virtual ulong getFirstOcc(ulong blockId) = 0;
        virtual void setUndefFirstOcc(ulong blockId) = 0;
        virtual void setFirstOcc(ulong blockId, ulong firstOcc)=0;
        virtual bool isUndefined(ulong first_occ) = 0;

    };

    class CompactBlockContainer: public BlockContainer{
    protected:
        GenericArray<ulong> first_occ;
//        Array first_occ;
    public:

        CompactBlockContainer(ulong nBlocks, ulong maxPointer):BlockContainer(nBlocks){
            undefValue = maxPointer+1;
//            std::cout << "In compact container trying to allocate space for " << nBlocks << std::endl;
            first_occ = GenericArray<ulong>(nBlocks,this->undefValue);
//            std::cout << "Space allocated. Will continue!"<< std::endl;
//            first_occ = Array(nBlocks,(uint)this->undefValue);
        }

        ~CompactBlockContainer(){}

        inline ulong getFirstOcc(ulong blockId){
            return first_occ[blockId];
        }
        inline void setUndefFirstOcc(ulong blockId){
            assert(blockId<nBlocks);
            first_occ.setField(blockId,undefValue);
        }
        inline void setFirstOcc(ulong blockId, ulong firstOcc){
            assert(blockId<nBlocks);
            first_occ.setField(blockId,firstOcc);
        }
        inline bool isUndefined(ulong fo){
            return fo==undefValue;
        }

    };

    class NaiveBlockContainer:public BlockContainer{
    protected:
        vector<ulong> first_occ;
    public:

        NaiveBlockContainer(ulong nBlocks, ulong maxPointer):BlockContainer(nBlocks){
            undefValue = -1ULL;
            first_occ = vector<ulong>(nBlocks);
        }

        ~NaiveBlockContainer(){}

        inline ulong getFirstOcc(ulong blockId){
            return first_occ[blockId];
        }

        inline void setUndefFirstOcc(ulong blockId){
            assert(blockId<nBlocks);
            first_occ[blockId] = undefValue;
        }
        inline void setFirstOcc(ulong blockId, ulong firstOcc){
            assert(blockId<nBlocks);
            first_occ[blockId] = firstOcc;
        }

        inline bool isUndefined(ulong fo){
            return fo==undefValue;
        }

    };

};


#endif