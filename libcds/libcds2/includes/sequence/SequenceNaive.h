/**
 * Copyright (C) 2015, Alberto Ordóñez, all rights reserved.
 *
 * e-mail: ofrade9@gmail.com
 *
 * Naive sequence implementation. All operations are solved by
 * brute force algorithms. It is a parameterized class where you
 * must tell the data structure you want to use to store the
 * sequence. This D.S. must have defined operator[].
 * Times: access O(1), rank and select in O(n).
 */

#ifndef ALL_SEQUENCENAIVE_H
#define ALL_SEQUENCENAIVE_H

#include <sequence/Sequence.h>
#include <direct_access/DirectAccess.h>

namespace cds_static {
    template<typename DA> class SequenceNaive: public Sequence {
    public:
        SequenceNaive(uint *input, size_t len);
        SequenceNaive(const Array& array);
        ~SequenceNaive();

        virtual size_t rank(uint c, size_t i) const;

        virtual size_t rankInterval(uint c, size_t start, size_t end) const ;

        virtual size_t select(uint c, size_t i) const ;

        virtual size_t selectInterval(uint c, size_t i, size_t start, size_t end) const;

        virtual uint access(size_t i) const;

        void save(ofstream &fp) const ;

        static SequenceNaive* load(ifstream &fp);

        size_t getSize() const;
    protected:
        SequenceNaive();
        DA *content;
    };

    template<typename DA> SequenceNaive<DA>::~SequenceNaive(){
        delete content;
    }

    template<typename DA> SequenceNaive<DA>::SequenceNaive(uint *input, size_t len):Sequence(len){
        this->sigma = 0UL;
        for (ulong i=0;i<len;i++){
            this->sigma = max(this->sigma,input[i]);
        }
        content = new DA(input,len);
    }

    template<typename DA> SequenceNaive<DA>::SequenceNaive():Sequence(0){
        this->sigma = 0UL;
        content = nullptr;
    }

    template<typename DA> SequenceNaive<DA>::SequenceNaive(const Array &array):Sequence(array.getLength()){
        this->sigma = 0UL;
        uint *seq = new uint[array.getLength()];
        for (ulong i=0;i<length;i++){
            this->sigma = max(this->sigma,array[i]);
            seq[i] = array[i];
        }
        content = new DA(seq,length);
        delete [] seq;
    }

    template<typename DA> size_t SequenceNaive<DA>::rank(uint c, size_t i) const {
        if (i+1==(0UL)) return 0;
        size_t res = 0;
        for (ulong j=0;j<=i;j++){
            if ((*content)[j]==c)
                res++;
        }
        return res;
    }

    template<typename DA> size_t SequenceNaive<DA>::rankInterval(uint c, size_t start, size_t end) const {
        if (start+1==(0UL)) return 0UL;
        if (end<start) return 0UL;
        size_t res = 0UL;
        for (long i=start;i<=end;i++){
            if ((*content)[i]==c)
                res++;
        }
        return res;
    }

    template<typename DA> size_t SequenceNaive<DA>::select(uint c, size_t i) const {
        ulong cnt = 0UL;
        for (ulong j=0;j<length;j++){
            if ((*content)[j]==c) {
                cnt++;
                if (cnt==i){
                    return j;
                }
            }
        }
        return static_cast<size_t>(-1);
    }

    template<typename DA> size_t SequenceNaive<DA>::selectInterval(uint c, size_t i, size_t start, size_t end) const {
        if (end<start) return static_cast<size_t>(-1);
        if (end>length)  end=length-1;
        size_t cnt = 0UL;
        for (ulong j=start;j<=end;j++){
            if ((*content)[j]==c) {
                cnt++;
                if (cnt==i){
                    return static_cast<size_t>(j-start);
                }
            }
        }
        return static_cast<size_t>(-1);
    }

    template<typename DA> uint SequenceNaive<DA>::access(size_t i) const {
        return (*content)[i];
    }

    template<typename DA> size_t SequenceNaive<DA>::getSize() const{
        return sizeof(this)+this->content->getSize();
    }

    template<typename DA> void SequenceNaive<DA>::save(ofstream &fp) const{
        uint wr = SEQUENCE_NAIVE_HDR;
        saveValue(fp,wr);
        saveValue(fp,this->length);
        saveValue(fp,this->sigma);
        this->content->save(fp);
    }

    template<typename DA> SequenceNaive<DA>* SequenceNaive<DA>::load(ifstream &fp){
        uint wr;
        wr = loadValue<uint>(fp);
        if (wr!=SEQUENCE_NAIVE_HDR){
            return nullptr;
        }
        SequenceNaive<DA> *ret = new SequenceNaive<DA>();
        ret->length = loadValue<size_t>(fp);
        ret->sigma = loadValue<uint>(fp);
        ret->content = DA::load(fp);
        return ret;
    }

};
#endif //ALL_SEQUENCENAIVE_H
