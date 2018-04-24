//
// Created by alberto on 8/12/15.
//

#ifndef ALL_DICTIONARYRPBUILDERTABEI_H
#define ALL_DICTIONARYRPBUILDERTABEI_H
#include <grammar/DictionaryRPBuilder.h>

namespace cds_static {
    class DictionaryRPBuilderTabei : public DictionaryRPBuilder{

    public:
        DictionaryRPBuilderTabei();

        virtual ~DictionaryRPBuilderTabei();

        virtual DictionaryRP *build(uint *left, uint *right, int R, uint alpha, uint *&perm);
    };

};


#endif //ALL_DICTIONARYRPBUILDERTABEI_H
