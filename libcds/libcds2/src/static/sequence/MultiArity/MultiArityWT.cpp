//
// Created by alberto on 4/9/15.
//

#include <sequence/MultiArity/MultiArityWT.h>

namespace cds_static {

    MultiArityWT::MultiArityWT() : Sequence(0UL) {}

    MultiArityWT::~MultiArityWT() {
        if (root)
            delete root;
        delete coder;
    }

    MultiArityWT::MultiArityWT(uint *seq, size_t len, uint cut, MultiArityCoder *_coder, vector<SequenceBuilder *> &seq_builder) : Sequence(len) {
        if (len == 0) return;
        bool all_equal = true;
        for (size_t i = 1; i < len; i++) {
            if (seq[i] != seq[i - 1]) {
                all_equal = false;
                break;
            }
        }
        this->coder = _coder;
        vector<vector<uint>> symbols_per_level;
        if (all_equal)
            this->root = new MultiArityLeafNode(seq[0]);
        else {
            this->root = new MultiArityInternalNode(seq,len,0U,cut,coder,seq_builder,symbols_per_level);

            tmp_space =0;
            for (auto &it: symbols_per_level) {
                Sequence *s;
                buildBestSequence(seq_builder,&it[0],it.size(),s);
                tmp_space  += s->getSize();
              //  cerr << "Size: " << s->getSize() * 8.0 / s->getLength() << endl;
                delete s;
            }
            //cerr << "Total: " << tmp_space *8.0/length << endl;
        }
    }

    size_t MultiArityWT::rank(uint c, size_t i) const {
        return root->rank(c,i,0U,coder);
    }

    size_t MultiArityWT::select(uint c, size_t i) const {
        return root->select(c,i,0,coder)-1;
    }

    uint MultiArityWT::access(size_t i) const {
        return root->access(i);
    }

    uint MultiArityWT::access(size_t i, size_t &r) const {
        return root->access(i, r);
    }

    size_t MultiArityWT::getSize() const {
        //TODO restore this back
//        return tmp_space;
        return sizeof(this) + root->getSize();
    }

    void MultiArityWT::save(ofstream &fp) const {
        uint wr = SEQUENCE_MULTI_ARY_WT_HDR;
        saveValue(fp, wr);
        saveValue(fp, length);
        saveValue(fp, sigma);
        coder->save(fp);
        root->save(fp);
    }

    Sequence *MultiArityWT::load(ifstream &fp) {
        uint wr;
        wr = loadValue<uint>(fp);
        if (wr != SEQUENCE_MULTI_ARY_WT_HDR) {
            return nullptr;
        }
        MultiArityWT *ret = new MultiArityWT();
        ret->length = loadValue<size_t>(fp);
        ret->sigma = loadValue<uint>(fp);
        ret->coder = MultiArityCoder::load(fp);
        ret->root = MultiArityNode::load(fp);
        return ret;
    }
}
