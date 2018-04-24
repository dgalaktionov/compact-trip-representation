//
// Created by alberto on 4/21/15.
//

#ifndef ALL_MULTIARITYCANONICALHUFFMAN_H
#define ALL_MULTIARITYCANONICALHUFFMAN_H

#include<sequence/MultiArity/MultiArityCoder.h>

namespace cds_static {
    class MultiArityCanonincalHuffman: public MultiArityCoder {
    public:
        //max_length: max length of a code (the same for all symbols)
        //chunk_length: chunk length (it is a multiary coder).
        MultiArityCanonincalHuffman(const vector<uint> &input, uint _arity);
        MultiArityCanonincalHuffman(const uint *input, const size_t len, uint _arity);
        virtual ~MultiArityCanonincalHuffman();
        virtual uint get_chunk(const uint symbol,const  uint level)const;
        virtual uint get_max_level(const uint symbols)const;
        virtual size_t getSize()const;
        virtual void save(ofstream &fp)const;
        static MultiArityCanonincalHuffman *load(ifstream &fp);

    protected:
        MultiArityCanonincalHuffman();
        void InternalConstructor(const vector<uint> &input);
        //codes stores the pair (code,length). We are assuming
        //codes[i] is associated with symbol i.
        vector<pair<uint, uint>> codes;
        uint arity;
        uint bits_arity;
        uint mask_arity;
    };

};
#endif //ALL_MULTIARITYCANONICALHUFFMAN_H
