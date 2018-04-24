//
// Created by alberto on 8/12/15.
//

#include <grammar/DictionaryRPPlain.h>

cds_static::DictionaryRPPlain::DictionaryRPPlain(uint *LEFT, uint *RIGHT, uint _alph, int _R, uint *&perm):DictionaryRP(_R,_alph) {
    perm = new uint[R+alphaPlus1];
    vector<uint> left, right;
    for (int i=0;i<alphaPlus1;i++){
        left.push_back(0);
        right.push_back(0);
    }
    for (int i=0;i<R;i++){
        left.push_back(LEFT[i]);
        right.push_back(RIGHT[i]);
        perm[i] = static_cast<uint>(i);
    }
    this->left = GenericArray<uint>(left);
    this->right = GenericArray<uint>(right);
//    assert(testDictionaryRP(LEFT,RIGHT,alpha,R,perm));
}

cds_static::DictionaryRPPlain::~DictionaryRPPlain() { }

uint cds_static::DictionaryRPPlain::getRight(size_t rule) {
    assert(rule<right.getLength());
    return right[rule];
}

uint cds_static::DictionaryRPPlain::getLeft(size_t rule) {
    assert(rule<left.getLength());
    return left[rule];
}

size_t cds_static::DictionaryRPPlain::getSize() {
    return sizeof(this)+left.getSize()+right.getSize();
}

size_t cds_static::DictionaryRPPlain::getSizeVerbose() {
    cout << "Size left: " << left.getSize()<< endl;
    cout << "Size right: " << right.getSize() << endl;
    return getSize();
}

void cds_static::DictionaryRPPlain::save(ofstream &of) {
    uint wr = DICT_RP_PLAIN;
    saveValue<uint>(of,wr);
    saveValue<int>(of,R);
    saveValue<uint>(of,alpha);
    saveValue<uint>(of,alphaPlus1);
    left.save(of);
    right.save(of);
}

cds_static::DictionaryRPPlain *cds_static::DictionaryRPPlain::load(ifstream &in) {
    uint wr = loadValue<uint>(in);
    if (wr!=DICT_RP_PLAIN)
        return nullptr;
    DictionaryRPPlain *dic = new DictionaryRPPlain();
    dic->R = loadValue<int>(in);
    dic->alpha = loadValue<uint>(in);
    dic->alphaPlus1 = loadValue<uint>(in);
    dic->left = GenericArray<uint>(in);
    dic->right = GenericArray<uint>(in);
//    ulong leftSize = loadValue<ulong>(in);
//    uint *l, *r;
//    l = loadValue<uint>(in,leftSize);
//    ulong rightSize = loadValue<ulong>(in);
//    r = loadValue<uint>(in,rightSize);
//    dic->left  = vector<uint>(l,l+leftSize);
//    dic->right = vector<uint>(r,r+rightSize);
//    delete [] l;
//    delete [] r;
    return dic;
}

cds_static::DictionaryRPPlain::DictionaryRPPlain():DictionaryRP() {}
