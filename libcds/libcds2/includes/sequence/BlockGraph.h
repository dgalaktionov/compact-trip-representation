/* BlockGraph.h
 * Copyright (C) 2015, Alberto Ordóñez, all rights reserved.
 *
 * e-mail: alberto.ordonez@udc.es
 *
 * Block Graph implementation. Initially, a block graph should only permit to
 * access a given position or a range of positions in a sequence. If one
 * wants to carry out rank/select operations, on must to take samples of ranks
 * until some positions (adding a \sigma overhead). Thus, we have implemented
 * the Block Graph using a template pattern: we implemented the parsing algorithm
 * and we left several functions unimplemented (or with a default behavior). For
 * instance, the functions that store the rank samplings must not be implemented by
 * a BlockGraph but by a SequenceBlockGraph. If a function has no sense for a class,
 * the class provides a default behaviour (do nothing for instance).
 *
 * SequenceBlockGraph<T,Leaves>. It adds rank/select capabilities to a BlockGraph.
 * It extends a BlockGraph and implements those functions necessary to
 * store and deal with rank samples. It also extends from class Sequence,
 * so that we can use a SequenceBlockGraph<T,Leaves> where a Sequence is needed, that
 * is, where rank/select/access functionallities are necessary.
 *
 * References:
 * [1]: Belazzougui, D.; Gagie, T.; Gawrychowski, P.; Kärkkäinen, J.; Ordóñez, A.; Puglisi, S.J.; Tabei, Y.:
 *      "Queries on LZ-Bounded Encodings", en Proceedings of the 2015 Data Compression Conference (DCC 2015),
 *      IEEE Computer Society, Salt Lake City, Utah (Estados Unidos), 2015, pp. 83-92.
 *
 * Seq
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

#ifndef _BLOCK_GRAPH_H_
#define _BLOCK_GRAPH_H_

#include <direct_access/DirectAccess.h>
#include <bitsequence/BitSequenceBuilder.h>
#include <utils/TemporaryFile.h>
#include <sequence/SequenceLZBLockHelper.h>
#include <vector>
#include <sequence/BlockGraphUtils.h>
#include <utils/HashTableBG.h>
#include <sequence/Sequence.h>
#include <utils/StringUtils.h>
#include <utils/SparseSuffixSort.h>
#include <chrono>
#include <ctime>
#include <utils/Logger.h>
#include <unordered_map>
#ifndef BLOCKGRAPH_HDR
#define BLOCKGRAPH_HDR 1
#endif

#ifndef SEQUENCE_BLOCK_GRAPH_HDR
#define SEQUENCE_BLOCK_GRAPH_HDR 2
#endif

#ifndef BLOCKGRAPH_INDEX_HDR
#define BLOCKGRAPH_INDEX_HDR 3
#endif
/**
* Default paremeters.
* MAX_LEVELS_BG: máximun number of leaves of a block graph
*/
#define MAX_LEVELS_BG 32
#define DEFAULT_MAX_SIGMA 255
#define DEFAULT_BUFFER_WRITE_SIZE 100000000
#define DEFAULT_BUFFER_READ_SIZE 100000000

using namespace cds_utils;

namespace cds_static {

    template<typename T, class Leaves>
    class BlockGraph {

    public:
        //constructor
        BlockGraph(StrategySplitBlock *s, uint gs=2);
        virtual void build(string &inFile, bool can_delete);
        virtual BlockGraph& setMarksBuilder(BitSequenceBuilder *bsb);
        /**
        * @param outStream: stream to write out the data structure. It must be opened if it is
        not empty. The data structure is written level by level, freeing the space the data
        structures take after written them.
        */
        virtual BlockGraph& setOutStream(string &outFile);
        virtual BlockGraph& setMaxV(T maxv);
        virtual BlockGraph& setBufferInputReadSize(ulong inputBufferSize);
        virtual BlockGraph& setBufferOutputReadSize(ulong inputBufferSize);
        virtual BlockGraph& setBufferInName(const string &tmpFileNameBufferIn);
        virtual BlockGraph& setBufferOutName(const string &tmpFileNameBufferOut);
        virtual BlockGraph& setLoggFile(const string &logger_file);
        //destructor
        virtual ~BlockGraph();

        StrategySplitBlock *getStrategy();
        virtual ulong getLength();
        T getSigma();
        //Save and load functions
        virtual void save(ofstream & fp) const;
        static BlockGraph* load(ifstream & fp);

        //access operations
        virtual uint operator[](size_t pos) const;
        virtual size_t getSize() const;
        virtual uint getHeader() const;

        void SetHashA(ulong _a){hash_a=_a;}
        void SetHashB(ulong _b){hash_b=_b;}
        void SetHashFactor(double f){hash_factor=f;}
    protected:
        ulong hash_a;
        ulong hash_b;
        double hash_factor;
        size_t n;
        T max_v;//the actual value of max_v (the max value found in the input stream)
        T min_v;//the min value found in the input stream
        T max_possible_v;//max possible value for max_v. By default 256
        ofstream *output;
        long bufferReadSize;
        long bufferWriteSize;
        int firstLevel;
        uint groupSize;//pairs of blocks=2, triplets=3, and so on.
        string tmpFileBufferIn;
        string tmpFileBufferOut;
        BitSequenceBuilder *bsb_marks;
        vector<BitSequence *> marks;
        vector<DirectAccess *> pointersBack;
        vector<DirectAccess *> offsetsBack;
        vector<ulong> blockLengthLevel;
        vector<ulong> degreeLevel;
        vector<uint> leavesLZB;
        Leaves *seq_leaves;
#ifdef MYDEBUG
        Sequence *seq_tests;
#endif

        cds_utils::Logger<string> logger;
        vector<ulong> positions;
        BlockGraph(){}
        StrategySplitBlock *strategySplitBlocks;


        /*Internal construction functions*/
        ulong run_build(T *&input, ulong n, uint level, string &buffer1, string &buffer2, vector<bool> &boundaries);
        void initializeListOfBlocks(HashTableBG<ulong> &, BlockContainer &listOfBlocks, T *input, const ulong start, const ulong end,
                                    const size_t base, const size_t modulo, const uint level, vector<bool> &plainMarks, string &bufferRead);
        ulong compressMarks(uint level, vector<bool> &plainMarks);
        ulong buildCompressBG(uint level,vector<uint> &pointersUnmarked, vector<uint> &offsetsUnmarked);
        virtual void indexLeaves(string &inputBuffer,uint level, const vector<bool> &boundaries);
        virtual void markBlockBoundaries(uint level, vector<bool> &marks, vector<bool>&newMarks);
        void postProcessMarks(uint level, ulong nBlocks, BlockContainer &listOfBlocks,vector<bool> &plainMarks, vector<pair<ulong,ulong>> &blockAndPointer);
        virtual void sortSuffixes(uint level,IfstreamBuffer<T> &inBuffer);
        inline virtual void updateBlockAndPointer(ulong blockId, ulong firstOcc,vector<pair<ulong,ulong>> &blockAndPointers){}
        /**
        * Default behaviour of functions related to rank operations is set to void. If you want to support rank/select operations,
        * you may extend this class and override that functions.
        */
        virtual ulong storePointersBack(T *&input, ulong n, uint level, ulong blockLengthPreviousLevel, BlockContainer &listOfBlocks,
                                        vector<uint> &pointersUnmarked, vector<uint> &offsetsUnmarked,vector<uint> *ranksBlocks, vector<uint> *ranksOffsets,
                                        string &bufferRead, string &bufferWrite,vector<pair<ulong,ulong>> &blockAndPointers);
        virtual ulong buildCompressRank(uint level, vector<uint> *ranksBlocks, vector<uint> *ranksOffsets,const vector<bool> &){return 0ul;}
        inline virtual void incrementCounterRank(uint level, ulong i, ulong blockLength, ulong blockLengthPreviousLevel, vector<uint> *ranksBlocks, vector<ulong> &counter){}
        virtual void addToBlockAndPointer(ulong blockId, ulong firstOcc, vector<pair<ulong,ulong>> &blockAndPointer){}

        //internal Save and Load

        void saveLevel(ofstream & fp, int level, bool saveLeaves) const;
        BlockGraph<T,Leaves>* loadByLevels(ifstream & fp);

        virtual void saveLevelRank(ofstream &fp, int level) const;
        virtual void loadByLevelsRank(ifstream & fp, uint level);


        //deleter
        void deleteLevel(uint level);
        virtual void deleteLevelRanks(uint level);

        virtual size_t getSizeRank() const;
        //access the sequence of leaves
        T accessLeaves(long pos) const;
        virtual uint getPointerBack(uint level, size_t block, size_t &unmarked_index, size_t &block_index, size_t &block_pos) const;
        //test functions for the construction algorithms
        virtual bool testRanks(vector<T> &, uint level){return true;};
        bool testParsing( vector<T> &, uint level);
        virtual bool testSegmentsIndex(uint level, vector<bool> &boundaries, ulong len);
        //virtual void computeBlockLength(uint level, vector<ulong> &blockLength, vector<ulong> &degree,ulong n);
    };

    /**
    *
    * SequenceBlockGraph encapsulates a BlockGraph and adds the data structures to support
    * rank/select queries. It extends from a BlockGraph but also from a Sequence.
    *
    */
    template<typename T, class Leaves> class SequenceBlockGraph : public BlockGraph<T,Leaves>, public Sequence{

        friend class BlockGraph<T,Leaves>;
    public:
        SequenceBlockGraph(StrategySplitBlock *st, uint gs=2);//:BlockGraph<T>(b,_nLevels,gs),Sequence(0){}
        virtual SequenceBlockGraph& setLeavesBuilder(SequenceBuilder *sb);
        virtual void build(string &inFile, bool can_delete);
        //If you don't define the access here, when used is as a Sequence, the access that is called is Sequence.access()
        //and not SequenceBlockGraph.access.
        virtual uint access(size_t pos) const;
        virtual size_t rank(uint symbol, size_t pos) const;
        virtual size_t select(uint symbol, size_t j) const;
        virtual void save(ofstream &fp) const;
        virtual ulong getLength() const;
        SequenceBlockGraph<T,Leaves> & setSequenceBuilder(SequenceBuilder *seq_builder);
        static SequenceBlockGraph* load(ifstream &fp);
        virtual size_t getSize() const;
        virtual uint getHeader() const;
    protected:
        vector<DirectAccess **> ranksBlocksDA;
        vector<DirectAccess **> ranksOffsetsDA;
        SequenceBlockGraph();
        SequenceBuilder *seq_builder;

        /**
        * Default behaviour of functions related to rank operations is set to void. If you want to support rank/select operations,
        * you may extend this class and override that functions.
        */
        virtual ulong storePointersBack(T *&input, ulong n, uint level, ulong blockLengthPreviousLevel, BlockContainer &listOfBlocks,
                                        vector<uint> &pointersUnmarked, vector<uint> &offsetsUnmarked,vector<uint> *ranksBlocks, vector<uint> *ranksOffsets,
                                        string &bufferRead, string &bufferWrite,vector<pair<ulong,ulong>> &blockAndPointers);
        virtual ulong buildCompressRank(uint level, vector<uint> *ranksBlocks, vector<uint> *ranksOffsets, const vector<bool> &symbolFound);
        virtual void incrementCounterRank(uint level, ulong i, ulong blockLength, ulong blockLengthPreviousLevel, vector<uint> *ranksBlocks, vector<ulong> &counter);
        virtual void indexLeaves(string &inputBuffer,uint level, const vector<bool> &boundaries);
        virtual void updateBlockAndPointer(ulong blockId, ulong firstOcc,vector<pair<ulong,ulong>> &blockAndPointers);
        virtual void saveLevelRank(ofstream &fp, int level) const;
        virtual void loadByLevelsRank(ifstream & fp, uint level);
        virtual size_t getSizeRank() const;
        /*****************RANK SELECT STUFF*******************/
        size_t accessRanksBlocks(uint level, uint symbol, size_t pos) const;
        size_t accessRanksOffsets(uint level, uint symbol, size_t pos) const;
        long bs_ranks(uint level, uint symbol, size_t l, size_t r, size_t b, size_t &v) const;
        bool testRanks(vector<T> &, uint level);

        virtual void deleteLevelRanks(uint level);
    };

