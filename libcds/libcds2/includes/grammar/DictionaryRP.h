/*  
 * Copyright (C) 2013-current_year, Alberto Ordóñez, all rights reserved.
 *
 * Alberto Ordóñez <alberto.ordonez@udc.es>
 * 
 * Implements the idea of Tabei et al.[32] (check out the paper "Faster
 * Compressed Suffix Trees for Repeititve Text Collections". At Section 3
 * we breafly describe it). 
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

#ifndef _DICIONARYRP_H
#define	_DICIONARYRP_H

#define DICT_RP_PLAIN 33333
#define DICT_RP_TABEI 1
#include <utils/cppUtils.h>
#include <grammar/Repair.h>
namespace cds_static{
	
	class DictionaryRP{               
		public:
            DictionaryRP(int R, uint alph);
            virtual uint getAlph();
            virtual int getNRules();
            virtual ~DictionaryRP();
            virtual uint getRight(size_t rule)=0;
            virtual uint getLeft(size_t rule)=0;

            virtual size_t getSize()=0;
            virtual size_t getSizeVerbose()=0;
            virtual void save(ofstream &of)=0;
            static DictionaryRP *load(ifstream &in);
		protected: 
			DictionaryRP();
			int R;
			uint alpha;
			uint alphaPlus1;
            bool testDictionaryRP(uint *LEFT, uint *RIGHT, uint ALPH, int R, uint *PERM);
    };

};

#include<grammar/DictionaryRPPlain.h>
#include<grammar/DictionaryRPTabei.h>
#endif	

