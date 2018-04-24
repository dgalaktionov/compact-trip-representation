//
// Created by alberto on 4/23/15.
//

#include <sequence/MultiArity/SequenceBuilderMultiArityWT.h>

namespace cds_static{


    SequenceBuilderMultiArityWT::SequenceBuilderMultiArityWT(vector<SequenceBuilder *> &seq_builder,
                                                             MultiArityCoder*mac, uint _cut):
            sb(seq_builder),coder(mac),cut(_cut){}

    SequenceBuilderMultiArityWT::~SequenceBuilderMultiArityWT() {
        for (auto it: sb)
            delete it;
    }

    Sequence * SequenceBuilderMultiArityWT::build(uint * seq, size_t len){
        Sequence *s = new MultiArityWT(seq,len,cut,coder,sb);
        return s;
    };

    Sequence * SequenceBuilderMultiArityWT::build(const Array & seq){
        throw runtime_error("Not implemented yet (SequenceBuilderMultiArityWT::build(const Array & seq)");
    };

};
