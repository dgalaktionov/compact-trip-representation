/* LZBaseLength.h
 * Copyright (C) 2014-current_year, Alberto Ordóñez, all rights reserved.
 *
 * Streategies to obtain the base sequence from a LZ Parsing. Adds to the base those whose length is <=beta 
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

#include <sequence/LZBaseLength.h>
#include <utils/libcdsBasics.h>
using namespace std;
using namespace cds_utils;

namespace cds_static
{

	
			LZBaseLength::LZBaseLength(uint _beta){
				this->beta = _beta;
			}
			/** Destroys the Wavelet Tree */
			LZBaseLength::~LZBaseLength(){

			}
		
			LZBaseLength* LZBaseLength::setDelta(uint beta){
				this->beta = beta;
				return this;
			}

			uint LZBaseLength::computeBase(uint *input, size_t length,  SequenceLZEndBase *seq){
				uint *sequence = new uint[length];
				uint pointerBase=0;
				uint *bmpExplicits;
				createEmptyBitmap(&bmpExplicits,seq->z+1);
				uint *pBase = new uint[seq->z];
				uint nExplicit=0;
				for (uint i=0;i<seq->z;i++){
					if (seq->phraseLengths[i]>beta) continue;
					if (seq->phraseLengths[i]==0){
						bitset(bmpExplicits,i);
						sequence[pointerBase]=seq->trailing[i];
						pBase[nExplicit++]=pointerBase;
						pointerBase++; 
					}else{
						uint start, end;
						end =seq-> endings->select1((*(seq->refs))[i]+1);
						start = end-seq->phraseLengths[i]+1;
						
						bitset(bmpExplicits,i);
						for (uint j=start;j<=end;j++){
							sequence[pointerBase++]=input[j]-1;
						}
						sequence[pointerBase]=seq->trailing[i];
						pBase[nExplicit++]=pointerBase;
						pointerBase++;
					}
				}
				//TODO...the symbols must be contiguous...
				bitset(bmpExplicits,seq->z);
				seq->explicity = new BitSequenceRRR(bmpExplicits,seq->z+1,32);
				pBase[nExplicit]=pointerBase;
				// for (uint i=0;i<=nExplicit;i++){
				// 	pBase[i]=pBase[i+1]-1;//make it to point to the tail of the sequence, not to the head
				// }
				seq->pointersBase = new DAC(pBase,nExplicit);
				uint ngaps,*gaps=nullptr;
				getGaps(sequence,(size_t)pointerBase,ngaps,gaps);
				if (gaps){
					//padding the sequence
					for (uint i=0;i<ngaps;i++)
						sequence[pointerBase++]=gaps[i];
				}
				seq->base = seq->seqBuilder->build(sequence,pointerBase);

				// Sequence *seq_base = new SequenceLZEnd(sequence,pointerBase,dic, false);
				// cerr << "seq_base: " << seq_base->getSize()*8.0 / pointerBase << endl;
				pointerBase-=ngaps;
				// assert(testExtractPhrases(input));
				delete [] gaps;
				delete [] bmpExplicits;
				delete [] sequence;
				delete [] pBase;
				return 0;
		
			}
	
};
