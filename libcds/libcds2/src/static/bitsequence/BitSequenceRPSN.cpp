/* BitSequenceRPSN.cpp
 * Copyright (C) 2008, Francisco Claude, all rights reserved.
 *
 * BitSequenceRPSN definition
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

#include <bitsequence/BitSequenceRPSN.h>
#include <mapper/Mapper.h>
#include <sequence/SequenceRepairSC.h>
#include <bitsequence/BitSequenceBuilder.h>
#include <permutation/PermutationBuilder.h>

namespace cds_static
{

	BitSequenceRPSN::BitSequenceRPSN() {
		length=0;
		ones=0;
		seq=NULL;
	}


	BitSequenceRPSN::BitSequenceRPSN(uint * bitseq, size_t n, BitSequenceBuilder *bsb, PermutationBuilder *pb, uint samplingC, uint delta, uint ss_rate,DictionaryRPBuilder *dic,bool deleteSymbols) {
		length = n;
		ones = 0;
		uint *new_seq = new uint[n];
		for (size_t i=0;i<n;i++){
			if (bitget(bitseq,i)){
				new_seq[i] = 1;	
				ones++;
			}else{
				new_seq[i]=0;
			}
			
		}
		seq= new SequenceRepairSN(new_seq,n,bsb, new MapperNone(),pb,samplingC,delta,ss_rate,dic,deleteSymbols);
		delete [] new_seq;
	}

	BitSequenceRPSN::BitSequenceRPSN(uint * bitseq, size_t n, Repair *rp, BitSequenceBuilder *bsb, PermutationBuilder *pb, uint samplingC, uint delta, uint ss_rate,DictionaryRPBuilder *dic, bool deleteSymbols) {
		length = n;
		ones = 0;
		uint *new_seq = new uint[n];
		for (size_t i=0;i<n;i++){
			if (bitget(bitseq,i)){
				new_seq[i] = 1;	
				ones++;
			}else{
				new_seq[i]=0;
			}
			
		}
		seq= new SequenceRepairSN(new_seq,n,rp,bsb, new MapperNone(),pb,samplingC,delta,ss_rate,dic,deleteSymbols);
		delete [] new_seq;
	}
	bool BitSequenceRPSN::access(const size_t i) const
	{
		return seq->access(i)!=0;
	}

	bool BitSequenceRPSN::access(const size_t i, size_t &r) const
	{
        uint rr;
		bool a = seq->access(i,rr)!=0;
        r = (uint)rr;
		return a;
	}

	size_t BitSequenceRPSN::rank0(size_t i) const
	{
		return seq->rank(0,i);
	}

	size_t BitSequenceRPSN::rank1(size_t i) const
	{
		return seq->rank(1,i);
	}

	size_t BitSequenceRPSN::select0(size_t i) const
	{
		// if (i>length-ones) return -1;
		return seq->select(0,i);
	}

	size_t BitSequenceRPSN::select1(size_t i) const
	{
		// if (i>ones) return -1;
		return seq->select(1,i);
	}

	size_t BitSequenceRPSN::getSize() const
	{
		return seq->getSize();
	}

	BitSequenceRPSN::~BitSequenceRPSN() {
		delete seq;
	}

	void BitSequenceRPSN::save(ofstream & f) const
	{
		uint wr = RP_SN_HDR;
		saveValue(f, wr);
		seq->save(f);
	}

	BitSequenceRPSN * BitSequenceRPSN::load(ifstream & f) {
		BitSequenceRPSN *ret = NULL;
		try
		{
			ret = new BitSequenceRPSN();
			uint type = loadValue<uint>(f);
			// TODO:throw an exception!
			if(type!=RP_SN_HDR) {
				abort();
			}
			SequenceRepairSN *s = (SequenceRepairSN*)Sequence::load(f);
            ret->length = s->getLength();
			ret->seq = s;
			assert(ret->seq->getLength());
			ret->ones = ret->seq->rank(1,ret->seq->getLength()-1);
			return ret;
		}catch(...){
			delete ret;
		}
		return NULL;
	}
}