    /*Block Graph Index*/
    template<typename T, class Leaves> class BlockGraphIndex : public BlockGraph<T,Leaves>{

        friend class BlockGraph<T,Leaves>;
    public:
        virtual ~BlockGraphIndex();
        BlockGraphIndex(StrategySplitBlock *st, uint gs=3);//:BlockGraph<T>(b,_nLevels,gs),Sequence(0){}
        virtual BlockGraphIndex& setSaveKernels(string &fileName);
        virtual uint getHeader() const;
    protected:
        BlockGraphIndex();

        virtual void indexLeaves(string &inputBuffer,uint level, const vector<bool> &boundaries);
        virtual void indexLeavesSimplified(string &inputBuffer,uint level, const vector<bool> &boundaries);
        void saveKernels(vector<T> &leaves);
        virtual void markBlockBoundaries(uint level, vector<bool> &marks, vector<bool>&newMarks);
        virtual bool testSegmentsIndex(uint level, vector<bool> &boundaries, ulong len);
        virtual void sortSuffixes(uint level,IfstreamBuffer<T> &inBuffer);
        virtual void sortSuffixesReversedPhrases(uint level,IfstreamBuffer<T> &inBuffer, vector<long> &sa);
        virtual void sortSuffixesPhraseBoundaries(uint level,IfstreamBuffer<T> &inBuffer, vector<long> &sa);

        long ell;
        string kernelsOutFile;
    };


    /***********************************************************************************/
    /***********************************************************************************/
    /*********************  BLOCK GRAPH IMPLEMENTATION  ********************************/
    /***********************************************************************************/
    /***********************************************************************************/

    template<typename T, class Leaves> ulong  BlockGraph<T,Leaves>::getLength(){
        return n;
    }
    template<typename T, class Leaves> T  BlockGraph<T,Leaves>::getSigma(){
        return max_v+1;
    }
    template<typename T, class Leaves> StrategySplitBlock*  BlockGraph<T,Leaves>::getStrategy(){
        return strategySplitBlocks;
    }

    template <typename T, class Leaves> void  BlockGraph<T,Leaves>::markBlockBoundaries(uint level, vector<bool> &marks, vector<bool>&newMarks){}

    template<typename T, class Leaves> void  BlockGraph<T,Leaves>::sortSuffixes(uint level,IfstreamBuffer<T> &inBuffer){}

    template<typename T, class Leaves> uint  BlockGraph<T,Leaves>::getHeader() const{
        uint w = BLOCKGRAPH_HDR;
        return w;
    }

    template<typename T, class Leaves> size_t   BlockGraph<T,Leaves>::getSizeRank() const{
        return (size_t)0;
    }

    template<typename T, class Leaves> size_t   BlockGraph<T,Leaves>::getSize() const{
        //in case of binary alphabets we only consider
        //the samplings for one out of the two symbols
        size_t total = 0;
        total += sizeof(BitSequence*)*(this->strategySplitBlocks->getNLevels()-1);
        for (uint i=firstLevel;i<this->strategySplitBlocks->getNLevels()-1;i++)
            total+=marks[i]->getSize();

        total+= sizeof(DirectAccess*)*(this->strategySplitBlocks->getNLevels()-1);
        for (uint i=firstLevel;i<this->strategySplitBlocks->getNLevels()-1;i++)
            total+=pointersBack[i]->getSize();

        total+= sizeof(DirectAccess*)*(this->strategySplitBlocks->getNLevels()-1);
        for (uint i=firstLevel;i<this->strategySplitBlocks->getNLevels()-1;i++)
            total+=offsetsBack[i]->getSize();

        total+= sizeof(uint)*(this->strategySplitBlocks->getNLevels()-1);

        total+=getSizeRank();

        total+=seq_leaves->getSize();
//        cerr << "Total size: " << total << ", leaves: " << seq_leaves->getSize() << endl;
//        cerr << "#elements in the leaves: " << seq_leaves->getLength() << endl;
        return total;
    }

    template<typename T, class Leaves> void  BlockGraph<T,Leaves>::loadByLevelsRank(ifstream & fp,  uint level){
        uint wr;
        wr = loadValue<uint>(fp);
        assert(wr== BLOCKGRAPH_HDR);
    }

    template<typename T, class Leaves> void  BlockGraph<T,Leaves>::saveLevelRank(ofstream &fp, int level) const{
        uint wr = BLOCKGRAPH_HDR;
        saveValue(fp,wr);
    }

    //destructor
    template<typename T, class Leaves>  BlockGraph<T,Leaves>::~BlockGraph() {
        if (output) delete output;
        for (uint i=firstLevel;i<this->strategySplitBlocks->getNLevels()-1;i++){
            deleteLevel(i);
        }
        delete strategySplitBlocks;
        if (seq_leaves) delete seq_leaves;
        for (auto &it : marks){
            if (it) delete it;
        }

    }

    //constructor
    template<typename T, class Leaves>  BlockGraph<T,Leaves>::BlockGraph(StrategySplitBlock* st, uint gs):strategySplitBlocks(st),groupSize(gs) {
        if (strategySplitBlocks->getNLevels()>= MAX_LEVELS_BG){
            cerr << "The number of levels must be smaller than " << MAX_LEVELS_BG << endl;
            exit(0);
        }
        groupSize   = gs;
        strategySplitBlocks = st;
        firstLevel  = -1l;
        bufferReadSize = DEFAULT_BUFFER_READ_SIZE;
        bufferWriteSize = DEFAULT_BUFFER_WRITE_SIZE;
        max_possible_v = DEFAULT_MAX_SIGMA;
        output      = nullptr;
        bsb_marks   = nullptr;
        seq_leaves  = nullptr;
        hash_a = 17;
        hash_b = 32;
        hash_factor = 1.25;
    }

    template<typename T, class Leaves> BlockGraph<T,Leaves>&  BlockGraph<T,Leaves>::setMarksBuilder(BitSequenceBuilder *bsb){
        this->bsb_marks = bsb;
        return *this;
    }


    template<typename T, class Leaves> BlockGraph<T,Leaves>&  BlockGraph<T,Leaves>::setOutStream(string &outFile){
        if (!outFile.empty()) {
            output = new ofstream(outFile);
        }
        return *this;
    }

    template<typename T, class Leaves> BlockGraph<T,Leaves>&  BlockGraph<T,Leaves>::setMaxV(T maxValue){
        this->max_possible_v = maxValue;
        return *this;
    }

    template<typename T, class Leaves> BlockGraph<T,Leaves>&  BlockGraph<T,Leaves>::
    setBufferInputReadSize(ulong inputBufferSize) {
        this->bufferReadSize = inputBufferSize;
        return *this;
    }

    template<typename T, class Leaves> BlockGraph<T,Leaves>&  BlockGraph<T,Leaves>::
    setBufferOutputReadSize(ulong outputBufferSize){
        this->bufferWriteSize = outputBufferSize;
        return *this;
    }

    template<typename T,class Leaves> BlockGraph<T,Leaves>& BlockGraph<T,Leaves>::setBufferInName(const string &tmpFileNameBufferIn){
        this->tmpFileBufferIn = tmpFileNameBufferIn;
        return *this;
    }
    template<typename T,class Leaves> BlockGraph<T,Leaves>& BlockGraph<T,Leaves>::setBufferOutName(const string &tmpFileNameBufferOut) {
        this->tmpFileBufferOut = tmpFileNameBufferOut;
        return *this;
    }
    template<typename T,class Leaves> BlockGraph<T,Leaves>& BlockGraph<T,Leaves>::setLoggFile(const string &logger_file) {
        this->logger.setFile(logger_file);
        return *this;
    }

    template<typename T, class Leaves> void  BlockGraph<T,Leaves>::build(string &inFile, bool can_delete) {

        //can_delete = false;
        /**Check if the files for input and output buffers were set. If no, create
        * a temporaty file*/
        string buffer1;
        string buffer2;
        bool tmpBuffer1, tmpBuffer2;
        tmpBuffer1=tmpBuffer2=false;
        buffer1 = tmpFileBufferIn;
        if (buffer1.empty()){
            //generate tmp files for the buffers
            TemporaryFile tmpFile1;
            buffer1 = tmpFile1.getFileName();
            tmpBuffer1 = true;
        }
        buffer2 = tmpFileBufferOut;
        if (buffer2.empty()){
            //generate tmp files for the buffers
            TemporaryFile tmpFile;
            buffer2 = tmpFile.getFileName();
            tmpBuffer2=true;
        }
        if (buffer1==buffer2){
            this->logger.ERROR("Both temporary files are the same:  " +buffer1+"\nProgram will close!","BlockGraph");
            return ;
        }
//        if (can_delete) {
        //the first buffer is the original file
        // buffer1 = inFile;//new string(inFile);
//        }
//        else {
//            //copy the input file in the first buffer
//            ofstream out(buffer1);
//            ifstream in(inFile);
//            if (!out.good()) {
//                throw runtime_error("Error while opening " + buffer1);
//            }
//            if (!in.good()) {
//                throw runtime_error("Error while opening " + inFile);
//            }
//            //SequenceLZBLockHelper::mapLargeSequence(inFile, buffer1, n, max_v);
//            SequenceLZBLockHelper::copyFile<T>(inFile, buffer1);
//        }
        //In the first iteration, the buffer1 points to the
        //source file (buffer1 is read only). In the following
        //,since we swap buffer1 and buffer2, buffe1 is not
        //anymore read only.
        string bufferInFirstIter = buffer1;
        buffer1 = inFile;
        T *input;
        ifstream in_f(buffer1);
        in_f.seekg(0, ios_base::end);
        n = in_f.tellg() / sizeof(T);
        if(in_f.tellg() % sizeof(T) != 0){
            this->logger.ERROR("The input file size is not a multiple of the data type you state it is.","BlockGraph");
            throw runtime_error("The input file size is not a multiple of the data type you state it is.");
        }
#ifdef MYDEBUG
        in_f.seekg(0, ios_base::beg);
        input = new T[n];
        in_f.read((char *) input, n * sizeof(T));
#endif
        in_f.close();


//        max_v = getMaxV(input, n);
        max_v = (T)0;
        blockLengthLevel.reserve(MAX_LEVELS_BG);
        degreeLevel.reserve(MAX_LEVELS_BG);

        if (bsb_marks == nullptr) {
            bsb_marks = new BitSequenceBuilderPlain(32);
        }
        if (strategySplitBlocks == nullptr){

        }
        marks.reserve(MAX_LEVELS_BG);
        for (auto &m: marks){
            m = nullptr;
        }
        pointersBack.reserve(MAX_LEVELS_BG);
        offsetsBack.reserve(MAX_LEVELS_BG);

#ifdef MYDEBUG
        positions.reserve(n);
        for (uint i=0U;i<n;i++)
            positions.push_back(i);
#endif

        //call the building algorithm
        uint level = 0;
        ulong new_length = n;
        vector<bool> boundaries;
        while (new_length) {
            this->logger.INFO("Running build algorithm for level " +level,"BlockGraph");
//            cerr << "Running build algorithm for level " << level << endl;
            new_length = run_build(input, new_length, level, buffer1, buffer2, boundaries);
            if (output) {
                if (output->good()) {
                    this->logger.INFO("Saving level "+level,"BlockGraph");
                    saveLevel(*output, level,strategySplitBlocks->stop(level));
                }
            }
            if (level==0){
                buffer1=buffer2;
                buffer2=bufferInFirstIter;
            }else {
                stringstream ss;
                ss << "Swapping buffers:" << endl;

                ss << "\tBuffer1: " << buffer1;
                ss  << "\tBuffer2: " << buffer2;
                swap(buffer1, buffer2);
                ss << "\tBuffer1: " << buffer1;
                ss << "\tBuffer2: " << buffer2 << endl;
                this->logger.INFO(ss.str(),"BlockGraph");
            }
            level++;
        }
#ifdef MYDEBUG
        delete [] input;
#endif

        //remove buffers
        remove(buffer1.c_str());
        remove(buffer2.c_str());
//TODO: Uncomment the next line
        //delete bsb_marks;
    }

