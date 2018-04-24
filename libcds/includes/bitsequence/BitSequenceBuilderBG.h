//
// Created by alberto on 8/28/15.
//

#ifndef _BIT_SEQUENCE_BUILDER_BG_H_
#define _BIT_SEQUENCE_BUILDER_BG_H_

#include <bitsequence/BitSequenceBuilder.h>

namespace cds_static {

    class BitSequenceBuilderBG : public BitSequenceBuilder {

    public:
        BitSequenceBuilderBG(uint nLevels, uint blockLengthAtLeaves, uint arity, string temporalBuffer1="",string temporalBuffer2="");
        BitSequenceBuilderBG& setLoggerFile(string &logger_file);

        virtual ~BitSequenceBuilderBG() { }

        virtual BitSequence *build(uint *bitseq, size_t len) const;

        virtual BitSequence *build(const BitString &bs) const;

    protected:
        uint nLevels;
        uint blockLengthAtLeaves;
        uint arity;
        string TemporalBuffer1;
        string TemporalBuffer2;
        string loggerFile;
    };

};

#endif
