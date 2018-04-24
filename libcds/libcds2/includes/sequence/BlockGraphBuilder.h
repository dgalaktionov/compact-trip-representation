//
// Created by alberto on 4/8/15.
//
#include <sequence/BlockGraph.h>


#ifndef _ALL_SEQUENCEBUILDERSEQUENCEBLOCKGRAPH_H_
#define _ALL_SEQUENCEBUILDERSEQUENCEBLOCKGRAPH_H_

namespace cds_static {

    class SequenceBlockGraphBuilder : public SequenceBuilder{
    public:

        SequenceBlockGraphBuilder(uint nLevels, uint blockLengthAtLeaves,uint arity);
        SequenceBlockGraphBuilder& setSequenceBuilder(SequenceBuilder *sb);
        virtual SequenceBlockGraphBuilder& setMarksBuilder(BitSequenceBuilder *_bsb);
        /**
        * @param outStream: stream to write out the data structure. It must be opened if it is
        not empty. The data structure is written level by level, freeing the space the data
        structures take after written them.
        */
        virtual SequenceBlockGraphBuilder& setOutStream(string &_outFile);

        virtual SequenceBlockGraphBuilder& setMaxV(uint _maxv);

        virtual SequenceBlockGraphBuilder& setGroupSize(uint _gs);

        virtual SequenceBlockGraphBuilder& setBufferInputReadSize(ulong _inputBufferSize);

        virtual SequenceBlockGraphBuilder& setBufferOutputReadSize(ulong _outputBufferSize);

        virtual SequenceBlockGraphBuilder& setBufferInName(string && _tmpFileNameBufferIn);

        virtual SequenceBlockGraphBuilder& setBufferOutName(string &&_tmpFileNameBufferOut);

        virtual SequenceBlockGraphBuilder& setLoggerFile(string &logger_file);
        virtual ~SequenceBlockGraphBuilder();

        virtual Sequence *build(uint *seq, size_t len);
        virtual Sequence *build(const Array &tmp);
        virtual Sequence *build(string &file);

    protected:

        SequenceBuilder *seq_builder;
        uint gs;
        string outFile;
        uint maxV;
        ulong inputBufferSize;
        ulong outputBufferSize;
        string tmpFileNameBufferIn;
        string tmpFileNameBufferOut;
        string loggerFile;
        uint nLevels;
        uint blockLengthAtLeaves;
        uint arity;
        BitSequenceBuilder *bsb;
    };
};
#endif //_ALL_SEQUENCEBUILDERSEQUENCEBLOCKGRAPH_H_