    template<typename T, class Leaves> ulong  BlockGraph<T,Leaves>::run_build(T *&input, ulong n, uint level, string &buffer1, string &buffer2, vector<bool> &boundaries){

        vector<bool> plainMarks;
        vector<uint> pointersUnmarked;
        vector<uint> offsetsUnmarked;
        vector<uint> *ranksBlocks = new vector<uint>[max_possible_v+1];
        vector<uint> *ranksOffsets = new vector<uint>[max_possible_v+1];
        ulong pointer_new_input = n;
#ifdef MYDEBUG
        vector<T> sequence_tests(n);
        for (size_t i=0;i<n;i++)
            sequence_tests[i]=input[i];
#endif
        auto checkBuffers = [=](string &buff){
            ifstream in(buff);
            if (!in.good()){
//                cerr << "Buffer " << buff << " not good" << endl;
                return false;
            }
            in.seekg(0, ios_base::end);
//            cerr << "Contains " << in.tellg() << " bytes" << endl;
            return true;

        };
        checkBuffers(buffer1);
        checkBuffers(buffer2);
        ulong blockLengthPreviousLevel = ((level==0) || (firstLevel==-1) || blockLengthLevel[level-1]==0)?n+1:blockLengthLevel[level-1];
        if (strategySplitBlocks->stop(level)){
            //NLevels = level+1;
            blockLengthLevel[level] = blockLengthLevel[level-1];
            testSegmentsIndex(level, boundaries, n);
            indexLeaves(buffer1,level,boundaries);
            pointer_new_input = 0;
        }else {
            this->logger.INFO("level: "+level, "BlockGraph");
            strategySplitBlocks->computeBlockLength(level, blockLengthLevel, degreeLevel, n);

            //TODO: cannot make such an assumption about max_possible_v
            size_t base = 1u << (bits(max((uint) max_possible_v, 2u)));
            ulong nBlocks = n / blockLengthLevel[level] + ((n % blockLengthLevel[level]) ? 1 : 0);
            //generally, it is convenient to uncomment the following lines (or define the arity in a different way) since
            //in the first two levels every block is marked. It doesn't affect the space performance nor the query time since,
            //later in the code, if all the blocks are marked we skip that level.
            if (level < 0) {
                pointer_new_input = 0;
            } else {

                plainMarks.assign(nBlocks, false);

                this->logger.INFO("Creating the hash table","BLockGraph");
                //I'm generous with the hash table size
                HashTableBG<ulong> *hash = new HashTableBG<ulong>(hash_a, hash_b, (ulong) (nBlocks * hash_factor));
                //vector<BlockHash> listOfBlocks(nBlocks);// = new BlockHash *[nBlocks];
                vector<bool> symbolFound(max_possible_v+1,false);
                CompactBlockContainer listOfBlocks(nBlocks, n);// = new BlockHash *[nBlocks];
//                cerr << "hash size: " << hash->getSize() << endl;
                //ensures that bits( (x%modulo ) * base)<64
                size_t modulo = getPrime(64 - bits(base) - 1);
//                cerr << "Init first phase" << endl;
                initializeListOfBlocks(*hash, listOfBlocks, input, 0, n, base, modulo, level, plainMarks, buffer1);
//                cerr << "End of the first phase" << endl;
                size_t firstPos = (groupSize - 1) * (blockLengthLevel[level]);
                long segmentLength = blockLengthLevel[level] * (groupSize - 1);
                size_t lim = n - segmentLength;
                ulong i = firstPos;

                //i'm currently using two input buffers, but we can simulate the
                //same by using just one. The problem appears if the given bufferReadSize
                //is smaller than the blockLengthLevel[level]
                IfstreamBuffer<T> inBufferHead(bufferReadSize, buffer1, 0);
                IfstreamBuffer<T> inBufferTail(bufferReadSize, buffer1, 0);
//                cerr << "After creating input buffers" << endl;
                assert(max_possible_v > 0);
                vector<ulong> counter(max_possible_v + 1, 0);
                for (size_t j = 0; j < i; j++) {
                    counter[inBufferHead[j]]++;
                    symbolFound[inBufferHead[j]] = true;
                }
                for (uint j = 0; j <= max_possible_v; j++) ranksBlocks[j].push_back(0);//the first sample of counters is at position -1 --> they are all 0

                //compute the RK for the starting position
                size_t v = rabinKarpBlock(inBufferHead, firstPos, firstPos + segmentLength - 1, base, modulo);

                vector<pair<ulong, ulong>> blockAndPointer;

                this->logger.INFO("Starting parsing","BLockGraph");
                size_t h_rabin_karp = computeH_RK(base, (unsigned long long) segmentLength, modulo);
                ulong sampling_debug = (1UL<<bits(n / 100))-1;
                ulong count_debug = 0;
                ulong total_debug = n / sampling_debug;

                auto currentDateTime =[] () {
                    std::chrono::time_point<std::chrono::system_clock> t;
                    t = std::chrono::system_clock::now();
                    std::time_t end_time = std::chrono::system_clock::to_time_t(t);
                    stringstream ss;
                    ss << std::ctime(&end_time);
                    return ss.str();
                };

//            long falseHits = 0;
                /*Iterate the input searching for the first occurrence of each block*/

                stringstream progress;
                do {
                    if ((i&sampling_debug) == sampling_debug){

                        progress<<count_debug++ << "/" << total_debug;
                        progress << ". Time: " << currentDateTime();
                        progress << "#Tries/iteration: " << hash->GetNTries()*1.0/i << endl;
//                        cerr <<count_debug++ << "/" << total_debug;
//                        cerr  << ". Time: " << currentDateTime();
//                        cerr << "#Tries/iteration: " << hash->GetNTries()*1.0/i << endl;

                    }
                    incrementCounterRank(level, i, blockLengthLevel[level], blockLengthPreviousLevel, ranksBlocks, counter);
                    //store the rank of each symbol every sampling step
                    //increment the rank counter (TODO: Move this to incrementCounterRank)
                    counter[inBufferTail[i]]++;
                    symbolFound[inBufferTail[i]]=true;
                    //compute the RK slide
                    v = rabinKarpRoll(inBufferTail[i], inBufferHead[i + segmentLength], v, h_rabin_karp, base, modulo);
#ifdef MYDEBUG
                    assert(input[i] == inBufferTail[i]);
#endif
                    i++;
                    //assert(rabinKarpBlock(inBufferHead, i, i+segmentLength-1, base, modulo)==v);
                    unsigned long long pos;
                    if ((pos = hash->search(v, v)) != -1ULL) {
                        //the signature for the current block is found in the hash
                        ulong blockId = hash->getAt(pos);
                        ulong first_occ = listOfBlocks.getFirstOcc(blockId);
                        if (listOfBlocks.isUndefined(first_occ) || ((i % blockLengthLevel[level]) == 0ul
                                                                    && (blockId != i / blockLengthLevel[level]))) {
                            //check if actually the block covered by "block" is the same than that in [i,blockLengthLevel[level])
//                        if (SequenceLZBLockHelper::checkBlocks(inBufferHead,
//                                (size_t) (blockId * blockLengthLevel[level]), i, (size_t) segmentLength)) {
                            if (!listOfBlocks.isUndefined(first_occ)) {
                                assert((i % blockLengthLevel[level]) == 0);
                                //if we are covering exactly a block, we compute the hash and the returned value is
                                //not the block with id = i/blockLengthLevel[level], then it means that two blocks are equal and then
                                //they have the same signature. Thus, if block has a pointer to a previous block (it occurred before),
                                //then we have to set the pointer of the block i/blockLengthLevel[level] to where block points to.
                                //Note that each search in the hash for the same key always return the same block (since the hash is deterministic).
                                //Thus, anytime we search for v we obtain block, which is the one that has
                                //listOfBlocks contains a pointer to what it is stored in the Hash. Besides, the block "i" is not marked (not marked by default).
                                if (!plainMarks[i / blockLengthLevel[level]]) {
                                    // bpList.push_front(make_pair(i / blockLengthLevel[level], block->getFirstOcc()));
                                    updateBlockAndPointer(i / blockLengthLevel[level], first_occ, blockAndPointer);
                                }

                                listOfBlocks.setFirstOcc(i / blockLengthLevel[level], first_occ);
#ifdef MYDEBUG
                                for (ulong j = 0; j < segmentLength; j++) {
                                    assert(input[i + j] == input[first_occ + j]);
                                }
#endif
                            } else {//however, if it is the first occurrence of a block (block->first_occ is set to -1)
                                //set the pointer for that block
                                listOfBlocks.setFirstOcc(blockId, i);
                                //mark the blocks that contains the first of occurrence of listOfBlocks[blockId]
                                ulong iniBlock, endBlock;
                                iniBlock = i / blockLengthLevel[level];
                                endBlock = iniBlock + (groupSize - 1) + ((i % blockLengthLevel[level]) ? 1 : 0);
                                for (ulong j = iniBlock; j < endBlock; j++)
                                    plainMarks[j] = true;

                                if (!plainMarks[blockId]) {
                                    updateBlockAndPointer(blockId, i, blockAndPointer);
//                                blockAndPointer.push_back(make_pair(block->getBlockId(), block->getFirstOcc()));
                                    assert(plainMarks[blockId] == false);
                                }
                            }
//                        }else{
//                            falseHits++;
//                        }
                        }
#ifdef HARDDEBUG
                        assert(SequenceLZBLockHelper::checkBlocks(input,(size_t) (listOfBlocks.getBlockId(blockId) * blockLengthLevel[level]), i,segmentLength ));
                    #endif
                    }
                } while (i < lim);


                this->logger.INFO(progress.str(),"BlockGraph");
                this->logger.INFO("Second phase finished","BlockGraph");
//                cerr << "Second phase finished" << endl;
//                cerr << "False hits: " << falseHits << endl;
                //process the remaining blockLengthLevel[level] elements
                //this should be moved to a function only implemented by
                //SequenceBlockGraph
                for (; i < n; i++) {
                    if (i % blockLengthLevel[level] == 0) {
                        for (uint j = 0; j <= max_possible_v; j++) {
                            ranksBlocks[j].push_back(counter[j]);
                        }
                    }
                    //counter[inBufferHead[i]]++;
                    counter[inBufferHead[i]]++;
                    symbolFound[inBufferHead[i]]=true;
                }

                //why (((n-1)%blockLengthLevel[level])==(blockLengthLevel[level]-1)) ?
                //it might happen that the last position "pos" of the sequence is s.t. "(pos+1)%blockLength==0"
                //For rank, when we find such a position we return the rank immediately since we have it stored. Thus,
                //we have to store that rank for the last position if we want to make the algorithm correct.
                if ((int) level == firstLevel ||
                    (((n - 1) % blockLengthLevel[level]) == (blockLengthLevel[level] - 1))) {
                    for (uint j = 0; j <= max_possible_v; j++) {
                        ranksBlocks[j].push_back(counter[j]);
                    }
                }

                if (n % blockLengthLevel[level]) {
                    plainMarks[nBlocks - 1] = true;
                    listOfBlocks.setFirstOcc(nBlocks - 1, blockLengthLevel[level] * n / blockLengthLevel[level]);
                }
                inBufferHead.close();
                inBufferTail.close();

                //recompute max_v (now we accuretly now its value).
                min_v = max_possible_v;
                for (T i = 0; i < max_possible_v; i++)
                    if (symbolFound[i]) {
                        min_v = i;
                        break;
                    }
                max_v = (T) 0;
                for (T i = max_possible_v; i >= 0; i--) {
                    if (symbolFound[i]) {
                        max_v = i;
                        break;
                    }
                }
//                cerr << "After parsing processing" << endl;
                postProcessMarks(level, nBlocks, listOfBlocks, plainMarks, blockAndPointer);
                //build the bitmap that tells which block is marked
                this->logger.INFO("Compressing bitmap at level " + level,"BlockGraph");
//                cerr << "Compressing bitmap at level " << level << endl;
                compressMarks(level, plainMarks);
                {
                    stringstream ss;
                    ss << "Level: " << level << ", blpckLength: " << blockLengthLevel[level] << ", n(sequence n): " <<
                    n;
                    ss << ", #Blocks: " << n / blockLengthLevel[level] << endl;
                    this->logger.INFO(ss.str(),"BlockGraph");
                }
                markBlockBoundaries(level, plainMarks, boundaries);
                this->logger.INFO("After marking boundaries","BlockGraph");
//                cerr << "After marking boundaries" << endl;
                //store the ranksOffsets of the blocks and the pointers back with their offset.
                pointer_new_input = storePointersBack(input, n, level, blockLengthPreviousLevel,listOfBlocks, pointersUnmarked,
                                                      offsetsUnmarked, ranksBlocks, ranksOffsets, buffer1, buffer2, blockAndPointer);

#ifdef MYDEBUG
                assert(testSegmentsIndex(level, boundaries, pointer_new_input));
#endif
                delete hash;
                this->logger.INFO("Building compressed BG","BlockGraph");
                buildCompressBG(level, pointersUnmarked, offsetsUnmarked);
                this->logger.INFO("Building compressed rank","BlockGraph");
                buildCompressRank(level, ranksBlocks, ranksOffsets, symbolFound);

#ifdef MYDEBUG
                assert(testParsing(sequence_tests, level));
                assert(testRanks(sequence_tests, level));
#endif
//                cerr << "After compressing ranks" << endl;
//                cerr << "Returning" << endl;
            }
        }
        delete[] ranksBlocks;
        delete[] ranksOffsets;
        return pointer_new_input;
    }

