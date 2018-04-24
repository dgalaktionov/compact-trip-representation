//
// Created by alberto on 4/23/15.
//

#ifndef ALL_SEQUENCEBUILDERMULTIARITYWT_H
#define ALL_SEQUENCEBUILDERMULTIARITYWT_H

#include <sequence/SequenceBuilder.h>
#include <sequence/MultiArity/MultiArityCoder.h>
#include <sequence/Sequence.h>
#include <sequence/MultiArity/MultiArityWT.h>

namespace cds_static {
    class SequenceBuilderMultiArityWT : public SequenceBuilder {

    public:
        SequenceBuilderMultiArityWT(vector<SequenceBuilder *> &seq_builder, MultiArityCoder*mac, uint cut=UINT32_MAX);
        virtual ~SequenceBuilderMultiArityWT();
        virtual Sequence * build(uint * seq, size_t len);
        virtual Sequence * build(const Array & seq);
    protected:
        vector<SequenceBuilder *> sb;
        MultiArityCoder *coder;
        uint cut;
        SequenceBuilder *sb_bottom;

    };
};


#endif //ALL_SEQUENCEBUILDERMULTIARITYWT_H
