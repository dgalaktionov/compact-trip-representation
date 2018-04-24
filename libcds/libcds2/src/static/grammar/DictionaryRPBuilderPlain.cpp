//
// Created by alberto on 8/12/15.
//

#include <grammar/DictionaryRPBuilderPlain.h>

namespace cds_static {

    DictionaryRPBuilderPlain::DictionaryRPBuilderPlain() {

    }

    DictionaryRPBuilderPlain::~DictionaryRPBuilderPlain() {

    }

    DictionaryRP *DictionaryRPBuilderPlain::build(uint *left, uint *right, int R, uint alpha, uint *&perm) {
        return new DictionaryRPPlain(left,right,R,alpha,perm);
    }
}