    template<typename T, class Leaves> void  BlockGraph<T,Leaves>::postProcessMarks(uint level, ulong nBlocks, BlockContainer &listOfBlocks,vector<bool> &plainMarks, vector<pair<ulong,ulong>> &blockAndPointer){
        uint nMark = 0;
        for (ulong j = 0; j < nBlocks; j++) {
            if (listOfBlocks.isUndefined(listOfBlocks.getFirstOcc(j))) {
                listOfBlocks.setFirstOcc(j,j * blockLengthLevel[level]);
                plainMarks[j] = true;
            }
            if (plainMarks[j]) {
                nMark++;
            }
        }
//        cerr << "nMarks: " << nMark << endl;
        //if more than, say, 90% of the blocks of a level are marked, then I mark them all and
        //continue in the next level with smaller blocks (from n/r to n/r^2). I can do this if
        //the same was true for the previous levels (in that case firstLevel is equal to -1).
        if (firstLevel == -1 && nMark * 1.0 / plainMarks.size() >= 0.9) {
            for (ulong j = 0; j < nBlocks; j++) {
                if (!plainMarks[j]) {
                    plainMarks[j] = true;
                    listOfBlocks.setFirstOcc(j,j * blockLengthLevel[level]);
                }
            }
            blockAndPointer.resize(0);//remove the elements of the backAndPointer (all of them are marked --> no pointers back)
        } else {
            //set this as the actual first level if it has not been set yet
            if (firstLevel == -1) {
                firstLevel = level;
            }
        }
    }


    template<typename T, class Leaves> ulong  BlockGraph<T,Leaves>::compressMarks(uint level, vector<bool> &plainMarks){
        if (bsb_marks== nullptr){
            bsb_marks = new BitSequenceBuilderPlain(32);
        }
        uint *bmp;
        createEmptyBitmap(&bmp, plainMarks.size());
        for (uint i = 0; i < plainMarks.size(); i++) {
            if (plainMarks[i]) bit_set(bmp, i);
        }
        marks[level] = bsb_marks->build(bmp, plainMarks.size());
        assert(marks[level]->getLength()==plainMarks.size());

        delete[] bmp;
//#ifdef MYDEBUG
        {
            stringstream ss;
            ss << "Total marks level " << level << ": " << marks[level]->getLength() << ", marked: " <<
            marks[level]->rank1(marks[level]->getLength() - 1) << " (as % " <<
            marks[level]->rank1(marks[level]->getLength() - 1) * 100.0 / marks[level]->getLength() << ")" << endl;
            this->logger.INFO(ss.str(),"BlockGraph");
        }
        //Marks level as firstLevel if the previous levels contain all blocks as marked but not in this.
        if (firstLevel==-1 && marks[level]->rank1(marks[level]->getLength()-1)!=marks[level]->getLength()){
            firstLevel = level;
        }
        return 0;
    }
    template<typename T, class Leaves> bool  BlockGraph<T,Leaves>::testSegmentsIndex(uint level, vector<bool> &boundaries, ulong len){
        return true;
    }

    template<typename T, class Leaves> ulong  BlockGraph<T,Leaves>::buildCompressBG(uint level,vector<uint> &pointersUnmarked, vector<uint> &offsetsUnmarked){
        if (!pointersUnmarked.empty() && !offsetsUnmarked.empty()){
            transform(pointersUnmarked.begin(),pointersUnmarked.end(),pointersUnmarked.begin(),[=](ulong x){return marks[level]->rank1(x)-1;});
            pointersBack[level] = new ArrayDA(&pointersUnmarked[0], pointersUnmarked.size());
            offsetsBack[level]  = new ArrayDA(&offsetsUnmarked[0],offsetsUnmarked.size());
        }
        return 0;
    }

    template <typename T, class Leaves> void  BlockGraph<T,Leaves>::initializeListOfBlocks(HashTableBG<ulong> &hash, BlockContainer &listOfBlocks, T *input, const ulong start, const ulong end,
                                                                                           const size_t base, const size_t modulo,  const uint level, vector<bool> &plainMarks, string &nameBufferRead){
        long n = end-start;
        stringstream ss;
        //TODO: adjust the buffer sizes
        ss << "Function initialize list of blocks (first RK-pass)" << endl;
        ss << "Creating the input buffer" << endl;
        ss << "Buffer parameters: " << endl;
        ss << "\tbufferSize: " << bufferReadSize << endl;
        ss << "\tname: " << nameBufferRead << endl;
        IfstreamBuffer<T> it(bufferReadSize,nameBufferRead,0);
        ss << "Block length of " << blockLengthLevel[level] << " at level " << level << endl;
        long index = 0;
        long delta = blockLengthLevel[level];
        long block = 0;
        long limSup;
        //first block are directly marked
        const long firstBlocks = groupSize;
        const long gs = groupSize-1;
        ss << "Starting loop" << endl;
        ss << "n: " << n << endl;
        ss << "Increment: " << blockLengthLevel[level]*gs << endl;
        ss << "BlockLengthLevel: " << blockLengthLevel[level] << endl;
        ss << "gs: " << gs << endl;
        const long groups_debug = 100;
        const long printing_period = degreeLevel[level];
        long iter_debug = 0;
        // ss << "Progress: ";
        ss << "Start computing RK for each block" << endl;
        while ((limSup=index+(blockLengthLevel[level]*gs))<=n){
            auto v = rabinKarpBlock(it,index,limSup-1,base, modulo);
            if (block<firstBlocks){
                //the first groupSize blocks are marked (generally two)
                listOfBlocks.setFirstOcc(block, index);
//                listOfBlocks.push_back(BlockHash(block,index));
                plainMarks[block]=true;
            }else{
                listOfBlocks.setUndefFirstOcc(block);
//                listOfBlocks.push_back(BlockHash(block));
            }
            // if (iter_debug%printing_period == 0){
            //     ss << iter_debug/printing_period  << "/" << printing_period << endl;
            // }
            // iter_debug++;
            hash.insert(v,v,block);
            index +=delta;
            block++;
        }
        ss << endl;
        ss << "Processing the remaining elements" << endl;
        //the last partial block...
        while (index<n) {
            auto v = rabinKarpBlock(it, (size_t)index, (size_t)min((ulong)n - 1ul,(ulong)index+blockLengthLevel[level]), base, modulo);
//            listOfBlocks.push_back(BlockHash(block));
            listOfBlocks.setUndefFirstOcc(block);
//            listOfBlocks.setFirstOcc(block  , -1);
            hash.insert(v, v, block);
            index+=delta;
            block++;
        }
        ss << "First RK-pass finished" << endl;
        this->logger.INFO(ss.str(),"BlockGraph");
    }

    template<typename T, class Leaves> void  BlockGraph<T,Leaves>::save(ofstream & fp) const
    {
        if (output==nullptr){
            for (int level=0;level<((int)this->strategySplitBlocks->getNLevels())-1;level++){
                saveLevel(fp, level,false);
            }
            bool saveLeaves = true;
            saveLevel(fp,this->strategySplitBlocks->getNLevels()-1,saveLeaves);
        }else{
//            this->logger.WARN("Warning: the sequence is already written on disk. Save function does nothing.","BlockGraph");
            cout << "Warning: the sequence is already written on disk. Save function does nothing." << endl;
        }
    }

    /*
        This function can be called after the construction of each level or once the
        whole data structure is built. For the first case, saveLeaves must be set to false (by default
        is set to false). For the former, it must be when we are at the last level of the block
        graph (the level that contains the leaves). That is necessary to force the writting of the
        sequence of leaves.
     */
    //TODO: if all blocks are marked in a given level, the structures for rank are set to NULL, and thus we have problems
    //when trying to save/load them (as well as an space performance problem).
    template<typename T, class Leaves> void  BlockGraph<T,Leaves>::saveLevel(ofstream & fp, int level, bool saveLeaves) const
    {
        if (level==0) {
            uint wr = getHeader();
            saveValue(fp, wr);
            saveValue(fp, n);
            saveValue(fp, max_v);
            strategySplitBlocks->save(fp);
            saveValue(fp, groupSize);

        }
        saveValue(fp,level);
        bool skipLevel = (level<firstLevel || firstLevel==-1);
        saveValue(fp,skipLevel);
        saveValue(fp, firstLevel);

        bool leaves = (seq_leaves!=nullptr && saveLeaves);
        saveValue(fp,leaves);
        if (skipLevel && !leaves) return;

        if (leaves) {
            saveValue(fp, blockLengthLevel[level]);
            saveValue(fp, degreeLevel[level]);
            seq_leaves->save(fp);
        }else {
            //intermediate level
            saveValue(fp, blockLengthLevel[level]);
            saveValue(fp, degreeLevel[level]);
            marks[level]->save(fp);
            pointersBack[level]->save(fp);
            offsetsBack[level]->save(fp);
            saveLevelRank(fp, level);
        }
    }

    template<typename T, class Leaves>  BlockGraph<T,Leaves>*  BlockGraph<T,Leaves>::load(ifstream & fp) {
        uint wr = loadValue<uint>(fp);
        switch(wr){
            case SEQUENCE_BLOCK_GRAPH_HDR:
                return SequenceBlockGraph<T,Leaves>::load(fp);
            case BLOCKGRAPH_HDR:
                BlockGraph<T,Leaves> *ret = new BlockGraph<T,Leaves>();
                return ret->loadByLevels(fp);
        }
        return nullptr;
    }

