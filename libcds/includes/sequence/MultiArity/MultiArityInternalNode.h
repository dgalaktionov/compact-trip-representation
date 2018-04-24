//
// Created by alberto on 4/14/15.
//

#ifndef ALL_MULTIARITYINTERNALNODE_H
#define ALL_MULTIARITYINTERNALNODE_H
#include <sequence/Sequence.h>

#include <sequence/MultiArity/MultiArityNode.h>
#include <vector>
#include <sequence/MultiArity/MultiArityWTHelper.h>

namespace cds_static
{
class MultiArityInternalNode : public MultiArityNode{
    friend class MultiArityWTHelper;
    //enum multi_ary_node_type {leaf_node=0,internal_node=1};
    public:
        MultiArityInternalNode(uint *_seq, size_t len, uint level, uint cut, MultiArityCoder *coder,
                               vector<SequenceBuilder *> &seq_builder,
                                vector<vector<uint>> & symbols_per_level);
        virtual ~MultiArityInternalNode();
        virtual size_t rank(uint c, size_t i,uint level, MultiArityCoder *coder) const;
        virtual size_t select(uint c, size_t i, uint level, MultiArityCoder *coder) const;
        virtual uint access(size_t i) const;
        virtual uint access(size_t i, size_t & r) const;
        virtual size_t getSize() const;
        virtual void save(ofstream & fp) const;
        static MultiArityInternalNode * load(ifstream & fp);
protected:
    MultiArityInternalNode(){}
    Sequence *seq;
    vector<MultiArityNode*> children;
    };
};


#endif //ALL_MULTIARITYINTERNALNODE_H
