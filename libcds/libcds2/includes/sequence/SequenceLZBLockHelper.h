
#ifndef  SEQUENCE_LZ_BLOCK_HELPER_H_
#define SEQUENCE_LZ_BLOCK_HELPER_H_
#include <utils/IfstreamBuffer.h>
#include <utils/OutBuffer.h>
#include <bits/stl_bvector.h>

namespace cds_static {

    class SequenceLZBLockHelper {

    public:
        /**
        * Maps a sequence of chars to a sequence of continuous integers (int [0,max_v]).
        */
        template<typename T> static long mapLargeSequence(string &infile, string &outfile, ulong &len, T &max_v) {

            long bufferSize = 10000000;
            long blockLength = 0;
            vector<T> counters(256,0);
            IfstreamBuffer<char> it(bufferSize,infile,blockLength);
            len = 0;
            while(!it.end()){
                if ((ulong)*it>counters.capacity()){
                    auto prevSize = counters.capacity();
                    counters.reserve(*it+1);
                    for (ulong i=prevSize;i<counters.capacity();i++)
                        counters[i]=0;
                }
                counters[*it]++;
                ++it;
                len++;
                if (len==1000000)
                    cerr << endl;
            }
            uint code=0;
            for (auto &x:counters){
                if (x){
                    x=code++;
                    max_v=x;
                }
            }
            it.restart();
            OutBuffer<T> out(bufferSize,outfile);
            while(!it.end()){
                out.push_back(counters[*it]);
                ++it;
            }
            return 0;
        }

        /**
        * Copy the content of infile into outfile
        */
        template<typename T> static long copyFile(string &infile, string &outfile) {

            ifstream in(infile);
            ofstream out(outfile,ios_base::binary);
            out << in.rdbuf();
            in.close();
            out.close();
           return 0;
        }

   template<typename T> static bool checkBlocks(T*input, size_t ini1, size_t ini2, size_t len){
        for (size_t i=0;i<len;i++){
            if (input[ini1+i]!=input[ini2+i])
                return false;
        }
        return true;
    }

    template <typename T> static bool checkBlocks(IfstreamBuffer<T> &input, size_t ini1, size_t ini2, size_t len){
        for (size_t i=0;i<len;i++){
            if (input[ini1+i]!=input[ini2+i])
                return false;
        }
        return true;
    }

        template<typename T> static ulong loadAsUints(string &buffer,vector<uint> &res){
            ulong n;

            //we assume the 2 times the size of the  input fits in memory--> otherwise we must use a buffer o size <n
            //the input must fit in memory at anycase since we have to build a WT on it
            AllInMemoryBuffer<T> it(buffer);
            n = it.size();
            res.reserve(n);
            //copy the block in the list of verbatim blocks
            for (ulong i=0;i<n;i++){
                res.push_back((uint)(it[i]));
            }
            return n;
        }

//    static bool testParsing(SequenceLZBlock &bg, vector<uint> &sequence_test, uint level){
//
//        if (level<bg.firstLevel) return true;
//        if (level+1==bg.nLevels) return true;
//
//        auto nblocks = bg.marks[level]->getLength();
//        for (uint i=0;i<nblocks;i++){
//            if (bg.marks[level]->access(i)) continue;
//            size_t block_index, block_pos, unmarked_index;
//            bg.getPointerBack(i, unmarked_index, block_index, block_pos, level);
//
//            auto block_end_pos = (block_pos+1)*bg.blockLengthLevel[level];
//            auto post_back_from_end = block_end_pos - (*bg.offsetsBack[level])[unmarked_index]-1;
//            for (size_t j=0;j<bg.blockLengthLevel[level];j++){
//                // assert(input[post_back_from_end+j]==input[i*blockLengthLevel[level]+j]);
//                if (bg.sequence_test[post_back_from_end+j]!=bg.sequence_test[i*bg.blockLengthLevel[level]+j]){
//                    assert((bg.sequence_test[post_back_from_end+j]==bg.sequence_test[i*bg.blockLengthLevel[level]+j]));
//                    cerr << endl;
//                }
//            }
//        }
//        return true;
//    }
//
//
//    static bool testRanks(SequenceLZBlock &bg, vector<uint> &input, uint level) {
//
//        SequenceBuilderWaveletMatrix sb(new BitSequenceBuilderRG(32),new MapperNone());
//        size_t n = input.size();
//        Sequence *seqTest = bg.sb.build(&input[0],n);
//        ulong nBlocks = bg.n / bg.blockLengthLevel[level];
//        for (ulong i=0;i<nBlocks;i++){
//            size_t pos = i*bg.blockLengthLevel[level]-1;
//            for (uint j=0;j<=bg.max_v;j++){
//                if (bg.rank(j,pos)!=bg.accessRanksBlocks(level, j, i)){
//                    cerr << endl;
//                }
//                assert(seqTest->rank(j,pos)== bg.accessRanksBlocks(level, j, i));
//            }
//        }
//
//        for (ulong i=0;i<nBlocks;i++){
//            size_t pos = i*bg.blockLengthLevel[level]-1;
//            if (bg.marks[level]->access(i)) continue;
//            for (uint j=0;j<=bg.max_v;j++){
//                size_t rb = bg.accessRanksBlocks(level,j,i);//rank at the beginning of the original block (marked or unmarked)
//                //assert((seq_tests==nullptr) || seq_tests->rank(symbol,ini_block_index*blockLengthLevel[level]));
//                //by beginning we mean ini_block_index*blockLength-1
//                size_t new_pos, block_ini, block_end_pos, block_pos, unmarked_index, offset;
//                size_t block_index_of_1s;
//
//                bg.getPointerBack(i, unmarked_index, block_index_of_1s, block_pos, level);
//                block_end_pos = (block_pos+1)*bg.blockLengthLevel[level];//the ending position of the marked block that is pointed by the current block
//                size_t new_block_index = block_pos;//the marked block index
//                block_ini  = block_end_pos - (*bg.offsetsBack[level])[unmarked_index]-1;//the starting position in the pointed block (is the position from the beginning)
//                offset = pos % bg.blockLengthLevel[level];//the offset inside the marked block
//                new_pos= block_ini + offset; //we add the offset of "inside the marked block" to the ini_pos in the marked block
//                size_t rank_pointed  = seqTest->rank(j,block_ini-1);
//                size_t rank_obtained =  bg.accessRanksBlocks(level,j,block_pos) + bg.accessRanksOffsets(level, j, 2*unmarked_index);
//                assert(rank_pointed==rank_obtained);
//            }
//        }
//
//        delete seqTest;
//        return true;
//    }


    };
};
#endif