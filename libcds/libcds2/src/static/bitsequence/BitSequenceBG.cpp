//
// Created by alberto on 8/28/15.
//

#include <bitsequence/BitSequenceBG.h>

namespace cds_static{

    BitSequenceBG::BitSequenceBG(uint *bitarray, size_t n, uint nLevels, uint blockLengthatLeaves, uint arity,
        const string &buffer1, const string &buffer2, const string &logger_file) {

        length = n;
        ones = 0UL;
        StrategySplitBlock *st = new StrategySplitBlockBottomUpWithArity(nLevels, blockLengthatLeaves,arity);
        bg = new SequenceBlockGraph<uint, Sequence>(st,2);
        bg->setMaxV(2);
        bg->setLoggFile(logger_file);
        bg->setSequenceBuilder(new SequenceBuilderNaive());
        uint *seq = new uint[n];
        for (size_t i=0;i<n;i++){
            if (bitget(bitarray,i)){
                seq[i]=1;
                ones++;
            }else{
                seq[i]=0;
            }
        }

        if (!buffer1.empty()){
            bg->setBufferInName(buffer1);
            if (!buffer2.empty()){
                bg->setBufferOutName(buffer2);
            }
        }
        string tmpForInput;
        if (!buffer1.empty()){
            stringstream ss;
            ss << buffer1 << rand();
            tmpForInput = ss.str();
        }

        string input_file;
        if (buffer1.empty()){
            input_file = GetTemporalFileName(tmpForInput);
        }else{
            input_file = buffer1;
        }
        ofstream fp(input_file);
        fp.write((char *) seq, sizeof(uint) * n);
        fp.close();

        bg->build(input_file,false);
        delete [] seq;

    }

    BitSequenceBG::BitSequenceBG() {
        length = 0UL;
    }

    BitSequenceBG::~BitSequenceBG() {
        delete bg;
    }

    bool BitSequenceBG::access(const size_t i) const {
        return (bg->access(i)==1);
    }

    size_t BitSequenceBG::rank1(const size_t i) const {
        return bg->rank(1,i);
    }

    size_t BitSequenceBG::select0(size_t i) const {
        return bg->select(0,i);
    }

    size_t BitSequenceBG::select1(size_t i) const {
        return bg->select(1,i);
    }

    size_t BitSequenceBG::getSize() const {
        return bg->getSize();
    }

    void BitSequenceBG::save(ofstream &f) const {
        uint wr = BG_HDR;
        saveValue(f,wr);
        bg->save(f);
    }

    BitSequenceBG *BitSequenceBG::load(ifstream &f) {
        uint wr;
        wr = loadValue<uint>(f);
        if (wr!=BG_HDR){
            return nullptr;
        }
        BitSequenceBG *ret = new BitSequenceBG();
        ret->bg =  SequenceBlockGraph<uint,Sequence>::load(f);
        ret->length = ret->bg->getLength();
        return ret;
    }

}
