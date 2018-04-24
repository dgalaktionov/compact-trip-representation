/* wt_coder_hutucker.h
 * Copyright (C) 2014-current_year, Alberto Ordóñez, all rights reserved.
 *
 * Builds a Hu-Tucker encoding. It represents the set of codes as a binary tree
 * using pointers.
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

#include <sequence/wt_coder_hutucker.h>
#include <climits>
#include <stdexcept>
#include <sequence/htwt/hu-tucker/hutucker.h>
#include <trees/Parentheses.h>

namespace cds_static
{

	wt_coder_hutucker::wt_coder_hutucker(uint *freqs, uint nfreqs):wt_coder(){
		sigma = nfreqs;
		huTucker *ht = createHuTucker((int*)freqs, nfreqs);
		// std::cout << std::endl;
		// printLevels(*ht);
		// std::cout << std::endl;
		// printHuTucker(*ht);
		leaves = new BinaryTree<int>*[nfreqs];
		uint symbol=0;
		root = buildTree(ht->root,symbol);
		assert(symbol==nfreqs);
		assert(testCodeDecode());
		//testBuildTree();
	}

    wt_coder_hutucker::wt_coder_hutucker() {}

    wt_coder_hutucker::~wt_coder_hutucker() {
    }

	uint wt_coder_hutucker::getCode(uint symbol, uint &code) const{
		assert(symbol<sigma);

		code = 0;
		BinaryTree<int> *b = leaves[symbol];
		assert(b);
		uint pos=0;
		while(b && b->getParent()){
			BinaryTree<int> *p = b->getParent();
			if (p->getRight()==b){
				bit_set(&code, pos);
			}
			pos++;
			b=p;
		}
		assert(bits(code)<=pos);
		return pos;
		// code = tmp_table[symbol].code;
		// return tmp_table[symbol].len;
	}

	uint wt_coder_hutucker::getSymbol(uint code, uint len) const{
		BinaryTree<int> *b = root;
		uint pos = 0;

		while(b->getLeft()!=nullptr && b->getRight()!=nullptr){
			if (bitget(&code,len-pos-1)){
				b=b->getRight();
			}else{
				b=b->getLeft();
			}
			pos++;
		}
		return b->getValue();
	}


    bool wt_coder_hutucker::testCodeDecode(){
    	uint code1,code2;
    	uint len1,len2;
    	len1 = getCode(0,code1);
    	for (uint i=1;i<sigma;i++){
    		len2 = getCode(i,code2);
    		int pc1,pc2;
    		pc1 = len1-1;
    		pc2 = len2-1;
    		while(pc1>=0 && pc2>=0){
    			uint b1,b2;
    			b1 = bitget(&code1,pc1);
    			b2 = bitget(&code2,pc2);
    			pc1--;pc2--;
    			if (b1==b2) continue;
    			assert(b1<b2);
    			break;
    		}

    		uint symb = getSymbol(code1, len1);
    		assert(symb==i-1);

    		len1=len2;
    		code1=code2;
    	}
    	uint symb = getSymbol(code1,len1);
    	assert(symb==sigma-1);
    	return true;
    }

    size_t wt_coder_hutucker::getSize() const
    {
		return 4*sigma / 8;
    }

    void wt_coder_hutucker::save(ofstream & fp) const
    {
    	uint *par;
    	createEmptyBitmap(&par, 4*sigma);

    	uint len=0;
    	obtainTreeParenthesesCoder(root, par,len);
    	assert(Parentheses::isATree(par,len));
    	cerr << endl;
			saveValue(fp,WT_CODER_HUTUCKER);
    	saveValue(fp,sigma);
    	saveValue(fp,len);
    	saveValue(fp,par,uint_len(len,1));
    	tuple *table_symbols_codes = new tuple[sigma];
    	for (uint i=0;i<sigma;i++){
    		table_symbols_codes[i].symbol=i;
    		table_symbols_codes[i].len = getCode(i,table_symbols_codes[i].code);
    	}
    	saveValue(fp, table_symbols_codes, sigma);
    }

    wt_coder_hutucker * wt_coder_hutucker::load(ifstream & fp) {
		uint rd = loadValue<uint>(fp);
		if(rd!=WT_CODER_HUTUCKER) return NULL;
		wt_coder_hutucker *wh = new wt_coder_hutucker();
		wh->sigma = loadValue<uint>(fp);
		uint len = loadValue<uint>(fp);
		uint *par;
		par = loadValue<uint>(fp,uint_len(len,1));

		assert(Parentheses::isATree(par, len));
		tuple *table_symbols_codes = new tuple[wh->sigma];
		table_symbols_codes = loadValue<tuple>(fp,wh->sigma);
		wh->tmp_table = table_symbols_codes;



		// uint *newbmp;
		// createEmptyBitmap(&newbmp, len);
		// for (uint i=0;i<len/2;i++){
		// 	if (bitget(par,i)){
		// 		bit_set(newbmp,len-i-1);
		// 	}
		// 	if (bitget(par,len-i-1)){
		// 		bit_set(newbmp,i);
		// 	}
		// }
		// Parentheses *p = new BP(newbmp,len,false);
		Parentheses *p = GCT::GCTFromBitmap(1,6,par,len);
		cerr << "p->findclose(0): " << p->findClose(0) << endl;
		int excess=0;


		uint pointer, symbol;
		pointer=0;symbol=0;
		excess=0;
		wh->leaves = new BinaryTree<int> *[wh->sigma];
		wh->root = new BinaryTree<int>();

	 	wh->buildTree(par,pointer,excess,symbol,wh->root);
	 	assert(pointer==len);
	 	assert(excess==0);
	 	assert(symbol==wh->sigma);
	 	assert(wh->testLoad(par, len));

	 	return wh;
	}

	BinaryTree<int> * wt_coder_hutucker::buildTree(bNode *node, uint &symbol){
		if (!node)
			return nullptr;
		assert((!node->children[0] && !node->children[1])|| (node->children[0] && node->children[1]));

		BinaryTree<int> *b = new BinaryTree<int>();

		if (node->children[0]==nullptr && node->children[1]==nullptr){
			//node is a leaf: both children are null
			b->setValue(symbol);
			b->setLeft(nullptr);
			b->setRight(nullptr);
			leaves[symbol++]=b;
			return b;
		}

		BinaryTree<int> *l,*r;
		l = buildTree(node->children[0],symbol);
		b->setLeft(l);
		if (l) l->setParent(b);

		r = buildTree(node->children[1],symbol);
		b->setRight(r);
		if (r) r->setParent(b);
		return b;
	}

	BinaryTree<int> * wt_coder_hutucker::buildTree(uint *par, uint &pointer, int &excess, uint &symbol, BinaryTree<int> *&node){
		   if (excess==0 && pointer)
		   	return nullptr;

		   if (bitget(par,pointer)){
		   		//cerr << "1";
		   		node = new BinaryTree<int>();
		   		pointer++;
		   		excess++;
		   		if (!bitget(par,pointer)){
		   			leaves[symbol]=node;
					node->setValue(symbol);
					node->setLeft(nullptr);
					node->setRight(nullptr);
					excess--;
					pointer++;
					symbol++;
					//cerr << "0";
			   	}else{
			   		BinaryTree<int> *l,*r;
			   		buildTree(par,pointer,excess,symbol,l);
			   		node->setLeft(l);
			   		buildTree(par,pointer,excess,symbol,r);
			   		node->setRight(r);
			   		assert(!bitget(par,pointer));
			   		pointer++;
			   		excess--;
			   	}

		   }else{
		   		//cerr << "0";
		   		excess--;
		   		pointer++;
		   		node = nullptr;
		   }
		return node;
	}

	uint  wt_coder_hutucker::obtainTreeParenthesesCoder(bNode *node, uint *v, uint &p) const{
		if (!node)
			return p;
		assert((!node->children[0] && !node->children[1])|| (node->children[0] && node->children[1]));
		bit_set(v, p++);

		obtainTreeParenthesesCoder(node->children[0],v,p);
		obtainTreeParenthesesCoder(node->children[1],v,p);
		p++;

		return p;
	}

	uint  wt_coder_hutucker::obtainTreeParenthesesCoder(BinaryTree<int> *node, uint *v, uint &p) const{

		if (node==nullptr)
			return p;
		//cerr << "1";
		bit_set(v, p++);
		obtainTreeParenthesesCoder(node->getLeft(),v,p);
		obtainTreeParenthesesCoder(node->getRight(),v,p);
		//cerr << "0";
		p++;
		return p;
	}

	//shit

    bool wt_coder_hutucker::is_set(uint symbol, uint l) const{
			uint code = 0, len = 0;
			len = getCode(symbol, code);

			if (l >= len)
				return false; // This is bad...

			if((1<<(len-l-1))&code) return true;
			return false;
    }
		bool wt_coder_hutucker::is_set(uint *symbol, uint l) const {
        return bitget(symbol,l);
    }
    bool wt_coder_hutucker::done(uint symbol, uint l) const{
    	return false;
    }
    uint * wt_coder_hutucker::get_symbol(uint symbol) {
    	uint code = 0, len = 0;
			_code = 0;
			len = getCode(symbol, code);

			for (size_t i = 0; i < len; i++) {
				if (bitget(&code, i)) bit_set(&_code, len-i-1);
			}

			return &_code;
    }

    bool wt_coder_hutucker::testTreeStructure (BinaryTree<int>* node){
    	if (node->getLeft()){
    		assert(node->getLeft()->getParent()==node);
    		assert(node->getLeft());
    	}
    	if (node->getRight()){
    		assert(node->getRight()->getParent()==node);
    		assert(node->getRight());
    	}
    	return true;
    }

    bool wt_coder_hutucker::testLoad(uint *par, uint len){
    	uint *bmp;
	 	createEmptyBitmap(&bmp, len);
	 	uint pointer=0;
		obtainTreeParenthesesCoder(root,bmp,pointer);
		 assert(pointer==len);
		 for (uint i=0;i<len;i++){
		 	assert(bitget(bmp,i)==bitget(par,i));
		 }

	 	testCodeDecode();
	 	assert(testTreeStructure(root->getLeft()) && testTreeStructure(root->getRight()));
	 	return true;
    }

    bool wt_coder_hutucker::testBuildTree(){
    	uint par=0;
		bit_set(&par, 0);
		bit_set(&par, 1);
		bit_set(&par, 2);
    	//bit_set(&par,3)=0;
    	//bit_set(&par,4)=0;
		bit_set(&par, 5);
    	//bit_set(&par,6)=0;
    	//bit_set(&par,7)=0;
    	BinaryTree<int> *node = new BinaryTree<int>();
    	uint pointer,symbol;
    	int excess=0;
    	pointer=symbol=0;
    	buildTree(&par, pointer, excess, symbol, node);
    	return true;
    }
};
