//
// Created by alberto on 7/22/15.
//

#include <sequence/MultiArity/MultiArityCoder.h>
#include <utils/cppUtils.h>

using namespace cds_utils;

namespace cds_static{

    MultiArityCoder* MultiArityCoder::load(ifstream &fp){

        uint type = loadValue<uint>(fp);
        size_t pos = static_cast<size_t>(fp.tellg());
        fp.seekg(pos-sizeof(uint),ios::beg);

        switch (type){
            case MULTIARY_CODER_FIXED:
                return MultiArityCoderFixed::load(fp);
            case MULTIARY_CODER_HUFFMAN:
                return MultiArityCanonincalHuffman::load(fp);
            default:
                return nullptr;
        }
    }

}
