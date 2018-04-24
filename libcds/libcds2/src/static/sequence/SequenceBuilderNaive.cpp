//
// Created by alberto on 11/5/15.
//

#include <sequence/SequenceBuilderNaive.h>
#include <sequence/SequenceNaive.h>

namespace cds_static {

    SequenceBuilderNaive::SequenceBuilderNaive(){

    }

    SequenceBuilderNaive::~SequenceBuilderNaive(){

    }

    Sequence *SequenceBuilderNaive::build(uint *seq, size_t len){
        return new SequenceNaive<ArrayDA>(seq,len);
    }

    Sequence *SequenceBuilderNaive::build(const Array &seq){
        return new SequenceNaive<ArrayDA>(seq);
    }
}