/* BitSequenceBG.h
   Copyright (C) 2005, Rodrigo Gonzalez, all rights reserved.

   Wrapper for a BlockGraph to works as a bitmap.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

#ifndef _STATIC_BITSEQUENCE_BLOCK_GRAPH_H
#define _STATIC_BITSEQUENCE_BLOCK_GRAPH_H

#include <utils/libcdsBasics.h>
#include <utils/BitString.h>
#include <bitsequence/BitSequence.h>
#include <sequence/BlockGraph.h>

namespace cds_static
{

    class BitSequenceBG : public BitSequence{
    protected:
        SequenceBlockGraph<uint,Sequence> *bg;
        BitSequenceBG();
    public:

        BitSequenceBG(uint *bitarray, size_t n, uint nLevels, uint blockLengthatLeaves, uint arity,
                      const string &buffer1, const string &buffer2,const string &logger_file="");

        ~BitSequenceBG();	 //destructor

        virtual bool access(const size_t i) const;
        virtual size_t rank1(const size_t i) const;
        virtual size_t select0(size_t x) const;
        virtual size_t select1(size_t x) const;
        virtual size_t getSize() const;
        /*load-save functions*/
        virtual void save(ofstream & f) const;
        static BitSequenceBG * load(ifstream & f);
    };
};
#endif
