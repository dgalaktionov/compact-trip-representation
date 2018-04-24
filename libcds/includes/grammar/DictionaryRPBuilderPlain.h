//
// Created by alberto on 8/12/15.
//

#ifndef ALL_DICTIONARYRPBUILDERPLAIN_H
#define ALL_DICTIONARYRPBUILDERPLAIN_H

#include <grammar/DictionaryRPBuilder.h>

namespace cds_static {

    class DictionaryRPBuilderPlain : public DictionaryRPBuilder {

    public:
        DictionaryRPBuilderPlain();

        virtual ~DictionaryRPBuilderPlain();

        virtual DictionaryRP *build(uint *left, uint *right, int R, uint alpha, uint *&perm);
    };

};


#endif //ALL_DICTIONARYRPBUILDERPLAIN_H
