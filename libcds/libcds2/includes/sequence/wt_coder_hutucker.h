/* wt_coder_hutucker.h
 * Copyright (C) 2014-current_year, Alberto Ordóñez, all rights reserved.
 *
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

#ifndef _WT_CODER_HU_TUCKER_H
#define _WT_CODER_HU_TUCKER_H

#include <utils/libcdsBasics.h>
#include <coders/HuffmanCoder.h>
#include <sequence/wt_coder_huff_can.h>
#include <mapper/Mapper.h>
#include <sequence/Tuple.h>
#include <sequence/MappingSymbols.h>
#include <utils/HashTable.h>
#include <utils/Array.h>
#include <utils/BinaryTree.h>

namespace cds_static
{


    /** Uses huffman codes to determine the shape of the wavelet tree
     *
     *  @author Alberto Ordóñez
     */
    class wt_coder_hutucker: public wt_coder
    {
        public:
            wt_coder_hutucker(uint *freqs, uint max_v);
            virtual ~wt_coder_hutucker();
            virtual uint getCode(uint symbol, uint &code) const;
            virtual uint getSymbol(uint code, uint len) const;
            virtual size_t getSize() const;
			virtual void save(ofstream & fp) const;
            static wt_coder_hutucker * load(ifstream & fp);

            virtual bool is_set(uint symbol, uint l) const;
            virtual bool is_set(uint *symbol, uint l) const;
            virtual bool done(uint symbol, uint l) const;
            virtual uint * get_symbol(uint symbol);

        protected:
            uint sigma;//[0,sigma)
            BinaryTree<int> *root;
            BinaryTree<int> **leaves;
						uint _code;
            tuple *tmp_table;


            wt_coder_hutucker();
            BinaryTree<int> *buildTree(bNode *node, uint &symbol);
            BinaryTree<int> *buildTree(uint *par, uint &pointer, int &excess, uint &symbol,BinaryTree<int> *&node);
            uint obtainTreeParenthesesCoder(bNode *node, uint *, uint &) const;
            uint obtainTreeParenthesesCoder(BinaryTree<int> *node, uint *, uint &) const;
            bool testBuildTree();
            bool testTreeStructure (BinaryTree<int>* node);
            bool testLoad(uint *par, uint len);
            //tests
            bool testCodeDecode();
    };
};

#endif