    template<typename T, class Leaves> BlockGraph<T,Leaves> *  BlockGraph<T,Leaves>::loadByLevels(ifstream & fp) {

        seq_leaves = nullptr;
        output = nullptr;
        bool first = false;
        for(uint level=0;!seq_leaves;level++) {
            if (level == 0) {
                uint wr = loadValue<uint>(fp);
                if (wr!= getHeader()){
                    return nullptr;
                }
                n = loadValue<size_t>(fp);
                max_v = loadValue<T>(fp);
                strategySplitBlocks = StrategySplitBlock::load(fp);
                groupSize = loadValue<uint>(fp);
            }

            uint lev = loadValue<uint>(fp);
            assert(lev==level);

            bool skipLevel = loadValue<bool>(fp);
            if (!skipLevel && !firstLevel){
                firstLevel = level;
            }

            firstLevel = loadValue<uint>(fp);

            //assert((skipLevel && (int)level<firstLevel) || !skipLevel);
            bool loadLeaves = loadValue<bool>(fp);
            if (loadLeaves) {
                auto bll = loadValue<ulong>(fp);
                blockLengthLevel.push_back(bll);
                auto d = loadValue<ulong>(fp);
                degreeLevel.push_back(d);
                seq_leaves = Sequence::load(fp);
            }else {
                //   size_t pos = fp.tellg();
                //intermediate level
                blockLengthLevel.push_back((skipLevel)?0:loadValue<ulong>(fp));
                degreeLevel.push_back((skipLevel)?0:loadValue<ulong>(fp));
                marks.push_back((skipLevel)?nullptr:BitSequence::load(fp));
                pointersBack.push_back((skipLevel)?nullptr:DirectAccess::load(fp));
                offsetsBack.push_back((skipLevel)?nullptr:DirectAccess::load(fp));

                if (!skipLevel) {
                    loadByLevelsRank(fp, level);
                }
                //  size_t end_pos = fp.tellg();
                // assert((skipLevel && (pos==end_pos)) || (!skipLevel));
            }
        }
        return this;
    }

    template<typename T, class Leaves> void   BlockGraph<T,Leaves>::deleteLevelRanks(uint level){}

    template<typename T, class Leaves>void  BlockGraph<T,Leaves>::deleteLevel(uint level){

//        bool skipLevel = (firstLevel==-1) || ((int)level<firstLevel);
//        if (skipLevel) return;
        if (level>=this->firstLevel) {
            if (marks[level]) {
                delete marks[level];
                marks[level] = nullptr;
            }
            if (pointersBack[level]) {
                delete pointersBack[level];
                marks[level] = nullptr;
            }
            if (offsetsBack[level]) {
                delete offsetsBack[level];
                marks[level] = nullptr;
            }
            deleteLevelRanks(level);
        }
    }

    template<typename T, class Leaves> ulong  BlockGraph<T,Leaves>::storePointersBack(T *&input, ulong n, uint level, ulong blockLengthPreviousLevel, BlockContainer &listOfBlocks,
                                                                                      vector<uint> &pointersUnmarked, vector<uint> &offsetsUnmarked, vector<uint> *ranksBlocks, vector<uint> *ranksOffsets,
                                                                                      string &bufferRead, string &bufferWrite, vector<pair<ulong,ulong>> &blockAndPointers){

#ifdef MYDEBUG
        vector<T> debInput(n);
        for (ulong j=0;j<n;j++)
            debInput[j]=input[j];

        T *new_input = new T[n];
        for (ulong i=0;i<n;i++)new_input[i]=input[i];
#endif

        //reserve space for the unmarked blocks
        ulong nBlocks = marks[level]->getLength();
        ulong nNotMarked = marks[level]->rank0(marks[level]->getLength()-1);
        if (nNotMarked>0) {
            pointersUnmarked.reserve(nNotMarked);
            offsetsUnmarked.reserve(nNotMarked);
        }

        //we could move using rank/select. However, access is realy cheap using a PlainRepresenation
        //(a way more than select).

        size_t pointer_new_input = 0;
        IfstreamBuffer<T> it(bufferReadSize,bufferRead);
//        ss << "Writting at " << bufferWrite << endl;
//        ss << "Buffer settings: " << endl;
//        cerr << "\tcapacity: " << this->bufferWriteSize << endl;
        OutBuffer <T> out(this->bufferWriteSize,bufferWrite);
        for (ulong i=0;i<nBlocks;i++){
            if (!marks[level]->access(i)) {
                ulong pos = pointersUnmarked.size();
                pointersUnmarked.push_back(listOfBlocks.getFirstOcc(i) / blockLengthLevel[level]);
                offsetsUnmarked.push_back((pointersUnmarked[pos] + 1) * blockLengthLevel[level] - listOfBlocks.getFirstOcc(i) - 1);
            }else {
                ulong ini = i * blockLengthLevel[level];
                ulong end = min(n, ini + blockLengthLevel[level]);
                it[ini];
                for (ulong j = ini; j < end; j++) {
#ifdef MYDEBUG
                    new_input[pointer_new_input] = input[j];
                    positions[pointer_new_input] = positions[j];
#endif
                    pointer_new_input++;
                    out.push_back(*it);
                    it++;
                }
            }
        }
//        for (auto iter = blockAndPointers.begin();iter != blockAndPointers.end();++iter){
//            if (!marks[level]->access((*iter).first)) {
//                ulong blockId = marks[level]->rank0((*iter).first) - 1;
//                //points to the block where the left limit of the interval overlaps
//                pointersUnmarked[blockId] = (*iter).second / blockLengthLevel[level];
//                //the offset with regard to the end of that overlaped block
//                offsetsUnmarked[blockId] = (pointersUnmarked[blockId] + 1) * blockLengthLevel[level] - (*iter).second - 1;
//            }
//        }



#ifdef MYDEBUG
        delete [] input;
        input = new_input;
#endif
        return pointer_new_input;
    }

    template<typename T, class Leaves> void  BlockGraph<T,Leaves>::indexLeaves(string &inputBuffer,
                                                                               uint level, const vector<bool> &boundaries){

        vector<uint> leaves;
        SequenceLZBLockHelper::loadAsUints<T>(inputBuffer,leaves);
        //The sequences implementations in libcds only take uint* as input.
        vector<uint> inputInts;
        inputInts.reserve(leaves.size());
        for (auto &x: leaves) inputInts.push_back((uint)x);
        seq_leaves = (Leaves*)new DAC(&inputInts[0],inputInts.size(),false);
    }

    template<typename T, class Leaves> bool  BlockGraph<T,Leaves>::testParsing(vector<T> &sequence_test, uint level){

        if ((int)level<firstLevel) return true;
        if ((uint)(level+1)==(uint)this->strategySplitBlocks->getNLevels()) return true;
        ulong segmentLength = blockLengthLevel[level]*(groupSize-1);
        auto nblocks = marks[level]->getLength();
        for (uint i=0;i<nblocks;i++){
            if (marks[level]->access(i)) continue;
            size_t block_index, block_pos, unmarked_index;
            getPointerBack(level,i, unmarked_index, block_index, block_pos);

            auto block_end_pos = (block_pos+1)*blockLengthLevel[level];
            auto post_back_from_end = block_end_pos - (*offsetsBack[level])[unmarked_index]-1;
            for (size_t j=0;j<segmentLength;j++){
                assert(sequence_test[post_back_from_end+j]==sequence_test[i*blockLengthLevel[level]+j]);
            }
        }
        return true;
    }



    /***********************************************************************************************/
    /***********************************************************************************************/
    /***********************************************************************************************/
    /***********************************************************************************************/

    template<typename T, class Leaves> T  BlockGraph<T,Leaves>::accessLeaves(long pos) const{
        assert(pos<(long)seq_leaves->getLength());
        return (T)seq_leaves->access(pos);
    }

    /**
    block: is the block that contains the position
    unmarked_index: how many unmarked blocks are before block
    block_index: marks->rank1(block)-1 (-1 because it is an index -indexes start in 0-)
    block_pos: index of the pointed block
    */
    template<typename T, class Leaves> uint  BlockGraph<T,Leaves>::getPointerBack(uint level, size_t block, size_t &unmarked_index, size_t &block_index, size_t &pointed_block_pos) const{
        assert(marks[level]->access(block)==0);
        unmarked_index      = marks[level]->rank0(block)-1;
        block_index         = (*pointersBack[level])[unmarked_index];
        pointed_block_pos   = marks[level]->select1(block_index+1);
        return 0;
    }

    template<typename T, class Leaves> uint  BlockGraph<T,Leaves>::operator[](size_t pos) const{
        uint level = firstLevel;
        if (level==-1)return accessLeaves(pos);
        while(!strategySplitBlocks->stop(level)){
            size_t block_index = pos / blockLengthLevel[level];
            if (!marks[level]->access(block_index)){
                //find the position "pos" corresponds to inside the block that is pointing
                size_t unmarked_index, block_index_of_1s,pblock_pos;
                getPointerBack(level,block_index, unmarked_index, block_index_of_1s, pblock_pos);
                size_t block_end_pos    = (pblock_pos+1)*blockLengthLevel[level];
                block_index             = pblock_pos;
                size_t block_ini        = block_end_pos - (*offsetsBack[level])[unmarked_index]-1;
                size_t offset           = pos % blockLengthLevel[level];
                pos                     = block_ini + offset;
            }
            //number of elements of marked blocks that are before block "block_index"
            pos = pos - marks[level]->rank0(block_index-1) * blockLengthLevel[level];
            level++;
        }
        return accessLeaves(pos);
    }

    /***********************************************************************************/
    /***********************************************************************************/
    /*********************  SEQUENCE BLOCK GRAPH IMPLEMENTATION  ***********************/
    /***********************************************************************************/
    /***********************************************************************************/

    template<typename T, class Leaves> void SequenceBlockGraph<T,Leaves>::updateBlockAndPointer
            (ulong blockId, ulong firstOcc, vector<pair<ulong,ulong>> &blockAndPointers){
        blockAndPointers.push_back(std::make_pair(blockId,firstOcc));
    }

    template<typename T, class Leaves> SequenceBlockGraph<T,Leaves>&  SequenceBlockGraph<T,Leaves>::setLeavesBuilder(SequenceBuilder *sb){
        this->seq_builder = sb;
        return *this;
    }

    template<typename T, class Leaves> SequenceBlockGraph<T,Leaves>::SequenceBlockGraph(StrategySplitBlock *st, uint gs):BlockGraph<T,Leaves>(st,gs),Sequence(0){
        seq_builder = nullptr;
        this->output      = nullptr;
//        this->ranksBlocksDA.reserve();
//        this->ranksOffsetsDA.reserve(this->NLevels);
    }

    template<typename T, class Leaves> void SequenceBlockGraph<T,Leaves>::build(string &inFile, bool can_delete){
        BlockGraph<T,Leaves>::build(inFile,can_delete);
        this->length = this->n;
    }

    template<typename T, class Leaves> SequenceBlockGraph<T,Leaves>* SequenceBlockGraph<T,Leaves>::load(ifstream &fp){
        SequenceBlockGraph<T,Leaves> *ret = new SequenceBlockGraph<T,Leaves>();
        ret->loadByLevels(fp);
        ret->length = ret->n;
        return ret;
    }


    template<typename T, class Leaves> void SequenceBlockGraph<T,Leaves>::save(ofstream &fp) const{
        BlockGraph<T,Leaves>::save(fp);
    }

    template<typename T, class Leaves> void  SequenceBlockGraph<T,Leaves>::saveLevelRank(ofstream &fp, int level) const{
        uint wr = SEQUENCE_BLOCK_GRAPH_HDR;
        saveValue(fp,wr);
        for (uint j = 0; j <= this->max_v; j++) {
            bool valid = (ranksBlocksDA[level][j]!=nullptr);
            saveValue(fp,valid);
            if (valid) {
                this->ranksBlocksDA[level][j]->save(fp);
            }
        }
        for (uint j = 0; j <= this->max_v; j++) {
            bool valid = (ranksBlocksDA[level][j]!=nullptr);
            saveValue(fp,valid);
            if (valid) {
                this->ranksOffsetsDA[level][j]->save(fp);
            }
        }
    }

    template<typename T, class Leaves>  void SequenceBlockGraph<T,Leaves>::loadByLevelsRank(ifstream & fp, uint level){
        uint wr = loadValue<uint>(fp);
        if (wr!= SEQUENCE_BLOCK_GRAPH_HDR){
            return;
        }

        if (ranksBlocksDA.size()<level){
            auto iter =level-ranksBlocksDA.size();
            for (auto i=0;i<iter;i++){
                ranksBlocksDA.push_back(nullptr);
                ranksOffsetsDA.push_back(nullptr);
            }
        }
        ranksBlocksDA.push_back(new DirectAccess*[this->max_v+1]);
        ranksOffsetsDA.push_back(new DirectAccess*[this->max_v+1]);
        for (uint j = 0; j <= this->max_v; j++) {
            bool valid = loadValue<bool>(fp);
            this->ranksBlocksDA[level][j] = nullptr;
            if (valid) {
                this->ranksBlocksDA[level][j] = DirectAccess::load(fp);
                assert(this->ranksBlocksDA[level][j]);
            }
        }
        for (uint j = 0; j <= this->max_v; j++) {
            bool valid = loadValue<bool>(fp);
            this->ranksOffsetsDA[level][j] = nullptr;
            if (valid) {
                this->ranksOffsetsDA[level][j] = DirectAccess::load(fp);
            }
        }

    }

