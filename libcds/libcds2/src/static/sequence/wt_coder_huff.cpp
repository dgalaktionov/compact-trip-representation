/* wt_coder_huff.cpp
 * Copyright (C) 2008, Francisco Claude, all rights reserved.
 *
 * wt_coder_huff definition
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

#include <sequence/wt_coder_huff.h>

namespace cds_static
{

    wt_coder_huff::wt_coder_huff(const Array & a, Mapper * am) {
      am->use();
        size_t n = a.getLength();
        uint * symbs = new uint[n];
        for(uint i=0;i<n;i++)
            symbs[i] = am->map(a[i]);

        sigma = 0;
        for (int i=0;i<a.getLength();i++)
            sigma=max(sigma,a[i]);
        hc = new HuffmanCoder(symbs, n);
        maxBuffer = hc->maxLength()/W+1;
        delete [] symbs;
      am->unuse();
    }
	//returns the length of the code and stores in code the code of symbol
	uint wt_coder_huff::getCode(uint symbol, uint &code) const{
	  code = 0;
	  return (uint)hc->encode(symbol, &code, (ulong)0);
	}

    wt_coder_huff::wt_coder_huff(uint * symbs, size_t n, Mapper * am) {
      am->use();
        sigma = 0;
        for (int i=0;i<n;i++)
            sigma=max(sigma,symbs[i]);
        for(uint i=0;i<n;i++)
            symbs[i] = am->map(symbs[i]);

        sigma = 0;
        for (int i=0;i<n;i++)
            sigma=max(sigma,symbs[i]);
        hc = new HuffmanCoder(symbs, n);
        maxBuffer = hc->maxLength()/W+1;
        for(uint i=0;i<n;i++)
            symbs[i] = am->unmap(symbs[i]);
      am->unuse();
    }

    wt_coder_huff::wt_coder_huff(uchar * symbs, size_t n, Mapper * am) {
      am->use();

        for(uint i=0;i<n;i++)
            symbs[i] = (uchar)am->map((uint)symbs[i]);

        sigma = 0;
        for (int i=0;i<n;i++)
            sigma=max(sigma,(uint)symbs[i]);

        hc = new HuffmanCoder(symbs, n);
        maxBuffer = hc->maxLength()/W+1;

        for(uint i=0;i<n;i++)
            symbs[i] = (uchar)am->unmap((uint)symbs[i]);
      am->unuse();
    }

    wt_coder_huff::wt_coder_huff(uchar * symbs, size_t n, GenericMapper<uchar> * am) {
//        am->use();

        for(uint i=0;i<n;i++)
            symbs[i] = (uchar)am->map(symbs[i]);

        sigma = 0;
        for (int i=0;i<n;i++)
            sigma=max(sigma,(uint)symbs[i]);

        hc = new HuffmanCoder(symbs, n);
        maxBuffer = hc->maxLength()/W+1;

        for(uint i=0;i<n;i++)
            symbs[i] = (uchar)am->unmap(symbs[i]);
//        am->unuse();
    }

    wt_coder_huff::wt_coder_huff(uint * freqs, size_t sigma) {
        hc = HuffmanCoder::getFromFreqs(freqs, sigma);
        maxBuffer = hc->maxLength()/W+1;
    }

    wt_coder_huff::wt_coder_huff() {}

    wt_coder_huff::~wt_coder_huff() {
        delete hc;
    }

  uint * wt_coder_huff::get_symbol(uint symbol) {
    uint * ret = new uint[maxBuffer];
    hc->encode(symbol, ret, (size_t)0);
    return ret;
  }

    uint wt_coder_huff::getSymbol(uint code) const{
        uint ret;
        hc->encode(code, &ret, (size_t)0);
        return ret;
    }
                                 // TODO: it could be implemented in a more efficient (low level) way
    bool wt_coder_huff::is_set(uint symbol, uint l) const
    {
        uint buffer[maxBuffer];
        hc->encode(symbol, buffer, (size_t)0);
        return bitget(buffer,l);
    }

    bool wt_coder_huff::is_set(uint *symbol, uint l) const {
        return bitget(symbol,l);
    }

    bool wt_coder_huff::done(uint symbol, uint l) const
    {
        uint buffer[maxBuffer];
        uint s_len = (uint)hc->encode(symbol, buffer, (size_t)0);
        return l==s_len;
    }
    uint wt_coder_huff::getMaxv() const {
        return hc->getMaxv();
    }

    size_t wt_coder_huff::getSize() const
    {
        return 2*sizeof(uint)+sizeof(wt_coder_huff)+hc->getSize()+(hc->maxLength()/W+1)*sizeof(uint);
    }

    void wt_coder_huff::save(ofstream & fp) const
    {
        uint wr = WT_CODER_HUFF_HDR;
        saveValue(fp,wr);
        hc->save(fp);
    }

    wt_coder_huff * wt_coder_huff::load(ifstream & fp) {
        uint rd = loadValue<uint>(fp);
        if(rd!=WT_CODER_HUFF_HDR) return NULL;
        wt_coder_huff * ret = new wt_coder_huff();
        ret->hc = HuffmanCoder::load(fp);
        assert(ret->hc!=NULL);
        ret->maxBuffer = ret->hc->maxLength()/W+1;
        return ret;
    }
};
