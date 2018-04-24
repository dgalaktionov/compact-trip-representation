//
// Created by alberto on 4/14/15.
//

#ifndef ALL_MULTIARITYWTHELPER_H
#define ALL_MULTIARITYWTHELPER_H
#include <sequence/MultiArity/MultiArityWT.h>

namespace  cds_static {

    class MultiArityWT;
    class MultiArityWTHelper {
    public:
        MultiArityWTHelper();
        uint getDepth(const MultiArityWT &wt);
        const Sequence *getSequence(const MultiArityWT &wt, const vector<uint> &node_id);
        const vector<MultiArityNode *>* getChildren(const MultiArityWT &wt, const vector<uint> &node_id);
        void GetLeavesPerLevel(const MultiArityWT &wt, vector<uint> &leaves);
    protected:
        uint getDepthRec(MultiArityNode *node);
        void GetLeavesPerLevelRec(MultiArityNode *node, vector<uint> &leaves, uint level);
    };
};


#endif //ALL_MULTIARITYWTHELPER_H