    template<typename T, class Leaves> void SequenceBlockGraph<T,Leaves>::incrementCounterRank(uint level, ulong i, ulong blockLength, ulong blockLengthPreviousLevel, vector<uint> *ranksBlocks, vector<ulong> &counter){
        //Blocks are stored all together but operations require they are isolated: we must
        //recall that rank counters start at a block boundary (with regard to the previous level).
        if ((i%blockLengthPreviousLevel)==0){
            size_t cnt = 0UL;
            for (const auto &it:counter)
                cnt+=it;
//            if ((cnt%blockLengthPreviousLevel)!=0)
//                cerr << endl;
            assert((cnt%blockLengthPreviousLevel)==0);
            for (auto &it:counter)
                it=0;
        }
        if ((i % blockLength) == 0) {
            for (uint j = 0; j <= this->max_possible_v; j++) {
                ranksBlocks[j].push_back(counter[j]);
            }
        }
    }

    template<typename T, class Leaves> ulong SequenceBlockGraph<T,Leaves>::buildCompressRank(uint level,vector<uint> *ranksBlocks, vector<uint> *ranksOffsets,
                                                                                             const vector<bool> &symbolFound){

        //build the compressed data structures to store the ranks
        this->ranksBlocksDA.push_back(new DirectAccess *[this->max_v+1]);
        this->ranksOffsetsDA.push_back(new DirectAccess *[this->max_v+1]);
        uint ss = 20;
        for (uint j = 0; j <= this->max_v; j++) {
            if (symbolFound[j]) {
                this->ranksBlocksDA[level][j] = new DAC(&ranksBlocks[j][0], ranksBlocks[j].size(), ss);
                this->ranksOffsetsDA[level][j] = new ArrayDA(&ranksOffsets[j][0], ranksOffsets[j].size());
            }else{
                this->ranksBlocksDA[level][j]  = nullptr;
                this->ranksOffsetsDA[level][j] = nullptr;
            }
        }
        return 0;
    }

    template<typename T, class Leaves> SequenceBlockGraph<T,Leaves> &SequenceBlockGraph<T,Leaves>::setSequenceBuilder(SequenceBuilder *seq_builder){
        this->seq_builder = seq_builder;
        return *this;
    }

    template<typename T, class Leaves> ulong SequenceBlockGraph<T,Leaves>::storePointersBack(T *&input, ulong n, uint level, ulong blockLengthPreviousLevel, BlockContainer &listOfBlocks,
                                                                                             vector<uint> &pointersUnmarked, vector<uint> &offsetsUnmarked, vector<uint> *ranksBlocks, vector<uint> *ranksOffsets,
                                                                                             string &bufferRead, string &bufferWrite, vector<pair<ulong,ulong>> &blockAndPointers){

        stringstream ss;
        ss << "Sorting " << blockAndPointers.size() << " elements" << endl;
        //sorts the unmarked blocks by the block they are pointing to. This is useful to store the ranks
        //offsets as we are processing the input in storePointersBack(...). (it avoids random reads).
        sort(blockAndPointers.begin(),blockAndPointers.end(),
             [](const pair<ulong, ulong> &a, const pair<ulong, ulong> &b) {return a.second < b.second;});

#ifdef MYDEBUG
        ss << "Loading input for debugging" << endl;
        vector<T> debInput(n);
        for (ulong j=0;j<n;j++)
            debInput[j]=input[j];

        T *new_input = new T[n];
        for (ulong i=0;i<n;i++)new_input[i]=input[i];
#endif
        ss << "Writting at " << bufferWrite << endl;ss << "Buffer settings: " << endl;ss << "\tcapacity: " << this->bufferWriteSize << endl;


        //reserve space for the unmarked blocks
        ulong nNotMarked = this->marks[level]->rank0(this->marks[level]->getLength()-1);
        if (nNotMarked>0) {
            pointersUnmarked.resize(nNotMarked);
            offsetsUnmarked.resize(nNotMarked);
            for (uint j=0;j<=this->max_v;j++){
                //for each unmarked block we store two offsets at position 2*block and 2*block+1
                ranksOffsets[j].resize(2*nNotMarked);
            }
        }

        vector<pair<ulong,ulong>>::iterator iter = blockAndPointers.begin();
        vector<ulong>        counters(this->max_v+1,0UL);
        IfstreamBuffer<T>   in_iter(this->bufferReadSize,bufferRead,this->blockLengthLevel[level]*(this->groupSize-1));
        OutBuffer <T>       out(this->bufferWriteSize,bufferWrite);
        size_t pointer_new_input = 0UL;

        for (ulong it=0;it<n;it++){
            if ((it%blockLengthPreviousLevel)==0){
                for (auto &iter:counters)
                    iter=0;
            }
            while(iter!=blockAndPointers.end() && (*iter).second==it){
                if (!this->marks[level]->access((*iter).first)) {
                    ulong blockId               = this->marks[level]->rank0((*iter).first) - 1;
                    ulong blockPointed          = (*iter).second / this->blockLengthLevel[level];
                    pointersUnmarked[blockId]   = blockPointed; //points to the block where the left limit of the interval overlaps
                    offsetsUnmarked[blockId]    = (pointersUnmarked[blockId] + 1) * this->blockLengthLevel[level] - (*iter).second - 1; //the offset with regard to the end of that overlaped block

                    for (uint j = 0; j <= this->max_v; j++) {
                        if(counters[j]<ranksBlocks[j][blockPointed]){
                            cerr << endl;
                        }
                        assert(counters[j]>=ranksBlocks[j][blockPointed]);
                        //I'm storing rank_a(B_1,g-1) (see [1])
                        ranksOffsets[j][2 * blockId] = counters[j] - ranksBlocks[j][blockPointed];
                    }
                }
                ++iter;
            }
            if ((it%this->blockLengthLevel[level])==0){
                ulong i = it/this->blockLengthLevel[level];
                //if the block is marked, just write it in a buffer for the next iteration
                if (this->marks[level]->access(i)){
                    auto limUp  = min(n,(i+1)*this->blockLengthLevel[level]);
                    auto limLow = i*this->blockLengthLevel[level];
                    for (ulong j=limLow;j<limUp;j++){
                        out.push_back(in_iter[j]);//input[j]);
#ifdef MYDEBUG
                        new_input[pointer_new_input]=in_iter[j];//input[j];
#endif
                        pointer_new_input++;
                    }
                }else{
                    //the block is not marked --> store ranks
                    ulong blockId   = this->marks[level]->rank0(i)-1;;
                    size_t ini      = pointersUnmarked[blockId] * this->blockLengthLevel[level];
                    size_t end      = ini + this->blockLengthLevel[level] - offsetsUnmarked [blockId]-1;
                    end             = it+(this->blockLengthLevel[level]-(end-ini));
                    //compute the rank from the beginning of the unmarked block until the position that overlaps
                    //the block limit (in the mapped block)

                    for (ulong j=it;j<end;j++){
                        //NOTE BOTH OFFSET RANKS (INSIDE THE MARKED BLOCK AND INSIDE THE UNMARKED ARE STORED CONSECUTIVE )
                        //Thus, let i be the unmarked block rank0_index, then the offset inside the marked block is at position rank0_index*2 and the
                        //offset inside the marked block at position rank0_index*2+1
                        ranksOffsets[in_iter[j]][2*blockId+1]++;
                    }

#ifdef MYDEBUG
                    auto p = (pointersUnmarked[blockId]+1)*this->blockLengthLevel[level];
                    p = p -offsetsUnmarked [blockId] -1;
                    for (uint j=0;j<this->blockLengthLevel[level];j++){
                        assert(debInput[i*this->blockLengthLevel[level]+j]==debInput[p+j]);
                    }
#endif
                }
            }

            counters[in_iter[it]]++;

        }
#ifdef MYDEBUG
        ss << "new_input: " << pointer_new_input << endl;
        delete [] input;
        input = new_input;
#endif
        this->logger.INFO(ss.str(),"BlockGraph");
        return pointer_new_input;
    }


    template<typename T, class Leaves> inline size_t SequenceBlockGraph<T,Leaves>::accessRanksBlocks(uint level, uint symbol, size_t pos) const{
        size_t ret = this->ranksBlocksDA[level][symbol]->access(pos);
        return ret;
        //return ranksBlocks[level][symbol][pos];
    }

    template<typename T, class Leaves> inline size_t SequenceBlockGraph<T,Leaves>::accessRanksOffsets(uint level, uint symbol, size_t pos) const{
        return this->ranksOffsetsDA[level][symbol]->access(pos);
        // return ranksOffsets[level][symbol][pos];
    }

    template<typename T, class Leaves> uint SequenceBlockGraph<T,Leaves>::access(size_t pos) const{
        return (uint) BlockGraph<T,Leaves>::operator[](pos);
    }

    template<typename T, class Leaves> void  SequenceBlockGraph<T,Leaves>::indexLeaves(string &inputBuffer,uint level, const vector<bool> &boundaries){

        vector<uint> leaves;
        SequenceLZBLockHelper::loadAsUints<T>(inputBuffer,leaves);
        //The sequences implementations in libcds only take uint* as input.
        vector<uint> inputInts;inputInts.reserve(leaves.size());
        for (auto &x: leaves) inputInts.push_back((uint)x);

        if (!this->seq_builder){
            Mapper *mc = new MapperCont(&inputInts[0],inputInts.size(),BitSequenceBuilderRG(32),0);
            mc->use();
            seq_builder = new SequenceBuilderWaveletTree(new BitSequenceBuilderRG(32),
                                                         mc, new wt_coder_huff(&inputInts[0],inputInts.size(), mc));
            mc->unuse();
        }
        this->seq_leaves = seq_builder->build(&inputInts[0],inputInts.size());
        delete this->seq_builder;
        this->seq_builder=nullptr;


    }

    template<typename T, class Leaves> size_t SequenceBlockGraph<T,Leaves>::rank(uint symbol, size_t pos) const {
        if ((pos+1)==0) return 0;
        auto level = this->firstLevel;
        if (level==-1)return this->seq_leaves->rank(symbol,pos);
        ulong computed_rank = 0;

        while(!this->strategySplitBlocks->stop(level)){

            //if pos is in a block-boundary-->access the samples.
            if (((pos+1)%this->blockLengthLevel[level])==0){
                return computed_rank + accessRanksBlocks(level,symbol,(pos+1)/this->blockLengthLevel[level]);
            }
            size_t ini_block_index,block_index;
            block_index     = ini_block_index =  pos / this->blockLengthLevel[level];
            computed_rank   += accessRanksBlocks(level,symbol,block_index);

            if (!this->marks[level]->access(ini_block_index)) {
                size_t new_pos, block_ini, block_end_pos, block_pos, unmarked_index, offset;
                size_t block_index_of_1s;
                this->getPointerBack(level,block_index, unmarked_index, block_index_of_1s, block_pos);
                block_end_pos   = (block_pos+1)*this->blockLengthLevel[level];//the ending position of the marked block that is pointed by the current block
                block_index     = block_pos;//the marked block index
                block_ini       = block_end_pos - (*this->offsetsBack[level])[unmarked_index]-1;//the starting position in the pointed block (is the position from the beginning)
                offset          = pos % this->blockLengthLevel[level];//the offset inside the marked block
                new_pos         = block_ini + offset; //we add the offset of "inside the marked block" to the ini_pos in the marked block

                if (((new_pos+1)%this->blockLengthLevel[level])==0){//if new_pos is at the beginning of a block (positions for which we have stored the samples), we solve it almost directly
                    assert(this->marks[level]->access(block_index+1) && this->marks[level]->access(block_index));
                    return computed_rank + accessRanksOffsets(level,symbol,(unmarked_index<<1)+1);
                }else if (new_pos < block_end_pos){
                    computed_rank -=  accessRanksOffsets(level,symbol,unmarked_index<<1);
                }else{
                    computed_rank += accessRanksOffsets(level,symbol,(unmarked_index<<1)+1);
                }
                pos= new_pos;
            }
            auto offset = pos % this->blockLengthLevel[level];
            auto block = (this->marks[level]->rank1(pos/this->blockLengthLevel[level])-1)*this->blockLengthLevel[level];
            pos = block + offset;
            level++;
        }

        auto start = pos / this->blockLengthLevel[level] * this->blockLengthLevel[level];
        return computed_rank + this->seq_leaves->rankInterval(symbol,start,pos);

    }

