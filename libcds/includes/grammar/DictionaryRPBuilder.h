//
// Created by alberto on 8/12/15.
//

#ifndef ALL_DICTIONARYRPBUILDER_H
#define ALL_DICTIONARYRPBUILDER_H

#include <grammar/DictionaryRP.h>

namespace cds_static {

    class DictionaryRPBuilder {
    public:
        virtual ~DictionaryRPBuilder(){}
        virtual DictionaryRP * build(uint *left, uint *right, int R, uint alpha, uint *&perm)=0;
    };
};

#include <grammar/DictionaryRPBuilderPlain.h>
#include <grammar/DictionaryRPBuilderTabei.h>
#endif //ALL_DICTIONARYRPBUILDER_H
