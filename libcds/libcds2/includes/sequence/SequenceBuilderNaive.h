//
// Created by alberto on 11/5/15.
//

#ifndef ALL_SEQUENCEBUILDERNAIVE_H
#define ALL_SEQUENCEBUILDERNAIVE_H

#include <utils/libcdsBasics.h>
#include <sequence/SequenceBuilder.h>


namespace cds_static
{
    class SequenceBuilderNaive : public SequenceBuilder
    {
    public:
        SequenceBuilderNaive();
        virtual ~SequenceBuilderNaive();
        virtual Sequence * build(uint * seq, size_t len);
        virtual Sequence * build(const Array & seq);
    };
};
#endif //ALL_SEQUENCEBUILDERNAIVE_H
