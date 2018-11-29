/*  Sequence.cpp
 * Copyright (C) 2010, Francisco Claude, all rights reserved.
 *
 * Francisco Claude <fclaude@cs.uwaterloo.ca>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include <sequence/Sequence.h>
#include <sequence/BitmapsSequence.h>

namespace cds_static
{

	Sequence::Sequence(size_t _length) {
		length = _length;
	}

	size_t Sequence::rank(uint c, size_t i) const
	{
		size_t count = 0;
		for(size_t k=0;k<=i;k++)
			if(access(k)==c) count++;
		return count;
	}

    size_t Sequence::rankInterval(uint c, size_t start, size_t end) const {
        if (end<start) return 0UL;
        if (start>=length) return 0UL;
        if ((end+1)==0UL) return 0UL;
        return rank(c,end) - ((start==(0UL))?0:rank(c,start-1));
    }

 	uint Sequence::rankLessThan(uint &symbol, uint pos) const{
 		return 0;
 	}

	size_t Sequence::select(uint c, size_t j) const
	{
		size_t count = 0;
		for(size_t k=0;k<length;k++) {
			if(access(k)==c) {
				count++;
				if(count==j) return k;
			}
		}
		return length;
	}

    size_t Sequence::selectInterval(uint c, size_t i, size_t start, size_t end) const{
        size_t rnk = rank(c,start-1);
        auto ret = select(c,rnk+i);
        if (end>length) end=length-1UL;
        if (ret>end) return (size_t)-1;
        return ret-start;
    }

	size_t Sequence::selectNext(uint c, size_t j) const
	{
		uint pos = rank(c,j);
		return select(c,pos+1);
	}

	ulong Sequence::extract( ulong i, ulong j, vector<uint> &res) const{
		j = min(j,length);
		for (ulong it=i;it<j;it++){
			res.push_back(access(it));
		}
		return j-i;
	}

	uint Sequence::access(size_t i) const
	{
		#define prev(k) ((i>0)?rank(k,i-1):0)
		for(uint k=0;k<sigma;k++) {
			if(rank(k,i)>prev(k)) return k;
		}
		// throw exception
		return (uint)-1;
	}

	uint Sequence::operator[](size_t i) const
	{
		return access(i);
	}

	uint Sequence::access(size_t i, size_t & _rank) const
	{
		uint s = access(i);
		_rank = rank(s,i);
		return s;
	}

	size_t Sequence::rangeCount(size_t xs, size_t xe, uint ys, uint ye) const
	{
		return 0;
	}

	size_t Sequence::range(int i1, int i2, int j1, int j2, std::pair<int,int> *limits, bool trackUp) const {
		return 0;
	}

	Sequence * Sequence::load(ifstream & fp) {
		uint type = loadValue<uint>(fp);
		size_t pos = fp.tellg();
		fp.seekg(pos-sizeof(uint),ios::beg);
		switch(type) {
			case GMR_CHUNK_HDR: return SequenceGMRChunk::load(fp);
			case GMR_HDR: return SequenceGMR::load(fp);
			case BS_HDR: return BitmapsSequence::load(fp);
			case WVTREE_HDR: return WaveletTree::load(fp);
			case WVTREE_NOPTRS_HDR: return WaveletTreeNoptrs::load(fp);
			case ALPHPART_HDR: return SequenceAlphPart::load(fp);
			case ALPHPARTREP_HDR: return SequenceAlphPartRep::load(fp);
			case WVMATRIX_HDR: return WaveletMatrix::load(fp);
			case CANONICALWT_NOPTRS_HDR: return CanonicalWTNoptrs::load(fp);
			case WVMATRIX_HUFF_HDR: return WaveletMatrixHuff::load(fp);
			case SEQUENCE_REPAIRSN_HDR: return SequenceRepairSN::load(fp);
			case SEQUENCE_REPAIRSC_HDR: return SequenceRepairSC::load(fp);
			case SEQUENCE_WT_RP: return SequenceWTRP::load(fp);
			//case SEQUENCE_WT_NOPTRS_HYBRID: return WaveletTreeNoptrsHybrid::load(fp);
			case SEQUENCE_MATRIX_HDR: return SequenceMatrix::load(fp);
//			case SEQUENCE_LZEND_HDR: return SequenceLZEnd::load(fp);
			case SEQUENCE_BLOCK_GRAPH_HDR: return SequenceBlockGraph<uint,Sequence>::load(fp);
			case SEQUENCE_BINARY_HDR: return SequenceBinary::load(fp);
			case SEQUENCE_MULTI_ARY_WT_HDR: return MultiArityWT::load(fp);
            case SEQUENCE_NAIVE_HDR: return SequenceNaive<DirectAccess>::load(fp);
		}
		return NULL;
	}


	uint Sequence::buildBestBitmap(vector<BitSequenceBuilder*> &bmb, uint *bmp, size_t len, BitSequence *&bs){
        //TODO solve this: short vectors are not easily handle by RPSN/RPSC
        uint index=0;
        if (len<10000UL){
            bs = new BitSequenceRRR(bmp,len,33);
            index=1;
            cout << "Warning: the bitmap is too small (len=" << len << "). We are compressing it with RRR(33)" << endl;
        }else {
            bs = bmb[0]->build(bmp, len);
            for (uint j = 1; j < bmb.size(); j++) {
                BitSequence *b = bmb[j]->build(bmp, len);
                if (b->getSize() < bs->getSize()) {
                    swap(bs, b);
                    index = j;
                    delete b;
                } else {
                    delete b;
                }
            }
        }
        return index;
	}

	uint Sequence::buildBestSequence(vector<SequenceBuilder*> &bmb, uint *bmp, size_t len, Sequence *&bs){
		bs = bmb[0]->build(bmp,len);
        uint index =0;
		for (uint j=1;j<bmb.size();j++){
			Sequence *b = bmb[j]->build(bmp,len);
			if (b->getSize()<bs->getSize()){
				swap(bs,b);
                index = j;
				delete b;
			}else{
				delete b;
			}
		}
        return index;
	}

	ulong Sequence::topK(uint k, uint xs, uint xe, uint *res) const {
		return 0;
	}

	ulong Sequence::topK(uint k, uint *res) const {
		return 0;
	}

};
