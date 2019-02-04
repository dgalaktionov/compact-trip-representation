/* wt_node_internal.h
 * Copyright (C) 2008, Francisco Claude.
 * Copyright (C) 2011, Matthias Petri.
 *
 * wt_node_internal
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

#ifndef wt_node_internal_h
#define wt_node_internal_h

#include <sequence/wt_node.h>
#include <utils/libcdsBasics.h>
#include <bitsequence/BitSequence.h>
#include <bitsequence/BitSequenceBuilder.h>
#include <cassert>

namespace cds_static
{

    /** Class for representing internal nodes
     *
     *  @author Francisco Claude
     */
    class wt_node_internal: public wt_node
    {
        public:
            wt_node_internal(uint * seq, size_t n, uint l, wt_coder * c, BitSequenceBuilder * bmb);
            wt_node_internal(uchar * seq, size_t n, uint l, wt_coder * c, BitSequenceBuilder * bmb, size_t left, uint * done);
            virtual ~wt_node_internal();
            virtual size_t rank(uint *symbol, size_t pos, uint level, wt_coder *c) const;
            virtual uint rankLessThan(uint &symbol, uint pos) const;
            //virtual size_t rankLessThan(uint &symbol, size_t pos) const;
            virtual size_t select(uint *symbol, size_t pos, uint level, wt_coder *c) const;
            virtual pair<uint,size_t> quantile_freq(size_t left,size_t right,uint q) const;
            virtual uint access(size_t pos) const;
            virtual uint access(size_t pos, size_t & rankp) const;
            virtual size_t getSize() const;
            virtual void save(ofstream & fp) const;
            virtual size_t rng(size_t xs, size_t xe, uint ys, uint ye,
                uint current, uint level, uint lefty, uint righty,
          			pair<int,int> *limits) const;
            static wt_node_internal * load(ifstream & fp);
            virtual int topKSplit(uint xs, uint xe, uint *res,
                std::priority_queue<BinTopK_Item, std::vector<BinTopK_Item>, TopK_Cmp> *topKQueue) const;

        protected:
            wt_node_internal();
            wt_node *left_child, *right_child;
            BitSequence * bitmap;

            //uint length;
    };
};
#endif