    template<typename T, class Leaves> inline long SequenceBlockGraph<T,Leaves>::bs_ranks(uint level, uint symbol, size_t l, size_t r, size_t b, size_t &v) const{
        assert(r<(*this->ranksBlocksDA[level][symbol]).getLength());
        long p=0;
        b--;
        while(l<=r){
            p=(l+r)/2;
            v = (*this->ranksBlocksDA[level][symbol])[p];
            if (v>b){
                r = p-1;
            }else if (v<=b){
                l = p+1;
            }
        }
        while(p>0 && v>b){
            p--;
            v = (*this->ranksBlocksDA[level][symbol])[p];
        }
        return p;
    }

    template<typename T, class Leaves>  size_t SequenceBlockGraph<T,Leaves>::select(uint symbol, size_t j) const{
        if (j==0) return static_cast<size_t>(-1);
        auto level = this->firstLevel;
        if (level==-1) return this->seq_leaves->select(symbol,j);
        size_t l=0UL, r=this->ranksBlocksDA[level][symbol]->getLength()-1;
        size_t acum_pos=0;
        while(!this->strategySplitBlocks->stop(level)){
            r = min(r,this->marks[level]->getLength()-1);
            size_t rank_ini_block= 0UL;
            size_t block_index = bs_ranks((uint)level, symbol, l, r, j, rank_ini_block);
            acum_pos += (block_index-l)*this->blockLengthLevel[level];
            j -= rank_ini_block;

            if (!this->marks[level]->access(block_index)){//the block is not marked --> we have to map to a marked block
                size_t  block_pos, unmarked_index, block_index_of_1s;
                this->getPointerBack(level,block_index, unmarked_index, block_index_of_1s, block_pos);
                block_index = block_pos;//the marked block index
                auto rnk_a_Bu_d = accessRanksOffsets(level,symbol,(unmarked_index<<1)+1); // see [1]
               if (j  <=  rnk_a_Bu_d) {
                    //search inside the first block
                   j += accessRanksOffsets(level,symbol,unmarked_index<<1);
                   acum_pos =acum_pos - (this->blockLengthLevel[level]-((*this->offsetsBack[level])[unmarked_index]+1));
                }else {
                    j -=rnk_a_Bu_d;
                   acum_pos += (*this->offsetsBack[level])[unmarked_index]+1;
                   block_index++;
                }
            }
            l = (this->marks[level]->rank1(block_index)-1)*this->degreeLevel[level];
            r = l+this->degreeLevel[level]-1;
            level++;
        }
        size_t start = l/this->degreeLevel[level-1] * this->blockLengthLevel[level];
        size_t end = start + this->blockLengthLevel[level];
        auto ret = this->seq_leaves->selectInterval(symbol,j,start,end);
        if (ret==static_cast<size_t>(-1)) return static_cast<size_t>(-1);
        return acum_pos + ret;
    }

    /**
    * TODO: there might be problems if the input vocabulary is not contiguous (in order to builde
    * the wavelet matrix).
    */
    template<typename T, class Leaves> bool SequenceBlockGraph<T,Leaves>::testRanks(vector<T> &input, uint level){

        SequenceBuilderWaveletMatrix sb(new BitSequenceBuilderRG(32),new MapperNone());
        size_t n = input.size();
        vector<uint> seq_test;
        seq_test.reserve(input.size());
        for (auto &x: input)
            seq_test.push_back((uint)x);

        Sequence *seq = sb.build(&seq_test[0],n);
        ulong nBlocks = n / this->blockLengthLevel[level];
        for (ulong i=0;i<nBlocks;i++){
            size_t pos = i*this->blockLengthLevel[level]-1;
            for (uint j=0;j<=this->max_v;j++){
                if (this->ranksBlocksDA[level][j]) {
                    assert(seq->rank(j, pos) == accessRanksBlocks(level, j, i));
                }
            }
        }
        for (ulong i=0;i<nBlocks;i++){
            //size_t pos = i*this->blockLengthLevel[level]-1;
            if (this->marks[level]->access(i)) continue;
            for (uint j=0;j<=this->max_v;j++){
                if (this->ranksBlocksDA[level][j]) {
                    //size_t rb = accessRanksBlocks(level,j,i);//rank at the beginning of the original block (marked or unmarked)
                    //assert((seq_tests==nullptr) || seq_tests->rank(symbol,ini_block_index*this->blockLengthLevel[level]));
                    //by beginning we mean ini_block_index*blockLength-1
                    size_t block_ini, block_end_pos, block_pos, unmarked_index;
                    size_t block_index_of_1s;

                    this->getPointerBack(level,i, unmarked_index, block_index_of_1s, block_pos);
                    block_end_pos = (block_pos + 1) * this->blockLengthLevel[level];//the ending position of the marked block that is pointed by the current block
                    //size_t new_block_index = block_pos;//the marked block index
                    block_ini = block_end_pos - (*this->offsetsBack[level])[unmarked_index] - 1;//the starting position in the pointed block (is the position from the beginning)
                    //offset = pos % this->blockLengthLevel[level];//the offset inside the marked block
                    //new_pos= block_ini + offset; //we add the offset of "inside the marked block" to the ini_pos in the marked block
                    size_t rank_pointed = seq->rank(j, block_ini - 1);
                    size_t rank_obtained = accessRanksBlocks(level, j, block_pos) + accessRanksOffsets(level, j, 2 * unmarked_index);
                    assert(rank_pointed == rank_obtained);
                }
            }
        }

        delete seq;
        return true;
    }
    template<typename T, class Leaves> void  SequenceBlockGraph<T,Leaves>::deleteLevelRanks(uint level){
        bool skipLevel = (this->firstLevel==-1) || ((int)level<this->firstLevel);
        if (skipLevel) return;
        if (this->ranksBlocksDA[level]) {
            for (uint j = 0; j <= this->max_v; j++) {
                delete this->ranksBlocksDA[level][j];
                this->ranksBlocksDA[level][j] = nullptr;
            }
            delete this->ranksBlocksDA[level];
            this->ranksBlocksDA[level]=nullptr;
        }
        if (this->ranksOffsetsDA[level]) {
            for (uint j = 0; j <= this->max_v; j++) {
                delete this->ranksOffsetsDA[level][j];
                this->ranksOffsetsDA[level][j] = nullptr;
            }

            delete this->ranksOffsetsDA[level];
            this->ranksOffsetsDA[level] = nullptr;
        }
    }
    template<typename T, class Leaves> size_t  SequenceBlockGraph<T,Leaves>::getSizeRank() const{
        uint new_max_v = (this->max_v==1)?1:this->max_v;
        size_t total = 0;
        total+= sizeof(DirectAccess*)*(this->strategySplitBlocks->getNLevels()-1);
        total+= sizeof(DirectAccess*)*(new_max_v);
        for (uint j=this->firstLevel;j<this->strategySplitBlocks->getNLevels()-1;j++){
            for (uint i=0;i<new_max_v;i++){
                total+=this->ranksBlocksDA[j][i]->getSize();
            }
        }
        total+= sizeof(DirectAccess*)*(this->strategySplitBlocks->getNLevels()-1);
        total+= sizeof(DirectAccess*)*(new_max_v);
        for (uint j=this->firstLevel;j<this->strategySplitBlocks->getNLevels()-1;j++){
            for (uint i=0;i<new_max_v;i++){
                total+=this->ranksOffsetsDA[j][i]->getSize();
            }
        }
        return total;
    }
    template<typename T, class Leaves> size_t  SequenceBlockGraph<T,Leaves>::getLength() const{
        return this->length;
    }

    template<typename T, class Leaves> size_t  SequenceBlockGraph<T,Leaves>::getSize() const{
        return  BlockGraph<T,Leaves>::getSize();
    }

    template<typename T, class Leaves> SequenceBlockGraph<T,Leaves>::SequenceBlockGraph()
            :BlockGraph<T,Leaves>(),Sequence(0){}

    template<typename T, class Leaves> uint SequenceBlockGraph<T,Leaves>::getHeader() const{
        uint w = SEQUENCE_BLOCK_GRAPH_HDR;
        return w;
    }


    template<typename T, class Leaves> BlockGraphIndex<T,Leaves>::~BlockGraphIndex(){

    }

    //TODO: better use a move constructor
    template<typename T, class Leaves> BlockGraphIndex<T,Leaves>
    &BlockGraphIndex<T,Leaves>::setSaveKernels(string & fileName){
        this->kernelsOutFile = fileName;
        return *this;
    }

    template<typename T, class Leaves>  BlockGraphIndex<T,Leaves>::BlockGraphIndex(StrategySplitBlock *st,
                                                                                   uint gs):BlockGraph<T,Leaves>(st,gs){}

    template<typename T, class Leaves> uint  BlockGraphIndex<T,Leaves>::getHeader() const{
        uint wr = BLOCKGRAPH_INDEX_HDR;
        return wr;
    }

    template<typename T, class Leaves>  BlockGraphIndex<T,Leaves>::BlockGraphIndex():BlockGraph<T,Leaves>(){
        this->ell = 0;
    }


    template <typename T,class Leaves> void  BlockGraphIndex<T,Leaves>::saveKernels(vector<T> &kernels){
        ofstream out(kernelsOutFile);
        if (out.good()){
            saveValue<uint>(out,sizeof(size_t)*8);
            saveValue<uint>(out,sizeof(T));
            bool mayShift = (this->min_v==0);
            saveValue<bool>(out,mayShift);
            T shift =(T) ((this->min_v<=(T)1)?2:0);
            saveValue<T>(out,shift);
            saveValue<ulong>(out,(ulong)kernels.size());
            saveValue<T>(out,&kernels[0],kernels.size());
        }
        out.close();
    }

//    /**
//     * Simplified version: always add a separator between two kernels
//     */
//    template <typename T,class Leaves> void  BlockGraphIndex<T,Leaves>::indexLeavesSimplified(string &inputBuffer,
//                                                                                    uint level, const vector<bool> &boundaries){
//        vector<uint> leaves;
//        SequenceLZBLockHelper::loadAsUints<T>(inputBuffer,leaves);
////        kernels.reserve(leaves.size());
//        vector<T> kernels;
//        vector<ulong> jumpsFound;
//        ulong jumps = 0;
//        ulong disc = 0;
//        jumps =0;
//        ulong rank1 = 1;
//        T shiftSigma =(T) ((this->min_v<=(T)1)?2:0);
//        if (this->max_v+shiftSigma>=(1<<(sizeof(T)*8))){
//            cerr << "Warning: The minium value found in the input stream is " << this->min_v << ". However," << endl;
//            cerr << "we need symbols 0 and 1 free, since the 1 is to mark the block boundaries"<<endl;
//            cerr << "while the 0 is (currently) the $ symbol of FM-indexes. The process will continue" << endl;
//            cerr << "but no result is guaranteed." << endl;
//        }
//        auto iterLeaves = leaves.begin();
//        assert(this->blockLengthLevel[level]==this->blockLengthLevel[level-1]);
//        //copy the first block in the sequence of kernels
//        for (ulong i=0UL;i<this->blockLengthLevel[level];i++){
//            kernels.push_back((*iterLeaves)+shiftSigma);
//            ++iterLeaves;
//        }
//        //process each copied block (from the previous level to this -which is the bottom-)
//        for (ulong i = 1; i < this->marks[level - 1]->getLength(); i++) {
//            //if the block (in the previous level (note level-1) was not set to 1 --> we skip it since was not copied
//            if (!this->marks[level - 1]->access(i)) continue;
//            kernels.push_back((T)0);
//            //copy the block in the sequence of kernels
//            ulong offset = rank1*this->blockLengthLevel[level];
//            ulong limSup = min(offset+this->blockLengthLevel[level],leaves.size());
//            for (auto j=offset;j<limSup;j++,++iterLeaves)
//                kernels.push_back((*iterLeaves)+shiftSigma);
//
//            rank1++;
//        }
//        saveKernels(kernels);
//    }

