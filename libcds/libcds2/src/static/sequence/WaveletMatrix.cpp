/* WaveletMatrix.cpp
 * Copyright (C) 2012, Francisco Claude & Gonzalo Navarro, all rights reserved.
 *
 * Range search added by Alberto Ordóñez.
 *
 * WaveletMatrix definition
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

#include <sequence/WaveletMatrix.h>
#include <queue>

namespace cds_static
{

	WaveletMatrix::WaveletMatrix(const Array &symbols2, BitSequenceBuilder * bmb, Mapper * am) : Sequence(n) {
		bmb->use();
		n = symbols2.getLength();
		uint *symbols = new uint[n];
		this->am = am;
		am->use();
		for(uint i = 0; i < n; i++)
			symbols[i] = am->map(symbols2.getField(i));

		max_v = max_value(symbols, n);
		height = bits(max_v);

		OCC = new uint[max_v + 2];
		for (uint i = 0; i <= max_v + 1; i++)
			OCC[i] = 0;
		for (uint i = 0; i < n; i++)
			OCC[symbols[i] + 1]++;

		uint to_add = 0;
		for (uint i = 1; i <= max_v + 1; i++)
			if (OCC[i] == 0) to_add++;

		uint * new_symb = new uint[n + to_add];
		for (uint i = 0; i < n; i++)
			new_symb[i] = symbols[i];
		delete [] symbols;

		to_add = 0;
		for (uint i = 1; i <= max_v + 1; i++)
		if (OCC[i] == 0) {
			OCC[i]++;
			new_symb[n + to_add] = i - 1;
			to_add++;
		}

		uint new_n = n + to_add;
		this->n = new_n;

		uint **_bm=new uint*[height];
		for(uint i = 0; i < height; i++) {
			_bm[i] = new uint[new_n / W + 1];
			for(uint j = 0;j < new_n / W + 1; j++)
				_bm[i][j] = 0;
		}

		build_level(_bm, new_symb, new_n, NULL);
		bitstring = new BitSequence*[height];
		C = new size_t[height];
		for(uint i = 0; i < height; i++) {
			bitstring[i] = bmb->build(_bm[i], new_n);
			C[i] = bitstring[i]->rank0(new_n - 1);
			delete [] _bm[i];
		}
		delete [] _bm;
		// delete [] oc;
		bmb->unuse();

		this->length = n;
	}

	WaveletMatrix::WaveletMatrix(uint * symbols, size_t n, BitSequenceBuilder * bmb, Mapper * am, bool deleteSymbols) : Sequence(n) {
		bmb->use();
		this->n=n;
		this->am=am;
		am->use();
		for(uint i=0;i<n;i++)
			symbols[i] = am->map(symbols[i]);

		max_v=max_value(symbols,n);
		height=bits(max_v);

		OCC = new uint[max_v + 2];
		for (uint i = 0; i <= max_v + 1; i++)
			OCC[i] = 0;
		for (uint i = 0; i < n; i++)
			OCC[symbols[i] + 1]++;

		uint to_add = 0;
		for (uint i = 1; i <= max_v + 1; i++)
			if (OCC[i] == 0) to_add++;

		uint * new_symb = new uint[n + to_add];
		for (uint i = 0; i < n; i++)
			new_symb[i] = symbols[i];

		if (deleteSymbols) {
			delete [] symbols;
			symbols = 0;
		}

		to_add = 0;
		for (uint i = 1; i <= max_v + 1; i++)
		if (OCC[i] == 0) {
			OCC[i]++;
			new_symb[n + to_add] = i - 1;
			to_add++;
		}

		uint new_n = n + to_add;
		this->n = new_n;

		uint ** _bm=new uint*[height];
		for(uint i=0;i<height;i++) {
			_bm[i] = new uint[new_n/W+1];
			for(uint j=0;j<new_n/W+1;j++)
				_bm[i][j]=0;
		}
		build_level(_bm, new_symb, new_n, NULL);
		bitstring = new BitSequence*[height];
		C = new size_t[height];
		for(uint i=0;i<height;i++) {
			bitstring[i] = bmb->build(_bm[i],new_n);
			C[i] = bitstring[i]->rank0(new_n - 1);
			// cout << "C=" << C[i] << endl;
			delete [] _bm[i];
		}
		delete [] _bm;

		if (!deleteSymbols)
			for(uint i=0;i<n;i++)
				symbols[i] = am->unmap(symbols[i]);

		// delete [] new_symb; // already deleted in build_level()!
		// delete [] oc;
		bmb->unuse();
		// for(uint i=0;i<height;i++)
		//     cout << "C=" << C[i] << endl;
		this->length = n;
	}
	WaveletMatrix::WaveletMatrix(uint * symbols, size_t n, vector<BitSequenceBuilder*> &bsbs, Mapper * am, bool deleteSymbols) : Sequence(n) {
		for (auto it: bsbs)
			it->use();

		this->n=n;
		this->am=am;
		am->use();
		for(uint i=0;i<n;i++)
			symbols[i] = am->map(symbols[i]);

		max_v=max_value(symbols,n);
		height=bits(max_v);

		OCC = new uint[max_v + 2];
		for (uint i = 0; i <= max_v + 1; i++)
			OCC[i] = 0;
		for (uint i = 0; i < n; i++)
			OCC[symbols[i] + 1]++;

		uint to_add = 0;
		for (uint i = 1; i <= max_v + 1; i++)
			if (OCC[i] == 0) to_add++;

		uint * new_symb = new uint[n + to_add];
		for (uint i = 0; i < n; i++)
			new_symb[i] = symbols[i];

		if (deleteSymbols) {
			delete [] symbols;
			symbols = 0;
		}

		to_add = 0;
		for (uint i = 1; i <= max_v + 1; i++)
		if (OCC[i] == 0) {
			OCC[i]++;
			new_symb[n + to_add] = i - 1;
			to_add++;
		}

		size_t new_n = n + to_add;
		this->n = new_n;

		uint ** _bm=new uint*[height];
		for(uint i=0;i<height;i++) {
			_bm[i] = new uint[new_n/W+1];
			for(uint j=0;j<new_n/W+1;j++)
				_bm[i][j]=0;
		}
		build_level(_bm, new_symb, new_n, NULL);
		bitstring = new BitSequence*[height];
		C = new size_t[height];

        bool trySeveralBitmaps = true;
		for(uint i=0;i<height;i++) {
//            cerr << "height " << i << " out of " << height << endl;
			if (trySeveralBitmaps) {
                uint index = buildBestBitmap(bsbs, _bm[i], new_n, bitstring[i]);
                trySeveralBitmaps &= (index==0);
            }else{
                bitstring[i] = bsbs[0]->build(_bm[i],new_n);
            }

			C[i] = bitstring[i]->rank0(new_n - 1);
			delete [] _bm[i];
		}
		delete [] _bm;

		if (!deleteSymbols)
			for(uint i=0;i<n;i++)
				symbols[i] = am->unmap(symbols[i]);
		for (auto it: bsbs)
			it->unuse();
		this->length = n;
	}

	WaveletMatrix::WaveletMatrix(uint * symbols, size_t n, BitSequenceBuilder * bmb, BitSequenceBuilder * bmb2, Mapper * am, bool deleteSymbols) : Sequence(n) {
		bmb->use();
		this->n=n;
		this->am=am;
		am->use();
		for(uint i=0;i<n;i++)
			symbols[i] = am->map(symbols[i]);

		max_v=max_value(symbols,n);
		height=bits(max_v);

		OCC = new uint[max_v + 2];
		for (uint i = 0; i <= max_v + 1; i++)
			OCC[i] = 0;
		for (uint i = 0; i < n; i++)
			OCC[symbols[i] + 1]++;

		uint to_add = 0;
		for (uint i = 1; i <= max_v + 1; i++)
			if (OCC[i] == 0) to_add++;

		uint * new_symb = new uint[n + to_add];
		for (uint i = 0; i < n; i++)
			new_symb[i] = symbols[i];

		if (deleteSymbols) {
			delete [] symbols;
			symbols = 0;
		}

		to_add = 0;
		for (uint i = 1; i <= max_v + 1; i++)
		if (OCC[i] == 0) {
			OCC[i]++;
			new_symb[n + to_add] = i - 1;
			to_add++;
		}

		size_t new_n = n + to_add;
		this->n = new_n;

		uint ** _bm=new uint*[height];
		for(uint i=0;i<height;i++) {
			_bm[i] = new uint[new_n/W+1];
			for(uint j=0;j<new_n/W+1;j++)
				_bm[i][j]=0;
		}
		build_level(_bm, new_symb, new_n, NULL);
		bitstring = new BitSequence*[height];
		C = new size_t[height];

		for(uint i=0;i<height;i++) {
			if (i>0){
				bitstring[i] = bmb->build(_bm[i],new_n);
			}else{
				bitstring[i] = bmb2->build(_bm[i],new_n);
			}
			C[i] = bitstring[i]->rank0(new_n - 1);
			// cout << "C=" << C[i] << endl;
			delete [] _bm[i];
		}
		delete [] _bm;

		if (!deleteSymbols)
			for(uint i=0;i<n;i++)
				symbols[i] = am->unmap(symbols[i]);

		// delete [] new_symb; // already deleted in build_level()!
		// delete [] oc;
		bmb->unuse();
		// for(uint i=0;i<height;i++)
		//     cout << "C=" << C[i] << endl;
		this->length = n;
	}

	WaveletMatrix::WaveletMatrix():Sequence(0) {
		bitstring = NULL;
		// occ = NULL;
		am = NULL;
	}

	WaveletMatrix::~WaveletMatrix() {
		if(bitstring) {
			for(uint i=0;i<height;i++)
				if(bitstring[i])
					delete bitstring[i];
			delete [] bitstring;
		}
        delete [] OCC;
		// if(occ)
		// 	delete occ;
		if(am)
			am->unuse();
		delete [] C;
	}

	void WaveletMatrix::save(ofstream & fp) const
	{
		uint wr = WVMATRIX_HDR;
		saveValue(fp,wr);
		saveValue<size_t>(fp,n);
		saveValue(fp,max_v);
		saveValue(fp,height);
		saveValue(fp, C, height);
		am->save(fp);
		for(uint i=0;i<height;i++)
			bitstring[i]->save(fp);
		// occ->save(fp);
		saveValue<uint>(fp, OCC, max_v + 2);
	}

	WaveletMatrix * WaveletMatrix::load(ifstream & fp) {
		uint rd = loadValue<uint>(fp);
		if(rd!=WVMATRIX_HDR) return NULL;
		WaveletMatrix * ret = new WaveletMatrix();
		ret->n = loadValue<size_t>(fp);
		ret->length = ret->n;
		ret->max_v = loadValue<uint>(fp);
		ret->height = loadValue<uint>(fp);
		ret->C = loadValue<size_t>(fp, ret->height);
		ret->am = Mapper::load(fp);
		if(ret->am==NULL) {
			delete ret;
			return NULL;
		}
		ret->am->use();
		ret->bitstring = new BitSequence*[ret->height];
		for(uint i=0;i<ret->height;i++)
			ret->bitstring[i] = NULL;
		for(uint i=0;i<ret->height;i++) {
			ret->bitstring[i] = BitSequence::load(fp);
			if(ret->bitstring[i]==NULL) {
				cout << "damn" << i << " " << ret->height << endl;
				delete ret;
				return NULL;
			}
		}
		ret->OCC = loadValue<uint>(fp, ret->max_v + 2);
		// ret->occ = BitSequence::load(fp);
		// if(ret->occ==NULL) {
		// 	delete ret;
		// 	return NULL;
		// }
		return ret;
	}

	inline uint get_start(uint symbol, uint mask) {
		return symbol&mask;
	}

	inline uint get_end(uint symbol, uint mask) {
		return get_start(symbol,mask)+!mask+1;
	}

	bool WaveletMatrix::is_set(uint val, uint ind) const
	{
		assert(ind<height);
		return (val & (1<<(height-ind-1)))!=0;
	}

	uint WaveletMatrix::set(uint val, uint ind) const
	{
		assert(ind<=height);
		return val | (1<<(height-ind-1));
	}

	uint WaveletMatrix::access(size_t pos) const
	{
		uint ret=0;
		uint invLevel=height-1;
		for (uint level = 0; level < height; level++) {
			size_t optR = 0;
			if (bitstring[level]->access(pos, optR)) {
				pos = C[level] + optR - 1;
				ret = ret | (1 << invLevel);
			}
			else {
				pos = optR - 1;
			}
			invLevel--;
		}
		return am->unmap(ret);
	}

	ulong WaveletMatrix::extract(ulong i, ulong j, vector<uint> &res) const{
		return extract(i,j,0U,0U,res);
	}

	//TODO
	ulong WaveletMatrix::extract(ulong i, ulong j, uint level, uint code, vector<uint> &res) const{
		if (i==j) return 0;
		if (level==height){
			for (ulong it=i;it<j;it++)
				res.push_back(code);
			return res.size();
		}
		ulong zeros_before_i = bitstring[level]->rank0(i-1);
		ulong rnk0 = bitstring[level]->rank0(j-1) - zeros_before_i;
		ulong rnk1 = (j-i)-rnk0;
		vector<uint> left, right;
		if (rnk0){
			extract(zeros_before_i,zeros_before_i+rnk0,level+1,code,left);
		}
		if (rnk1){
			auto ones_before_i = i-zeros_before_i;
			extract(C[level]+ones_before_i,C[level]+ones_before_i+rnk1,level+1,code | (1 << (height-level-1)),right);
		}

		//merge part: we iterate the interval check which is the correct position of each symbol
		assert(left.size()+right.size()==(j-i));
		auto iter_left = left.begin();
		auto iter_right = right.begin();
		for (ulong it=i;it<j;it++){
			if (bitstring[level]->access(it)){
				res.push_back(*iter_right);
				++iter_right;
			}else{
				res.push_back(*iter_left);
				++iter_left;
			}
		}
		return res.size();
	}

	size_t WaveletMatrix::rank(uint symbol, size_t pos) const
	{
		if (pos+1==0) return 0;
		symbol = am->map(symbol);
		for (uint level = 0; level < height; level++) {
			if(is_set(symbol, level)) {
				pos = bitstring[level]->rank1(pos) + C[level] - 1;
			}
			else {
				pos = pos - bitstring[level]->rank1(pos);
			}
		}
		return pos + 1 - OCC[symbol];
	}

	size_t WaveletMatrix::select(uint symbol, size_t j) const
	{
		symbol = am->map(symbol);
		size_t pos = OCC[symbol] + j-1;
		for (int level = height - 1; level >= 0; level--) {
								 // left
			if (pos < C[level]) {
				pos = bitstring[level]->select0(pos + 1);
			}					 // right
			else {
				pos = bitstring[level]->select1(pos - C[level] + 1);
			}
		}
		return pos;
	}



	size_t WaveletMatrix::rangeCount(size_t xs, size_t xe, uint ys, uint ye) const {
		return rngCount(xs,xe,am->map(ys),am->map(ye),0,0,max_v,0);
	}


	uint WaveletMatrix::trackUp(uint pos, int l) const {
		for (int i=l;i>=0;i--){
			if (pos>=C[i]){
				pos=bitstring[i]->select1(pos-C[i]+1);
			}else{
				pos=bitstring[i]->select0(pos+1);
			}
		}
		return pos;
	}

	uint WaveletMatrix::trackUp(uint pos) const {
		return trackUp(pos, height-1);
	}

	size_t WaveletMatrix::rng(int xs, int xe, int ys, int ye, uint current, int level, uint lefty, uint righty, std::pair<int,int> *limits, 
		std::pair<uint,uint> *limit_symbols, std::pair<uint,uint> *limit_levels) const {

		if ((lefty>=(uint)ys) && (righty<=(uint)ye)){
			if (limits != NULL){
				/*
				for (int i=xs;i<=xe;i++){
					pair<int,int> p;
					p.first=current;
					p.second=trackUp((uint)i,level-1);
					res->push_back(p);
				}
				*/

				if (limit_symbols == NULL) {
					if (limits->second == 0) {
						limits->first = xs;
						limits->second = xe;
					} else {
						if (xs < limits->first)
							limits->first = xs;

						if (xe > limits->second)
							limits->second = xe;
					}
				} else {
					if (lefty <= limit_symbols->first) {
						limits->first = xs;
						limit_symbols->first = lefty;
						limit_levels->first = level;
					}

					if (righty >= limit_symbols->second) {
						limits->second = xe;
						limit_symbols->second = righty;
						limit_levels->second = level;
					}
				}
			}
			return xe-xs+1;
		}
		int lc,rc;
		lc=rc=0;
		if (level==(int)height) return 0;
		int xs0,xe0,shift;
		shift=height-level-1; current<<=1;xs0=-1;
		if (current>=(uint)(ys>>shift)){
			xs0=bitstring[level]->rank0(xs-1);
			xe0=bitstring[level]->rank0(xe);
			if (xs0<xe0){
				uint newlefty=(current<<(shift));
				lc=rng(xs0,xe0-1,ys,ye,current,level+1,newlefty,newlefty|((1u<<(shift))-1),
					limits, limit_symbols, limit_levels);
			}
		}

		current|=0x1;
		if (current<=(uint)(ye>>shift)){
			if (xs0==-1){
				xs0=bitstring[level]->rank0(xs-1);
				xe0=bitstring[level]->rank0(xe);
			}
			int newxs1,newxs2;
			newxs1=xs-xs0+C[level];
			newxs2=xe-xe0+C[level];
			if (newxs1<=newxs2){
				lefty=(current<<(shift));
				rc=rng(newxs1,newxs2,ys,ye,current,level+1,lefty,lefty|((1u<<(shift))-1),
					limits, limit_symbols, limit_levels);
			}
		}
		return lc+rc;
	}

	size_t WaveletMatrix::rngCount(size_t xs, size_t xe, uint ys, uint ye, uint current, uint lefty, uint righty,int level) const {
		if (xs>xe) return 0;
		return rng(xs,xe,ys,ye,0,0,0,max_v,NULL,NULL,NULL);
	}


	size_t WaveletMatrix::range(int i1, int i2, int j1, int j2, std::pair<int,int> *limits, bool tu) const{
		if (i1>i2) return 0;
		assert(i2 < n);

		if (limits!=NULL && tu) {
			std::pair<uint,uint> limit_symbols = std::make_pair<uint,uint>(j2,j1);
			std::pair<uint,uint> limit_levels;
			size_t res = rng(i1,i2,j1,j2,0,0,0,max_v,limits,&limit_symbols,&limit_levels);

			if (res) {
				limits->first = trackUp(limits->first, limit_levels.first-1);
				limits->second = trackUp(limits->second, limit_levels.second-1);
			}

			return res;
		} else {
			return rng(i1,i2,j1,j2,0,0,0,max_v,limits, NULL, NULL);
		}
	}
	size_t WaveletMatrix::getSize() const
	{
		size_t ptrs = sizeof(WaveletMatrix)+height*sizeof(Sequence*);
		size_t bytesBitstrings = 0;
		for(uint i=0;i<height;i++){
			bytesBitstrings += bitstring[i]->getSize();
		}
		//In case of range search, OCC is not used and then it does not contribute
		//to the size of the structure
		#ifdef RANGESEARCH
		return bytesBitstrings /* + occ->getSize() */ + ptrs
			+ height * sizeof(uint) ;
		#else
		return bytesBitstrings /* + occ->getSize() */ + ptrs
			+ height * sizeof(uint) + sizeof(uint) * (max_v + 2);
		#endif
	}

	void WaveletMatrix::build_level(uint **bm, uint *symbols, size_t length, uint *occs) {
		uint sigma = max_value(symbols, length);
		uint *new_order = new uint[sigma + 1];
		for (uint level = 0; level < height; level++) {
			uint zeroes = 0;
			for (uint i = 0; i < sigma + 1; i++)
			if (!is_set(i, level)) {
				new_order[i] = 0;
			}
			else {
				new_order[i] = 1;
			}
			for (uint i = 0; i < length; i++)
				if (!new_order[symbols[i]])
					zeroes++;
			uint *new_symbols = new uint[length];
			uint new_pos0 = 0, new_pos1 = zeroes;
			for (uint i = 0; i < length; i++) {
				if (!new_order[symbols[i]]) {
					new_symbols[new_pos0++] = symbols[i];
					bitclean(bm[level], i);
				}
				else {
					new_symbols[new_pos1++] = symbols[i];
					bit_set(bm[level], i);
				}
			}
			delete [] symbols;
			symbols = new_symbols;
			uint prev=-1;
			if (level+1==height){
				for (uint w=0;w<length;w++){
					if (symbols[w]!=prev){
						if (symbols[w]%2==0)
							OCC[symbols[w]]=w;
						else
							OCC[symbols[w]]=w;
						prev=symbols[w];
					}
				}
			}

		}
		delete [] symbols;
		delete [] new_order;
	}



	uint WaveletMatrix::max_value(uint *symbols, size_t n) {
		uint max_v = 0;
		for(size_t i=0;i<n;i++)
			max_v = max(symbols[i],max_v);
		return max_v;
	}

	uint WaveletMatrix::bits(uint val) const {
		uint ret = 0;
		while(val!=0) {
			ret++;
			val >>= 1;
		}
		return ret;
	}

	size_t WaveletMatrix::get_occ(vector<uint> &res) const{
		res.assign(OCC, OCC + max_v + 1);
		return res.size();
	}

	typedef struct {
		uint node;
		uint freq;
	} TopK_Item;

	bool topK_cmp(const TopK_Item a, const TopK_Item b) {
		return a.freq>b.freq;
	}

	void sift_down(std::vector<TopK_Item> &topK, ulong position) {
		ulong left, right;

		left = position*2 + 1;
		right = position*2 + 2;

		while (right < topK.size()) {
			if (topK[position].freq > topK[left].freq || topK[position].freq > topK[right].freq) {
				if (topK[left].freq < topK[right].freq) {
					std::swap(topK[position], topK[left]);
					position = left;
				} else {
					std::swap(topK[position], topK[right]);
					position = right;
				}
			} else {
				// We're done. Don't use break to enable more compiler optimizations
				position = topK.size();
			}

			left = position*2 + 1;
			right = position*2 + 2;
		}

		if (left < topK.size() && topK[position].freq > topK[left].freq) {
			std::swap(topK[position], topK[left]);
		}
	}

	ulong WaveletMatrix::topK(uint k, uint ys, uint ye, uint *res) const {
		if (ys > ye) return 0;

		uint i = 0;
		k = min(k, ye-ys+1);
		uint OCC_1[max_v + 1];
		uint freq[max_v + 1];
		std::vector<TopK_Item> topK(k, {0,0});
		TopK_Item item = {0, 0};

		for (i = 0; i <= max_v; i++) {
			OCC_1[i] = i;
		}

		std::sort(OCC_1, OCC_1+max_v+1, [this](const uint a, const uint b) {
			return OCC[a] < OCC[b];
		});

		for (i = 0; i < max_v; i++) {
			freq[OCC_1[i]] = OCC[OCC_1[i+1]] - OCC[OCC_1[i]];
		}
		freq[OCC_1[max_v]] = length - OCC[OCC_1[max_v]];

		for (i = ys; i < k; i++) {
			item = {i, freq[i]};
			topK[i] = item;
		}

		std::make_heap(topK.begin(), topK.end(), topK_cmp);

		for (; i <= ye; i++) {
			if (freq[i] > topK.front().freq) {
				item = {i, freq[i]};
				topK[0] = item;
				sift_down(topK, 0);
			}
		}

		std::sort_heap (topK.begin(), topK.end(), topK_cmp);

		/*
		std::cout << "final sorted range :\n";
		for (i=0; i<topK.size(); i++) {
			  std::cout << ' ' << topK[i].node << ' ' << topK[i].freq << '\n';
		  }
		std::cout << '\n';
		*/


		for (i = 0; i < k; i++)
			res[i] = topK[i].node;

		return k;
	}

	/*
	typedef struct {
		uint start_v;
		uint end_v;
		size_t start_i;
		size_t end_i;
	} BinTopK_Item;

	class TopK_Cmp {
	public:
		bool operator() (BinTopK_Item a, BinTopK_Item b) {
			return (b.end_i - b.start_i) > (a.end_i - a.start_i);
		}
	};

	ulong WaveletMatrix::topK2(uint k, uint ys, uint ye, uint *res) const {
		// FIXME
		if (ys > ye) return 0;

		uint current_k = 0;
		uint middle_v = 0;
		int middle_i = 0;
		int level = 0;

		std::priority_queue<BinTopK_Item, std::vector<BinTopK_Item>, TopK_Cmp> topKQueue;
		BinTopK_Item item = {ys, ye, 1, length-1};
		BinTopK_Item item_left;
		BinTopK_Item item_right;
		topKQueue.push(item);

		while (current_k < k) {
			item = topKQueue.top();
			topKQueue.pop();
			level = height - bits(item.end_v - item.start_v);

			if (level == height) {
				//std::cout << ' ' << item.start_v << ' ' << item.end_i - item.start_i + 1 << '\n';
				res[current_k++] = item.start_v;
			} else {
				middle_v = item.start_v + (1 << (height - level - 1));
				item_left.start_v = item.start_v;
				item_left.end_v = min(item.end_v, middle_v - 1);
				item_left.start_i = item.start_i ? bitstring[level]->rank0(item.start_i-1): 0;
				item_left.end_i = max((size_t) 0, bitstring[level]->rank0(item.end_i) - 1);
				topKQueue.push(item_left);

				if (middle_v <= item.end_v) {
					item_right.start_v = middle_v;
					item_right.end_v = item.end_v;
					item_right.start_i = C[level] + (item.start_i ? bitstring[level]->rank1(item.start_i-1) : 0);
					item_right.end_i = max((size_t) 0, C[level] + bitstring[level]->rank1(item.end_i) - 1);
					topKQueue.push(item_right);
				}
			}
		}

		return k;
	}
	*/

	ulong WaveletMatrix::topK(uint k, uint *res) const {
		return topK(k, 0, max_v, res);
	}

};
