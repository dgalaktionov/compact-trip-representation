
#include <sequence/SequenceBuilderWTRPHeur.h>

namespace cds_static {
    SequenceBuilderWTRPHeur::SequenceBuilderWTRPHeur(BitSequenceBuilder *_bsb, Mapper *_mp, uint _limit)
        :bsb(_bsb),mp(_mp),limit(_limit){}

    SequenceBuilderWTRPHeur::~SequenceBuilderWTRPHeur() {
        delete bsb;
        delete mp;
    }

    Sequence *SequenceBuilderWTRPHeur::build(uint *seq, size_t len) {
        return new SequenceWTRP(seq,len,limit,bsb,mp,false);
    }

    Sequence *SequenceBuilderWTRPHeur::build(const Array &seq) {
        return nullptr;
    }
};