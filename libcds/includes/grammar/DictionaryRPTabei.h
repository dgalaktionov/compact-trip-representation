//
// Created by alberto on 8/12/15.
//

#ifndef ALL_DICTIONARYRPTABEI_H
#define ALL_DICTIONARYRPTABEI_H


#include <utils/TreePointers.h>
#include <bitsequence/BitSequence.h>
#include <grammar/DictionaryRP.h>

namespace cds_static {

    class PermutationBuilder;
    class Permutation;

    class DictionaryRPTabei : public DictionaryRP {
    public:
        DictionaryRPTabei(uint *LEFT, uint *RIGHT, uint alph, int R, uint *&PERM);
        DictionaryRPTabei(uint *LEFT, uint *RIGHT, uint alph, int R, uint *&PERM,PermutationBuilder *pmb);
        DictionaryRPTabei(Repair *rp,uint *&PERM,PermutationBuilder *pmb);
        ~DictionaryRPTabei();
        virtual uint getRight(size_t rule);
        virtual uint getLeft(size_t rule);
        virtual size_t getSize();
        virtual size_t getSizeVerbose();
        virtual void save(ofstream &of);
        static DictionaryRPTabei *load(ifstream &in);
    protected:
        DictionaryRPTabei();
        void buildTrees(uint *LEFT, uint *RIGHT, uint ALPH, int R, uint *&PERM, PermutationBuilder *pmb);
        void printTree(TreePointers<int> *t);
        void getLOUDS(TreePointers<int> **queue,uint &head,uint &pq, uint &p, uint *louds);
        void getPERM(TreePointers<int> *t, uint &p, uint *perm);
        size_t getParentLOUDS(uint r, BitSequence *louds);
        void deleteStructures(TreePointers<int> *t);
        BitSequence *LT_LOUDS;
        BitSequence *RT_LOUDS;
        uint invertUnary(uint val, uint ini, uint len);
        Permutation *permutation;
        uint *PERM_tmp;
        uint *INVPERM_tmp;
        uint PERM_SIZE;
        uint RSIZE;
    };

};


#endif //ALL_DICTIONARYRPTABEI_H