    /**
     * Simplified version: always add a separator between two kernels
     */
    template <typename T,class Leaves> void  BlockGraphIndex<T,Leaves>::indexLeavesSimplified(string &inputBuffer,
                                                                                              uint level, const vector<bool> &boundaries){
        IfstreamBuffer<T> inBuffer(10000000,inputBuffer,0);
        ulong n = inBuffer.size();
        OutBuffer<T> out(10000000,kernelsOutFile);
        out.push_back(inBuffer[0]);
        inBuffer++;
        for (ulong i=1;i<n;i++){
            if (i%this->blockLengthLevel[level]==0){
                out.push_back(static_cast<T>(0));
            }
            out.push_back(inBuffer[i]);
            inBuffer++;
        }
//        out.close();
//        inBuffer.close();
//        vector<uint> leaves;
//        SequenceLZBLockHelper::loadAsUints<T>(inputBuffer,leaves);
////        kernels.reserve(leaves.size());
//        vector<T> kernels;
//        vector<ulong> jumpsFound;
//        ulong jumps = 0;
//        ulong disc = 0;
//        jumps =0;
//        ulong rank1 = 1;
//        T shiftSigma =(T) ((this->min_v<=(T)1)?2:0);
//        if (this->max_v+shiftSigma>=(1<<(sizeof(T)*8))){
//            cerr << "Warning: The minium value found in the input stream is " << this->min_v << ". However," << endl;
//            cerr << "we need symbols 0 and 1 free, since the 1 is to mark the block boundaries"<<endl;
//            cerr << "while the 0 is (currently) the $ symbol of FM-indexes. The process will continue" << endl;
//            cerr << "but no result is guaranteed." << endl;
//        }
//        auto iterLeaves = leaves.begin();
//        assert(this->blockLengthLevel[level]==this->blockLengthLevel[level-1]);
//        //copy the first block in the sequence of kernels
//        for (ulong i=0UL;i<this->blockLengthLevel[level];i++){
//            kernels.push_back((*iterLeaves)+shiftSigma);
//            ++iterLeaves;
//        }
//        //process each copied block (from the previous level to this -which is the bottom-)
//        for (ulong i = 1; i < this->marks[level - 1]->getLength(); i++) {
//            //if the block (in the previous level (note level-1) was not set to 1 --> we skip it since was not copied
//            if (!this->marks[level - 1]->access(i)) continue;
//            kernels.push_back((T)0);
//            //copy the block in the sequence of kernels
//            ulong offset = rank1*this->blockLengthLevel[level];
//            ulong limSup = min(offset+this->blockLengthLevel[level],leaves.size());
//            for (auto j=offset;j<limSup;j++,++iterLeaves)
//                kernels.push_back((*iterLeaves)+shiftSigma);
//
//            rank1++;
//        }
//        saveKernels(kernels);
    }

    /**
     * This is the more adequate implementation: checks whether we have to set a 0 between two kernels or not.
     */
    template <typename T,class Leaves> void  BlockGraphIndex<T,Leaves>::indexLeaves(string &inputBuffer,
                                                                                              uint level, const vector<bool> &boundaries){
        //TODO: we are calling the simplified version of this procedure. The more adequate one is this but it is not
        //actually running as it should.

        indexLeavesSimplified(inputBuffer,level,boundaries);
        return;
        vector<uint> leaves;
        SequenceLZBLockHelper::loadAsUints<T>(inputBuffer,leaves);
//        kernels.reserve(leaves.size());
        vector<T> kernels;
        vector<ulong> jumpsFound;
        ulong jumps = 0;
        ulong disc = 0;
        jumps =0;
        ulong rank1 = 1;
        T shiftSigma =(T) ((this->min_v<=(T)1)?2:0);
        if (this->max_v+shiftSigma>=(1<<(sizeof(T)*8))){
            cerr << "Warning: The minium value found in the input stream is " << this->min_v << ". However," << endl;
            cerr << "we need symbols 0 and 1 free, since the 1 is to mark the block boundaries"<<endl;
            cerr << "while the 0 is (currently) the $ symbol of FM-indexes. The process will continue" << endl;
            cerr << "but no result is guaranteed." << endl;
        }
        auto iterLeaves = leaves.begin();
        assert(this->blockLengthLevel[level]==this->blockLengthLevel[level-1]);
        //copy the first block in the sequence of kernels
        for (ulong i=0UL;i<this->blockLengthLevel[level];i++){
            kernels.push_back((*iterLeaves)+shiftSigma);
            ++iterLeaves;
        }
        //process each copied block (from the previous level to this -which is the bottom-)
        for (ulong i = 1; i < this->marks[level - 1]->getLength(); i++) {
            //if the block (in the previous level (note level-1) was not set to 1 --> we skip it since was not copied
            if (!this->marks[level - 1]->access(i)) continue;
            //if there is a boundarie at the end of that block we add it to the kernels sequence
            if (!(this->marks[level-1]->access(i - 1) && boundaries[i])){
                disc++;
#ifdef MYDEBUG
                assert(this->positions[rank1 * this->blockLengthLevel[level] - 1] + 1 != this->positions[rank1 * this->blockLengthLevel[level]]);
#endif
                kernels.push_back((T)0);
            }
            //copy the block in the sequence of kernels
            ulong offset = rank1*this->blockLengthLevel[level];
            ulong limSup = min(offset+this->blockLengthLevel[level],leaves.size());
            for (auto j=offset;j<limSup;j++,++iterLeaves)
                kernels.push_back((*iterLeaves)+shiftSigma);

            rank1++;
        }
#ifdef MYDEBUG
        for (ulong i=1;i<leaves.size();i++)
            if ((this->positions[i-1]+1)!=this->positions[i]) {
                jumps++;
            }

        cerr << "jumps: " << jumps << ", disc: " << disc <<  endl;
        assert(jumps==disc);
#endif
        saveKernels(kernels);

    }

    template <typename T, class Leaves> void  BlockGraphIndex<T,Leaves>::
    markBlockBoundaries(uint level, vector<bool> &marksPlain,vector<bool>&prevMarks){

        vector<bool> newMarks(marksPlain);
        if ((int)level>this->firstLevel) {

            ulong current = 1;
            ulong rank1   = 1;
            while (current < prevMarks.size()) {
                if (!prevMarks[current]) {
                    ulong pos = rank1 * this->blockLengthLevel[level - 1];
                    ulong block = pos / this->blockLengthLevel[level];
                    if (block<newMarks.size()) {
                        newMarks[block] = false;
//                        assert(this->positions[this->blockLengthLevel[level] * rank1 - 1]
//                               != this->positions[this->blockLengthLevel[level] * rank1]);
                    }
                }
                if (this->marks[level-1]->access(current)) rank1++;
                current++;
            }
            assert(newMarks.size()==marksPlain.size());
            vector<bool> newMarksForContinuers;//nMarks.size()
            newMarksForContinuers.push_back(true);
            for (ulong i=1;i<marksPlain.size();i++){
                newMarksForContinuers.push_back(marksPlain[i] && marksPlain[i-1] && newMarks[i]);
            }
            assert(newMarksForContinuers.size()==marksPlain.size());
            prevMarks = newMarksForContinuers;
        }else{
#ifdef MYDEBUG
            for (ulong i=1;i<marksPlain.size();i++){
                if (!marksPlain[i] && marksPlain[i-1]){
                    assert(this->positions[this->blockLengthLevel[level]*i-1]+1
                            !=this->positions[this->blockLengthLevel[level]*i-1]);
                }
            }
            #endif
            prevMarks = marksPlain;
        }

    }
    template<typename T, class Leaves> bool  BlockGraphIndex<T,Leaves>::testSegmentsIndex(uint level, vector<bool> &boundaries, ulong len) {
        if (this->positions.empty()) return true;
        ulong disc = 0;
        if (level + 1 == this->strategySplitBlocks->getNLevels()) {
            ulong rank1 = 1;
            for (ulong i = 1; i < this->marks[level - 1]->getLength(); i++) {
                if (this->marks[level-1]->access(i) && (!this->marks[level-1]->access(i - 1) || !boundaries[i])) {
                    assert(this->positions[rank1 * this->blockLengthLevel[level] - 1] + 1 != this->positions[rank1 * this->blockLengthLevel[level]]);
                    disc++;
                }
                if (this->marks[level - 1]->access(i))rank1++;
            }
        } else {

            uint rank1 = 1;
            for (ulong i = 1; i < this->marks[level]->getLength(); i++) {
                assert((!boundaries[i]) || (boundaries[i] && this->marks[level]->access(i)));
                if (this->marks[level]->access(i) && (!this->marks[level]->access(i - 1) || !boundaries[i])) {
                    assert(this->positions[rank1 * this->blockLengthLevel[level] - 1] + 1 != this->positions[rank1 * this->blockLengthLevel[level]]);
                    disc++;
                }
                if (this->marks[level]->access(i))rank1++;
            }
        }
        ulong njumps = 0;
        for (ulong i = 1; i < len; i++) {
            if (this->positions[i - 1] + 1 != this->positions[i]) {
                njumps++;
            }
        }

        cerr << "jumps: " << njumps << ", and detected: " << disc << endl;
        assert(njumps == disc);
        return true;
    }

    template<typename T, class Leaves> void  BlockGraphIndex<T,Leaves>::sortSuffixesReversedPhrases(uint level, IfstreamBuffer<T> &inBuffer,
                                                                                                    vector<long> &sa){
        ulong nMarks = this->marks[level]->getLength();
        vector<CellSort> reversedPhrases;
        reversedPhrases.reserve(this->marks[level]->rank1(nMarks-1));
        ulong rank1 = 0;
        for (auto i=0UL;i<nMarks; i++){
            if (this->marks[level]->access(i)){
                reversedPhrases.push_back(CellSort((i+1)*this->blockLengthLevel[level],
                                                   i*this->blockLengthLevel[level],rank1));
                rank1++;
            }
        }
        SparseSuffixSort<T,IfstreamBuffer<T>> sss(inBuffer);
        sss.sparseStringSort(reversedPhrases);
        sss.sparseSuffixSort(reversedPhrases, sa);
    }

    template<typename T, class Leaves> void  BlockGraphIndex<T,Leaves>::sortSuffixesPhraseBoundaries(uint level,
                                                                                                     IfstreamBuffer<T> &inBuffer, vector<long> &sa) {
        ulong nMarks = this->marks[level]->getLength();
        vector<CellSort> reversedPhrases;
        reversedPhrases.reserve(this->marks[level]->rank1(nMarks-1));
        ulong rank1 = 0;
        for (auto i=0UL;i<nMarks; i++){
            if (this->marks[level]->access(i)){
                reversedPhrases.push_back(CellSort((i+1)*this->blockLengthLevel[level],
                                                   i*this->blockLengthLevel[level],rank1));
                rank1++;
            }
        }
        SparseSuffixSort<T,IfstreamBuffer<T>> sss(inBuffer);
        sss.sparseStringSort(reversedPhrases);
        sss.sparseSuffixSort(reversedPhrases, sa);
    }

    template<typename T, class Leaves> void  BlockGraphIndex<T,Leaves>::sortSuffixes(uint level, IfstreamBuffer<T> &buffer){
        //sortSuffixesReversedPhrases
        //sortSuffixesPhraseBoundaries
    }

};

#endif