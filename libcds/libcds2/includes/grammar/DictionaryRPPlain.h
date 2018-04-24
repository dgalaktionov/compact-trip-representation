//
// Created by alberto on 8/12/15.
//

#ifndef ALL_DICTIONARYRPPLAIN_H
#define ALL_DICTIONARYRPPLAIN_H

#include <grammar/DictionaryRP.h>
#include <utils/GenericArray.h>
using namespace cds_utils;

namespace cds_static {
    class DictionaryRPPlain : public DictionaryRP{
    public:
        DictionaryRPPlain(uint *LEFT, uint *RIGHT, uint alph, int R, uint *&);
        virtual ~DictionaryRPPlain();
        virtual uint getRight(size_t rule);
        virtual uint getLeft(size_t rule);
        virtual size_t getSize();
        virtual size_t getSizeVerbose();
        virtual void save(ofstream &of);
        static DictionaryRPPlain *load(ifstream &in);
    protected:
        DictionaryRPPlain();
        GenericArray<uint> left;
        GenericArray<uint> right;
    };
};



#endif //ALL_DICTIONARYRPPLAIN_H
