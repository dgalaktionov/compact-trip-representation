//
// Created by alberto on 8/28/15.
//

#include <bitsequence/BitSequenceBuilderBG.h>
#include <bitsequence/BitSequenceBG.h>

namespace cds_static {

    BitSequenceBuilderBG::BitSequenceBuilderBG(uint _nLevels, uint _blockLengthAtLeaves, uint _arity,
        string tmpBuff1, string tmpBuff2):nLevels(_nLevels),blockLengthAtLeaves(_blockLengthAtLeaves),arity(_arity),
        TemporalBuffer1(tmpBuff1),TemporalBuffer2(tmpBuff2){

    }

    BitSequenceBuilderBG &BitSequenceBuilderBG::setLoggerFile(string &logger_file){
        this->loggerFile = logger_file;
        return *this;
    }

    BitSequence *BitSequenceBuilderBG::build(uint *bitseq, size_t len) const {
        return new BitSequenceBG(bitseq,len,nLevels,blockLengthAtLeaves,arity,TemporalBuffer1,TemporalBuffer2);
    }

    BitSequence *BitSequenceBuilderBG::build(const BitString &bs) const {
        return nullptr;
    }

}