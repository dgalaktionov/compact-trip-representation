//
// Created by alberto on 4/9/15.
//

#ifndef _ALL_MULTIARYWT_H
#define _ALL_MULTIARYWT_H
#include <sequence/MultiArity/MultiArityNode.h>
#include <sequence/MultiArity/MultiArityCoder.h>
#include <sequence/Sequence.h>

using namespace std;

namespace cds_static
{

    class MultiArityWT: public Sequence{

    friend class MultiArityWTHelper;

    public:
        //Builds a multi ary WT using the coder for the symbols. Besides, we need to provide a parameter cut,
        //which means that those sequences beneath the level "cut" are represented with seq_builder_bottom, while the
        //rest are represented with seq_builder
        MultiArityWT(uint *seq, size_t len, uint cut, MultiArityCoder *coder, vector<SequenceBuilder *> &seq_builder);
        virtual ~MultiArityWT();
        virtual size_t rank(uint c, size_t i) const;
        virtual size_t select(uint c, size_t i) const;
        virtual uint access(size_t i) const;
        virtual uint access(size_t i, size_t & r) const;
        virtual size_t getSize() const;
        virtual size_t getLength() const { return length; }
        virtual void save(ofstream & fp) const;
        static Sequence * load(ifstream & fp);
    protected:
        wt_coder *wtc;
        MultiArityNode *root;
        MultiArityCoder *coder;
        MultiArityWT();
        ulong tmp_space;
    };

};

#endif //_ALL_MultiAryWT<T>_H_
