//
// Created by alberto on 8/18/15.
//

#include <sequence/BlockGraphBuilder.h>

namespace cds_static {

    SequenceBlockGraphBuilder::SequenceBlockGraphBuilder(uint nLevels, uint blockLengthAtLeaves, uint arity) {
        this->nLevels = nLevels;
        this->blockLengthAtLeaves = blockLengthAtLeaves;
        this->arity = arity;
        gs = 2;
        maxV = DEFAULT_MAX_SIGMA;
        inputBufferSize = DEFAULT_BUFFER_READ_SIZE;
        outputBufferSize = DEFAULT_BUFFER_WRITE_SIZE;
        bsb = nullptr;
        this->seq_builder = nullptr;
    }

    SequenceBlockGraphBuilder &SequenceBlockGraphBuilder::setMarksBuilder(BitSequenceBuilder *_bsb) {
        this->bsb = _bsb;
        return *this;
    }

    /**
        * @param outStream: stream to write out the data structure. It must be opened if it is
        not empty. The data structure is written level by level, freeing the space the data
        structures take after written them.
        */
    SequenceBlockGraphBuilder &SequenceBlockGraphBuilder::setOutStream(string &_outFile) {
        this->outFile = _outFile;
        return *this;
    }

    SequenceBlockGraphBuilder &SequenceBlockGraphBuilder::setMaxV(uint _maxv) {
        this->maxV = _maxv;
        return *this;
    }

    SequenceBlockGraphBuilder &SequenceBlockGraphBuilder::setGroupSize(uint _gs) {
        this->gs = _gs;
        return *this;
    }

    SequenceBlockGraphBuilder &SequenceBlockGraphBuilder::setBufferInputReadSize(ulong _inputBufferSize) {
        this->inputBufferSize = _inputBufferSize;
        return *this;
    }

    SequenceBlockGraphBuilder &SequenceBlockGraphBuilder::setBufferOutputReadSize(ulong _outputBufferSize) {
        this->outputBufferSize = _outputBufferSize;
        return *this;
    }

    SequenceBlockGraphBuilder &SequenceBlockGraphBuilder::setBufferInName(string &&_tmpFileNameBufferIn) {
        this->tmpFileNameBufferIn = std::move(_tmpFileNameBufferIn);
        return *this;
    }

    SequenceBlockGraphBuilder &SequenceBlockGraphBuilder::setBufferOutName(string &&_tmpFileNameBufferOut) {
        this->tmpFileNameBufferOut = std::move(_tmpFileNameBufferOut);
        return *this;
    }

    SequenceBlockGraphBuilder &SequenceBlockGraphBuilder::setLoggerFile(string &logger_file){
        this->loggerFile = logger_file;
        return *this;
    }
    SequenceBlockGraphBuilder::~SequenceBlockGraphBuilder() {
    }

    SequenceBlockGraphBuilder &SequenceBlockGraphBuilder::setSequenceBuilder(SequenceBuilder *sb) {
        this->seq_builder = sb;
        return *this;
    }

    Sequence *SequenceBlockGraphBuilder::build(uint *seq, size_t len) {

        StrategySplitBlock *st = new StrategySplitBlockBottomUpWithArity(nLevels, blockLengthAtLeaves,arity);
        SequenceBlockGraph<uint, Sequence>*bg = new SequenceBlockGraph<uint, Sequence>(st,gs);
        bg->setBufferInName(tmpFileNameBufferIn);
        bg->setBufferOutName(tmpFileNameBufferOut);
        bg->setBufferInputReadSize(inputBufferSize);
        bg->setBufferOutputReadSize(outputBufferSize);
        bg->setMaxV(maxV);
        bg->setSequenceBuilder(seq_builder);
        bg->setOutStream(outFile);
        bg->setLoggFile(loggerFile);

        string tmpFile;
        if (tmpFileNameBufferIn.empty()){
            tmpFile = GetTemporalFileName();
        }else{
            tmpFile = tmpFileNameBufferIn;
        }
        ofstream fp(tmpFile);
        fp.write((char *) seq, sizeof(uint) * len);
        fp.close();

        bg->build(tmpFile,false);
        return bg;
    }

    Sequence *SequenceBlockGraphBuilder::build(const Array &tmp) {
        return nullptr;
    }

    Sequence *SequenceBlockGraphBuilder::build(string &file) {
        StrategySplitBlock *st = new StrategySplitBlockBottomUpWithArity(nLevels, blockLengthAtLeaves,arity);
        //create the block graph (does not run the construction algorithm)
        SequenceBlockGraph<uint, Sequence>*bg = new SequenceBlockGraph<uint, Sequence>(st,gs);
        bg->setBufferInName(tmpFileNameBufferIn);
        bg->setBufferOutName(tmpFileNameBufferOut);
        bg->setBufferInputReadSize(inputBufferSize);
        bg->setBufferOutputReadSize(outputBufferSize);
        bg->setMaxV(maxV);
        bg->setSequenceBuilder(seq_builder);
        //bg->setOutStream(outFile);
        bg->build(file,false);
        return bg;
//        return new SequenceBlockGraph<uint, Sequence>(file, st, bsb, outFile, maxV, inputBufferSize,
//                                                      outputBufferSize, tmpFileNameBufferIn, tmpFileNameBufferOut, gs,
//                                                      seq_builder);

    }
}
