/*  
 * Copyright (C) 2013-current_year, Alberto Ordóñez, all rights reserved.
 *
 * Alberto Ordóñez <alberto.ordonez@udc.es>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <grammar/DictionaryRP.h>
#include <grammar/Repair.h>
#include <climits>


namespace cds_static{

	DictionaryRP::~DictionaryRP(){}
    DictionaryRP::DictionaryRP():R(0),alpha(0),alphaPlus1(0){}
    DictionaryRP::DictionaryRP(int _R, uint _alpha):R(_R),alpha(_alpha-1),alphaPlus1(_alpha){}

    DictionaryRP *DictionaryRP ::load(ifstream &in){
        uint wr = loadValue<uint>(in);
        size_t pos = in.tellg();
        in.seekg(pos-sizeof(wr),ios::beg);
        switch(wr){
            case DICT_RP_PLAIN:
                return DictionaryRPPlain::load(in);
            default:
                return DictionaryRPTabei::load(in);
        }
    }

    uint DictionaryRP::getAlph() {
        return alphaPlus1;
    }

    int DictionaryRP::getNRules() {
        return R;
    }

    bool DictionaryRP::testDictionaryRP(uint *LEFT, uint *RIGHT, uint ALPH, int R, uint *PERM){
        for (int i=ALPH;i<R;i++){
            assert(LEFT[i-ALPH]==getLeft(PERM[i]));
            assert(RIGHT[i-ALPH]==getRight(PERM[i]));
        }
        //cerr << "Test DictionaryRPTabei OK!" << endl;
        return true;
    }
}
