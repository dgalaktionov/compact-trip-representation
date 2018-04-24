//
// Created by alberto on 8/12/15.
//

#include <grammar/DictionaryRPBuilderTabei.h>

namespace cds_static{

    DictionaryRPBuilderTabei::DictionaryRPBuilderTabei() {

    }

    DictionaryRPBuilderTabei::~DictionaryRPBuilderTabei() {

    }

    DictionaryRP *DictionaryRPBuilderTabei::build(uint *left, uint *right, int R, uint alpha, uint *&perm) {
        return new DictionaryRPTabei(left,right,R,alpha,perm);
    }
}