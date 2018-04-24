/* SequenceWTRP.cpp
 * Copyright (C) 2014, Alberto Ordóñez 
 *
 * 
 * SequenceWTRP definition
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

#include <sequence/SequenceWTRP.h>
#include <mapper/Mapper.h>
#define WTRPHeur_TD 1
#define WTRPHeur_BU 0
#define WTRPHeur_REGULAR 0
namespace cds_static
{

	ostream & operator<<(ostream &os, const terminal& p)
	{
		os << "id: " << p.id << ", len: " << p.len << ", noccs: " << p.noccs;
		return os;
	}

int comparePair (const void * a, const void * b)
{
  return ((pair<uint,int>*)b)->second - ((pair<uint,int>*)a)->second;
}

int comparePairD (const void * a, const void * b)
{
  return ((pair<uint,float>*)b)->second > ((pair<uint,float>*)a)->second;
}


#if WTRPHeur_BU
	SequenceWTRP::SequenceWTRP(uint * symbols, size_t n, int limit, BitSequenceBuilder * bmb, Mapper * am, bool deleteSymbols) : Sequence(n) {

		bmb->use();
		this->n=n;
		this->am=am;
		am->use();
		for(uint i=0;i<n;i++)
			symbols[i] = am->map(symbols[i]);
		maxCodeLen = 0;
		max_v=max_value(symbols,n);
		uint *occs = new uint[max_v+1];
		for (uint i=0;i<=max_v;i++)
			occs[i]=0;
		for (uint i=0;i<n;i++)
			occs[symbols[i]]++;

		uint **bmps = new uint*[bits(max_v)*2];
		uint *pointerLevel = new  uint[bits(max_v)*2];
		for (uint i=0;i<bits(max_v)*2;i++){
			bmps[i] = nullptr;
			pointerLevel[i] = 0;
		}
		Repair *rp = new Repair((int*)symbols,n);

		terms = new terminal*[(rp->getAlpha())];
		DisjointSet2<ufn*> *ufds = new DisjointSet2<ufn*>(rp->getAlpha()+rp->getNRules());
		for (int i=0;i<rp->getAlpha();i++){
			terms[i] = new terminal(i,occs[i]);
			ufds->makeSet(i, new ufn(terms[i]));
		}
		uint *lrs = new uint[rp->getNRules()*2];
		for (int i=0;i<rp->getNRules();i++){
			lrs[i*2] = rp->getLeft(i);
			lrs[i*2+1]=rp->getRight(i);
		}
		buildCodesBottomUp(rp->getNRules(), lrs, ufds, rp->getAlpha());

		for (uint i=0U;i<rp->getAlpha();i++){
			cerr << *(terms[i]) << endl;
		}

		BitSequenceBuilder *seqClasses = new BitSequenceBuilderRPSN(new BitSequenceBuilderRRR(33),new PermutationBuilderMRRR(5,new BitSequenceBuilderRG(10)),10,0,8u);
		uint i=0U;
		double total_bps = 0.0;
		while(bmps[i]){
			BitSequence *bmp = seqClasses->build(bmps[i],pointerLevel[i]);
			auto bps = bmp->getSize()*8.0/pointerLevel[i];
			cerr << "Size of bmp[" << i << "]: " << bps << endl;
			total_bps+=bps;
			i++;
		}
		cerr << "Total space: " << total_bps + (max_v+1)*2*32.0/length << " (including the model)" << endl;

		//buildCodesBottomUp(uint total, uint *lrs, DisjointSet2<ufn*> *ufds, uint alpha);
		//build_level(bmps,symbols,n,0,pointerLevel,factor);



		// for (uint i=0;i<4;i++){
		// 	BitSequence *bs = bmb->build(bmps[i],pointerLevel[i]);
		// 	cerr << "pointerLevel("<< i << "): "<< pointerLevel[i] << endl;
		// 	cerr << "bs("<<i<<"): " << bs->getSize()*8.0/pointerLevel[i] << endl;
		// }
//	BitSequenceBuilder *seqClasses = new BitSequenceBuilderRPSN(new BitSequenceBuilderRRR(33),new PermutationBuilderMRRR(5,new BitSequenceBuilderRG(10)),4,4,8u);
//		for (uint i=0;i<6;i++){
//			BitSequence *bmp = seqClasses->build(bmps[i],length);
//			cerr << "Size of bmp[" << i << "]: " << bmp->getSize() << endl;
//		}
//	assert(pointerLevel[0]==pointerLevel[1] && pointerLevel[0]==n);
//	assert(pointerLevel[2]==n);
//	int *pares = new int[n];
//	int *simple1 = new int[n];
//	int *simple2 = new int[n];
//	int *simple3 = new int[n];
//	int *simple4 = new int[n];
//	for (uint i=0;i<n;i++){
//		simple1[i]=bitget(bmps[0],i);
//		simple2[i]=bitget(bmps[1],i);
//		// simple3[i]=bitget(bmps[2],i);
//		// simple4[i]=bitget(bmps[3],i);
//		pares[i]=(simple1[i]<<1)|simple2[i];//<<2)|(simple3[i]<<1)|simple4[i];
//		//pares[i]=(simple1[i]<<3)|(simple2[i]<<2)|(simple3[i]<<1)|simple4[i];
//	}
//	Repair *rp_pares = new Repair((int*)pares, n);
//	cerr << "Repair pares: " << rp_pares->getSize()/(n+0.0) << endl;
//
//	Repair *rp1 = new Repair((int*)simple1, n);
//	cerr << "Repair level1: " << rp1->getSize()/(n+0.0) << endl;
//
//	Repair *rp2 = new Repair((int*)simple2, n);
//	cerr << "Repair level2: " << rp2->getSize()/(n+0.0) << endl;

		// Repair *rp3 = new Repair((int*)simple3, n);
		// cerr << "Repair level3: " << rp3->getSize()/(n+0.0) << endl;

		// Repair *rp4 = new Repair((int*)simple4, n);
		// cerr << "Repair level4: " << rp4->getSize()/(n+0.0) << endl;

		for (uint i=0;i<bits(max_v)*2;i++){
			if (bmps[i]){
				delete [] bmps[i];
			}
		}
		delete [] bmps;
		delete [] pointerLevel;
		delete [] occs;

	}
#endif
#if 0
	SequenceWTRP::SequenceWTRP(uint * symbols, size_t n, int limit, BitSequenceBuilder * bmb, Mapper * am, bool deleteSymbols) : Sequence(n) {

		bmb->use();
		this->n=n;
		this->am=am;
		am->use();
		for(uint i=0;i<n;i++)
			symbols[i] = am->map(symbols[i]);
		maxCodeLen = 0;
		max_v=max_value(symbols,n);
		uint *occs = new uint[max_v+1];
		for (uint i=0;i<=max_v;i++)
			occs[i]=0;
		for (uint i=0;i<n;i++)
			occs[symbols[i]]++;
		pair<uint,uint> *symb_occ = new pair<uint,uint>[max_v+1];
		for (uint i=0;i<=max_v;i++){
			symb_occ[i].first = i;
			symb_occ[i].second = occs[i];
		}
		qsort(symb_occ,max_v+1,sizeof(pair<uint,uint>),comparePair);
		uint **bmps = new uint*[bits(max_v)*2];
		uint *pointerLevel = new uint[bits(max_v)*2];
		for (uint i=0;i<bits(max_v)*2;i++){
			bmps[i]=NULL;
			pointerLevel[i]=0;
		}
		uint nc = 5;
		double factor = 2.0/3.0;

		pair<uint,uint> * my_codes = new pair<uint,uint>[nc];

		for (uint i=26;i<(1<<5);i++){
			cerr << i << endl;
			for (uint j=0;j<nc;j++)	{
				my_codes[j].first = symb_occ[j].first;
				my_codes[j].second = (bitget(&i,j))?1:0;
				//cerr << "my_codes.first: " << my_codes[j].first << ", second: " << my_codes[j].second << endl;
			}
			for (uint j=0;j<bits(max_v)*2;j++){
				pointerLevel[j]=0;
			}
			buildRecTopDown(symbols,n,bmb,bmps,pointerLevel,0,factor,my_codes,nc);
			// for (uint i=0;i<maxLevelTest;i++){
			// 	BitSequence *bs = bmb->build(bmps[i],pointerLevel[i]);
			// 	cerr << "bs("<<i<<"): " << bs->getSize()*8.0/pointerLevel[i] << endl;
			// }
			// assert(pointerLevel[0]==pointerLevel[1] && pointerLevel[0]==n);
			// assert(pointerLevel[2]==n);
			// int *pares = new int[n];
			int *simple1 = new int[n];
			// int *simple2 = new int[n];
			// int *simple3 = new int[n];
			// int *simple4 = new int[n];
			for (uint j=0;j<n;j++){
				simple1[j]=bitget(bmps[0],j);
				// simple2[i]=bitget(bmps[1],i);
				// simple3[i]=bitget(bmps[2],i);
				// simple4[i]=bitget(bmps[3],i);
				// pares[i]=(simple1[i]<<1)|simple2[i];//<<2)|(simple3[i]<<1)|simple4[i];
				//pares[i]=(simple1[i]<<3)|(simple2[i]<<2)|(simple3[i]<<1)|simple4[i];
			}
			for (uint j=0;j<bits(max_v)*2;j++)
				bmps[0][j]=0;
			// Repair *rp_pares = new Repair((int*)pares, n);
			// cerr << "Repair pares: " << rp_pares->getSize()/(n+0.0) << endl;

			Repair *rp1 = new Repair((int*)simple1, n);
			cerr << "Repair level1: " << rp1->getSize()/(n+0.0) << endl;
			delete [] simple1;
			// Repair *rp2 = new Repair((int*)simple2, n);
			// cerr << "Repair level2: " << rp2->getSize()/(n+0.0) << endl;
		}
		bmb->unuse();
		am->unuse();
	}
#endif
#if WTRPHeur_TD
	SequenceWTRP::SequenceWTRP(uint * symbols, size_t n, int limit, BitSequenceBuilder * bmb, Mapper * am, bool deleteSymbols) : Sequence(n) {

		bmb->use();
		this->n=n;
		this->am=am;
		am->use();
		for(uint i=0;i<n;i++)
			symbols[i] = am->map(symbols[i]);
		maxCodeLen = 0;
		max_v=max_value(symbols,n);
		uint *occs = new uint[max_v+1];
		for (uint i=0;i<=max_v;i++)
			occs[i]=0;
		for (uint i=0;i<n;i++)
			occs[symbols[i]]++;

		uint **bmps = new uint*[bits(max_v)*2];
		uint *pointerLevel = new uint[bits(max_v)*2];
		for (uint i=0;i<bits(max_v)*2;i++){
			bmps[i]=NULL;
			pointerLevel[i]=0;
		}
		double factor = 1.0/2.0;
		buildRecTopDown(symbols,n,bmb,bmps,pointerLevel,0,factor);
		BitSequenceBuilder *seqClasses = new BitSequenceBuilderRPSN(new BitSequenceBuilderRRR(33),new PermutationBuilderMRRR(5,new BitSequenceBuilderRG(10)),10,0,8u);
		uint i=0U;
		double total_bps = 0.0;
		while(bmps[i]){
			BitSequence *bmp = seqClasses->build(bmps[i],pointerLevel[i]);
			auto bps = bmp->getSize()*8.0/pointerLevel[i];
			cerr << "Size of bmp[" << i << "]: " << bps << endl;
			total_bps+=bps;
			i++;
		}
		cerr << "Total space: " << total_bps + (max_v+1)*2*32.0/length << " (including the model)" << endl;

//		for (uint i=0;i<maxLevelTest;i++){
//			cerr << "bs("<<i<<").length: " << pointerLevel[i] << endl;
//			BitSequence *bs = bmb->build(bmps[i],pointerLevel[i]);
//			cerr << "bs("<<i<<"): " << bs->getSize()*8.0/pointerLevel[i] << endl;
//		}
//		assert(pointerLevel[0]==pointerLevel[1] && pointerLevel[0]==n);
//		assert(pointerLevel[2]==n);
//		int *pares = new int[n];
//		int *simple1 = new int[n];
//		int *simple2 = new int[n];
//		int *simple3 = new int[n];
//		int *simple4 = new int[n];
//		for (uint i=0;i<n;i++){
//			simple1[i]=bitget(bmps[0],i);
//			simple2[i]=bitget(bmps[1],i);
//			simple3[i]=bitget(bmps[2],i);
//			simple4[i]=bitget(bmps[3],i);
//			//pares[i]=(simple1[i]<<1)|simple2[i];//<<2)|(simple3[i]<<1)|simple4[i];
//			pares[i]=(simple1[i]<<3)|(simple2[i]<<2)|(simple3[i]<<1)|simple4[i];
//		}
//		 SequenceBuilder *seqClasses = new SequenceBuilderSequenceRepairSC(new BitSequenceBuilderRRR(32),
//				new MapperNone(),new PermutationBuilderMRRR(5,new BitSequenceBuilderRG(10)),4,4,8u);
//
//		Sequence *s = seqClasses->build((uint*)pares,n);
//		cerr << "Repair pares: " << s->getSize()*8.0/(n+0.0) << endl;

		// Repair *rp1 = new Repair((int*)simple1, n);
		// cerr << "Repair level1: " << rp1->getSize()/(n+0.0) << endl;

		// Repair *rp2 = new Repair((int*)simple2, n);
		// cerr << "Repair level2: " << rp2->getSize()/(n+0.0) << endl;

		bmb->unuse();
		am->unuse();
	}
#endif
#if WTRPHeur_REGULAR
	//Regular constructor (the only difference with a regular WT is that slightly unbalance the tree)
 	SequenceWTRP::SequenceWTRP(uint * symbols, size_t n, int limit, BitSequenceBuilder * bmb, Mapper * am, bool deleteSymbols) : Sequence(n) {

	bmb->use();
	this->n=n;
	this->am=am;
	am->use();
	for(uint i=0;i<n;i++)
		symbols[i] = am->map(symbols[i]);
	maxCodeLen = 0;
	max_v=max_value(symbols,n);

	uint **bmps = new uint*[bits(max_v)*2];
	uint *pointerLevel = new  uint[bits(max_v)*2];
	for (uint i=0;i<bits(max_v)*2;i++){
		bmps[i] = nullptr;
		pointerLevel[i] = 0;
	}
	double factor = 2.0/3.0;
	build_level(bmps,symbols,n,0,pointerLevel,factor);


		BitSequenceBuilder *seqClasses = new BitSequenceBuilderRPSN(new BitSequenceBuilderRRR(33),new PermutationBuilderMRRR(5,new BitSequenceBuilderRG(10)),10,0,8u);
		uint i=0U;
		double total_bps = 0.0;
		while(bmps[i]){
			BitSequence *bmp = seqClasses->build(bmps[i],pointerLevel[i]);
			auto bps = bmp->getSize()*8.0/pointerLevel[i];
			cerr << "Size of bmp[" << i << "]: " << bps << endl;
			total_bps+=bps;
			i++;
		}
		cerr << "Total space: " << total_bps + (max_v+1)*2*32.0/length << " (including the model)" << endl;

//	for (uint i=0;i<maxLevelTest;i++){
//		BitSequence *bs = bmb->build(bmps[i],pointerLevel[i]);
//		cerr << "pointerLevel("<< i << "): "<< pointerLevel[i] << endl;
//		cerr << "bs("<<i<<"): " << bs->getSize()*8.0/pointerLevel[i] << endl;
//	}
//	assert(pointerLevel[0]==pointerLevel[1] && pointerLevel[0]==n);
//	assert(pointerLevel[2]==n);
//	int *pares = new int[n];
//	int *simple1 = new int[n];
//	int *simple2 = new int[n];
//	int *simple3 = new int[n];
//	int *simple4 = new int[n];
//	for (uint i=0;i<n;i++){
//		simple1[i]=bitget(bmps[0],i);
//		simple2[i]=bitget(bmps[1],i);
//		// simple3[i]=bitget(bmps[2],i);
//		// simple4[i]=bitget(bmps[3],i);
//		pares[i]=(simple1[i]<<1)|simple2[i];//<<2)|(simple3[i]<<1)|simple4[i];
//		//pares[i]=(simple1[i]<<3)|(simple2[i]<<2)|(simple3[i]<<1)|simple4[i];
//	}
//	Repair *rp_pares = new Repair((int*)pares, n);
//	cerr << "Repair pares: " << rp_pares->getSize()/(n+0.0) << endl;
//
//	Repair *rp1 = new Repair((int*)simple1, n);
//	cerr << "Repair level1: " << rp1->getSize()/(n+0.0) << endl;
//
//	Repair *rp2 = new Repair((int*)simple2, n);
//	cerr << "Repair level2: " << rp2->getSize()/(n+0.0) << endl;

	// Repair *rp3 = new Repair((int*)simple3, n);
	// cerr << "Repair level3: " << rp3->getSize()/(n+0.0) << endl;

	// Repair *rp4 = new Repair((int*)simple4, n);
	// cerr << "Repair level4: " << rp4->getSize()/(n+0.0) << endl;

	for (uint i=0;i<bits(max_v)*2;i++){
		if (bmps[i]){
			delete [] bmps[i];
		}
	}
	delete [] bmps;
	delete [] pointerLevel;


}
#endif



	bool SequenceWTRP::testMarkTerms(uint *input, uint ini, uint fin, uint *bmp, uint nbits, Repair *rp, uint *voc){
	uint total = 0;
	clearBitmap(bmp,nbits);
	for (uint i=ini;i<fin;i++){
		if (!bitget(bmp,input[i])){
			bit_set(bmp, input[i]);
			voc[total++]=input[i];
		}
	}
	sort(voc,voc+total);
	assert((total>1 && voc[0]<voc[1]) || total<=1);

	uint *new_voc = new uint[rp->getAlpha()];
	//uint new_total = markTerms(input,ini,fin,bmp,nbits,rp,new_voc);
	uint new_total = markTerms(input,ini,fin,rp,new_voc);
	assert(new_total==total);
	for (uint i=0;i<total;i++){
		assert(new_voc[i]==voc[i]);
	}
	delete [] new_voc;
	return true;
}

uint SequenceWTRP::markTerms(uint *input, uint ini, uint fin, uint *bmp, uint nbits, Repair *rp, uint *voc){
	uint total=0;
	for (uint i=ini;i<fin;i++){
		if (!bitget(bmp,input[i])){
			bit_set(bmp, input[i]);
			total++;
		}
	}
	uint prev_total=total;
	total=0;
	uint nints = nbits/W;
	for (uint i=0;i<nints;i++){
		if (bmp[i]){
			for (uint j=i*W;j<((i+1)*W);j++){
				if (bitget(bmp,j)){
					voc[total++]=j;
				}
			}
		}
		bmp[i]=0;
	}

	if (nbits%W){
		for (uint j=nbits*W;j<nbits;j++){
			if (bitget(bmp,j)){
				voc[total++]=j;
			}
		}
		bmp[nbits]=0;
	}
	assert(total==prev_total);
	assert((total>1 && voc[0]<voc[1]) || total<=1);
	return total;
}

uint SequenceWTRP::markTerms(uint *input, uint ini, uint fin, Repair *rp, uint *voc){
	
	uint *symbols = new uint[fin-ini];
	for (uint i=ini;i<fin;i++){
		symbols[i-ini]=input[i];
	}
	
	sort(symbols,symbols+(fin-ini));
	voc[0]=symbols[0];
	uint total=1;
	for (uint i=1;i<(fin-ini);i++){
		if (symbols[i]!=symbols[i-1])
			voc[total++]=symbols[i];
	}
	return total;
}

void SequenceWTRP::countOccsRules(uint rule, Repair *rp, uint *occs_rules){
	if (rule>=rp->getAlpha()){
	rule-=rp->getAlpha();
	occs_rules[rule]++;
	countOccsRules(rp->getLeft(rule),rp,occs_rules);
	countOccsRules(rp->getRight(rule),rp,occs_rules);
	}
}

void SequenceWTRP::getSymbols(uint rule, Repair *rp, uint *symbols, uint &nsymb){
	if ((int)rule<rp->getAlpha()){
		symbols[nsymb++]=rule;
	}else{
		rule-=rp->getAlpha();
		getSymbols(rp->getLeft(rule),rp,symbols,nsymb);
		getSymbols(rp->getRight(rule),rp,symbols,nsymb);
	}
}
uint SequenceWTRP::markTerms(uint rule, Repair *rp, uint *voc){
	uint *symbols = new uint[rp->getLength(rule)];
	uint nsymb=0;
	getSymbols(rule,rp,symbols,nsymb);
	sort(symbols,symbols+nsymb);
	voc[0]=symbols[0];
	uint total=1;
	for (uint i=1;i<nsymb;i++){
		if (symbols[i]!=symbols[i-1])
			voc[total++]=symbols[i];
	}
	return total;
}

bool SequenceWTRP::testIntesection(){
	uint l1[5] = {1,3,5,6,7};
	uint l2[5] = {2,3,5,8,9};
	assert(intersection(l1,l2,5,5)==2);
	assert(intersection(l1,l2,0,5)==0);
	assert(intersection(l1,l2,5,0)==0);
	assert(intersection(l1,l2+1,5,4)==2);
	return true;
}

uint SequenceWTRP::intersection(uint *voc1, uint *voc2, uint n1, uint n2){
	uint i=0,j=0,total=0;
	while(i<n1 && j<n2){
		if (voc1[i]==voc2[j]){
			total++;
			i++;j++;
		}else if (voc1[i]<voc2[j]){
			i++;
		}else{
			j++;
		}
	}	
	return total;
}

bool SequenceWTRP::testMerge(){
	uint l1[10] = {1,3,5,6,7};
	uint l2[5] = {2,3,5,8,9};
	uint n1,n2;
	n1=5;n2=5;
	mergeLists(l1,l2,n1,n2);
	assert(n1==8);
	for (uint i=1;i<n1;i++){
		assert(l1[i-1]<l1[i]);
	}
	assert(l1[0]==1);
	assert(l1[1]==2);
	assert(l1[2]==3);
	assert(l1[3]==5);
	assert(l1[n1-1]==9);
	return true;
}
/**
Merges the lists list1 and list2 skipping the repeated symbols. It stores the 
result in list1, setting n1 to the number of elements resulting from the intersection. 
Note list1 has to have enough space to store the intersection. 
*/
uint SequenceWTRP::mergeLists(uint *list1, uint *list2, uint &n1, uint n2){
	uint *list = new uint[n1+n2+1];
	uint i=0,j=0,p=0;
	list[0]=-1;
	while(i<n1 && j<n2){
		if (list1[i]<list2[j]){
			if (list1[i]!=list[p]){
				list[++p]=list1[i];
			}
			i++;
		}else if (list1[i]>list2[j]){
			if (list2[j]!=list[p]){
				list[++p]=list2[j];
			}
			j++;
		}else{
			if (list1[i]!=list[p]){
				list[++p]=list1[i];
			}
			i++;j++;
		}
	}
	while(i<n1){
		if (list1[i]!=list[p]){
			list[++p]=list1[i];
		}
		i++;
	}
	while(j<n2){
		if (list2[j]!=list[p]){
			list[++p]=list2[j];
		}
		j++;
	}
	assert(n1+n2+1>=p);
	for (uint i=0;i<p;i++)
		list1[i]=list[i+1];
	n1 = p;
	delete [] list;
	return p;
}

void SequenceWTRP::buildCodesTD(uint total, uint *lrs,Repair *rp, uint *input, uint inputLen, vector<bool> &codes){
	uint *bmpLeft;
	uint *bmpRight;
	uint *bmpMarks;
	uint *Cseen;
	uint *vocLeft = new uint[rp->getAlpha()];
	uint *vocRight = new uint[rp->getAlpha()];
	uint **voc = new uint*[rp->getNTerm()+rp->getNRules()];
	uint totalLeft=0,totalRight=0;

	for (int i=0;i<rp->getNTerm()+rp->getNRules();i++)voc[i]=NULL;
	createEmptyBitmap(&bmpMarks,rp->getNTerm()+rp->getNRules());
	createEmptyBitmap(&bmpLeft,rp->getNTerm());
	createEmptyBitmap(&bmpRight,rp->getNTerm());
	createEmptyBitmap(&Cseen,rp->getNTerm()+rp->getNRules());
	uint ini=0,fin=0;
	for (int i=0;i<rp->getNC();i++){
		if (i%100000==0)cerr << i/100000 << endl;
		fin += rp->getLength(rp->getC(i));
		if (!bitget(Cseen,rp->getC(i))){
			bit_set(Cseen, rp->getC(i));
			voc[rp->getC(i)] = new uint[rp->getLength(rp->getC(i))];
			//if (i<5)assert(testMarkTerms(input,ini,fin,bmpMarks,rp->getNTerm()+rp->getNRules(),rp,voc[rp->getC(i)]));
			//uint nterms = markTerms(input,ini,fin,bmpMarks,rp->getNTerm()+rp->getNRules(),rp,voc[rp->getC(i)]);
			uint nterms = markTerms(input,ini,fin,rp,voc[rp->getC(i)]);
			if (nterms<(double)rp->getAlpha()*2/3){
				if (totalLeft==0){
					//assert(testMerge());
					//copies voc[rep->..] into vocLeft since we say length vocLeft is 0
					mergeLists(vocLeft,voc[rp->getC(i)],totalLeft,nterms);
				}else{
					uint inter = intersection(vocLeft,voc[rp->getC(i)],totalLeft,nterms);
					if (inter+totalLeft+nterms-2*inter < (double)rp->getAlpha()*2/3){
						mergeLists(vocLeft,voc[rp->getC(i)],totalLeft,nterms);
					}else if (inter==0){
						//we can only consider the right branch if the symbols have void intersection with those in the left branch
						//try in the right branch
						inter = intersection(vocRight,voc[rp->getC(i)],totalRight,nterms);
						if (inter+totalRight+nterms-2*inter <(double)rp->getAlpha()*2/3){
							mergeLists(vocRight,voc[rp->getC(i)],totalRight,nterms);
						}
					}
				}
			}	
		}
		assert(totalLeft<(double)rp->getAlpha()*2/3);
		assert(totalRight<(double)rp->getAlpha()*2/3);	
		ini=fin;
	}
	//add those symbols not yet assigned
	codes.reserve(rp->getAlpha());
	uint pl=0,pr=0;
	uint newTotalLeft=totalLeft,newTotalRight=totalRight;
	for (uint i=0;i<(uint)rp->getAlpha();i++){
		if (vocLeft[pl]==i){
			cerr << i << ": 0" << endl;
			codes[i]=false;
			pl++;
		}else if (vocRight[pr]==i){
			cerr << i << ": 1" << endl;
			codes[i]=true;
			pr++;
		}else{
			//add i to the left or to the right
			if (totalLeft<totalRight){
				cerr << i << ": 0" << endl;
				vocLeft[newTotalLeft++]=i;
				codes[i]=0;
			}else{
				cerr << i << ": 1" << endl;
				vocRight[newTotalRight++]=i;
				codes[i]=1;
			}
		}
	}
	totalLeft=newTotalLeft;
	totalRight=newTotalRight;
	delete [] bmpLeft;
	delete [] bmpRight;
	delete [] bmpMarks;
	for (int i=0;i<rp->getAlpha();i++)
		delete [] voc[i];
	delete [] voc;
	delete [] Cseen;
	//assert(totalLeft+totalRight==(uint)rp->getAlpha());
}


/**
 * rules are stored in a pairwise way. For instance, if we have a rule A->ab, then, 
 * at some point of lrs we will have lrs[i]=a and lrs[i+1]=b. 
 * 
 * */
void SequenceWTRP::buildCodes(uint total, uint *lrs, DisjointSet<ufn*> *ufds, uint alpha,
			uint *occs, uint grammarDepth, uint &order){//, uint groupLen, uint &currentGroup, uint *leftGroup, uint *rightGroup){
		
		
		uint maxCodeLen = 0;
		//iterate over the rules making pairs and merging sets
		for (uint i=0;i<total;i++){
			uint lr[2];
			//lr[0]=left, lr[1]=right
			lr[0]  = lrs[i*2];
			lr[1] =  lrs[i*2+1];
			//find the set representatives of each symbol left/right.
			
			uint set_repr[2];;
			set_repr[0] = ufds->findSet(lr[0]);
			set_repr[1] = ufds->findSet(lr[1]);
			
			//if both symbols share their representative it is because they were already 
			//merged -> we do nothing.
			if (set_repr[0]==set_repr[1]){ 
				if (!(ufds->accessSet(i+alpha))){
					//we create a new set for the new rule containing only 
					ufn *new_rep   = new ufn(i);
					ufds->makeSet(i+alpha,new_rep);
					uint res;
					ufds->unionSet(set_repr[0],i+alpha,res);
					assert(res==set_repr[0]);
					//write down the result of the union in lrs
					lrs[i]=res;
				}
				continue;
			}
			//othewise some kind of merge has to be done
			//first count the number of symbols to be stored in the new set (the set 
			//that results from the union)
			uint nterms = 0;
			//~ uint nsymb  = 0;
			for (uint j=0;j<2;j++){
				nterms +=ufds->accessSet(set_repr[j])->getValue().content->nterm;		
			}
			
			//we allocate a new representative 
			ufn *new_repr = new ufn((uint)i+alpha,nterms);
			//if both left and right rules are closed
			uint nterm_prime = nterms;
			new_repr->nterm=0;
			for (uint j=0;j<2;j++){
				for (uint k=0;k<ufds->accessSet(set_repr[j])->getValue().content->nterm;k++){	
					terminal t(ufds->accessSet(set_repr[j])->getValue().content->term[k].id, ufds->accessSet(set_repr[j])->getValue().content->term[k].code,
								ufds->accessSet(set_repr[j])->getValue().content->term[k].len,ufds->accessSet(set_repr[j])->getValue().content->term[k].noccs);
					assert(t.len<sizeof(uint)*8*MAX_CODE_LEN_TERMINALS);
					t.len++;
					auto prevMax = maxCodeLen;
					maxCodeLen = max(maxCodeLen,t.len);
					if (prevMax!=maxCodeLen)
						cerr << "MaxCodeLen: " << maxCodeLen << endl;
					new_repr->term[new_repr->nterm++]=t;
				}
					
			}
			assert(nterm_prime==new_repr->nterm);
				//if bothe nodes are open, we merge them in a new set
			uint res;
			ufds->unionSet(set_repr[0],set_repr[1],res);
			ufds->makeSet((uint)i+alpha,new_repr,ufds->accessSet(res));
			//~ ufds->accessSet(i+alpha)->getValue().content->order = order++;
			lrs[i]=(uint)i+alpha;

		}
		cerr << "maxCodeLen: " << maxCodeLen << endl;
	}
	/**
	* Takes two sets (n1 and n2) and copy all the symbols they contain in set dest, after 
	* deleting all the symbols already contained in dest.
	**/
	void SequenceWTRP::copyTerminals(uint n1, uint n2, uint dest, DisjointSet2<ufn*> *ufds){
		if (ufds->accessSet(dest)->getValue().content->term){
			/*for (uint i=0;i<ufds->accessSet(dest)->getValue().content->nterm;i++){
				delete ufds->accessSet(dest)->getValue().content->term[i].code;
			}*/
			delete [] ufds->accessSet(dest)->getValue().content->term;		
		}
		uint nt1 = ufds->accessSet(n1)->getValue().content->nterm;
		uint nt2 = ufds->accessSet(n2)->getValue().content->nterm;
		
		ufds->accessSet(dest)->getValue().content->term = new terminal[nt1+nt2];
		ufds->accessSet(dest)->getValue().content->nterm=0;
		for (uint i=0;i<ufds->accessSet(n1)->getValue().content->nterm;i++){
			terminal t = ufds->accessSet(n1)->getValue().content->term[i];
			terminal t2(t.id,t.code,t.len,t.noccs);
			//cerr << t2 << endl;
			t2.len++;
			terms[t2.id]->len = t2.len;
			terms[t2.id]->noccs = t2.noccs;
			ufds->accessSet(dest)->getValue().content->term[ufds->accessSet(dest)->getValue().content->nterm++]=t2;
		}
		for (uint i=0;i<ufds->accessSet(n2)->getValue().content->nterm;i++){
			terminal t = ufds->accessSet(n2)->getValue().content->term[i];
			terminal t2(t.id,t.code,t.len,t.noccs);
			bit_set(t2.code, t2.len);
			//cerr << t2 << endl;
			t2.len++;
			terms[t2.id]->code = t2.code;
			terms[t2.id]->len = t2.len;
			terms[t2.id]->noccs = t2.noccs;
			ufds->accessSet(dest)->getValue().content->term[ufds->accessSet(dest)->getValue().content->nterm++]=t2;
		}
		//ufds->accessSet(dest)->getValue().content->nterm = nt1+nt2;
	}
	
	/**
	*It takes two sets and merge the symbols they contain. The codes of the symbols associated with 
	* set1 increase their lenght by one. The sets set1 and set2 are added as children of new_id.
	*/
	uint SequenceWTRP::regularMerge(uint set1, uint set2,DisjointSet2<ufn*> *ufds,ufn *new_repr){
		uint set_repr[2];
		set_repr[0]=set1;
		set_repr[1]=set2;
		for (uint j=0;j<2;j++){
				for (uint k=0;k<ufds->accessSet(set_repr[j])->getValue().content->nterm;k++){	
					ufn *c = ufds->accessSet(set_repr[j])->getValue().content;
					terminal t(c->term[k].id, c->term[k].code,c->term[k].len,c->term[k].noccs);
					assert(t.len<sizeof(uint)*8*MAX_CODE_LEN_TERMINALS);
					//the code now is associated with the possition in the DisjointSet(which actually is a tree).
					if (j==1) bit_set(t.code, t.len);
					t.len++;
					if (t.len>63) cerr << "len: " << t.len << endl;
					maxCodeLen = max(t.len,maxCodeLen);
					new_repr->term[new_repr->nterm++]=t;
				}		
			}
		//new_repr->nterm = ufds->accessSet(set_repr[0])->getValue().content->nterm+ufds->accessSet(set_repr[1])->getValue().content->nterm;
		assert(!ufds->accessSet(new_repr->id));
		ufds->makeSet(new_repr->id,new_repr);
			assert(ufds->accessSet(new_repr->id)->getNChildren()==0);
		ufds->addSet(new_repr->id,set_repr[0]);
			assert(ufds->accessSet(new_repr->id)->getNChildren()==1);
		ufds->addSet(new_repr->id,set_repr[1]);
			assert(ufds->accessSet(new_repr->id)->getNChildren()==2);
		return new_repr->id;
	}

	bool SequenceWTRP::balanceCondition(uint nt1, uint nt2){
		return (nt1<=2*nt2);
	}
	/**
		Recursively finds the appropiate branch of node in which it should paste the new subtree rooted smallerSet. After finding 
		the appropiate set (node), it applies a regularMerge and then propagates the changes when it comes back from the recursion.
	*/

	uint SequenceWTRP::finishRecursion(uint node, uint smallerSet, bool new_set_left, bool head, DisjointSet2<ufn*> *ufds, ufn *new_repr){
		if(!ufds->accessSet(node)->getParent()){
			cerr << endl;
		}
		assert(ufds->accessSet(node)->getParent());
		uint parent_id = ufds->accessSet(node)->getParent()->getValue().id;
			int parent_nchild_TEST = ufds->accessSet(node)->getParent()->getNChildren();
		if (new_set_left){
			regularMerge(smallerSet,node,ufds,new_repr);
				assert(ufds->accessSet(parent_id)->getNChildren()==parent_nchild_TEST-1);
		}else{
			regularMerge(node,smallerSet,ufds,new_repr);
				assert(ufds->accessSet(parent_id)->getNChildren()==parent_nchild_TEST-1);
		}
		if (head){
			ufds->addSetHead(ufds->accessSet(parent_id)->getValue().id,new_repr->id);
				assert(ufds->accessSet(parent_id)->getNChildren()==parent_nchild_TEST);
		}else{
			ufds->addSetTail(ufds->accessSet(parent_id)->getValue().id,new_repr->id);
				assert(ufds->accessSet(parent_id)->getNChildren()==parent_nchild_TEST);
		}
		return new_repr->id;
	}

	uint SequenceWTRP::recInsert(uint node, uint smallerSet, bool new_set_left, bool head, DisjointSet2<ufn*> *ufds, ufn *new_repr){
		
		if (ufds->accessSet(node)->getNChildren()==0){
			return finishRecursion(node,smallerSet,new_set_left,head,ufds,new_repr);		
		}
		//we add artificial nodes that has only one-child. We have to go down in the tree
		//until we find a node with more than one child (a tree-node).
		assert(ufds->accessSet(node)->getNChildren()>=2);
		uint ntnew = max(1u,ufds->accessSet(smallerSet)->getValue().content->nterm);
		uint res;
		uint nnode = ufds->accessSet(node)->getValue().content->nterm;
		//the difference in the number of elements of the new element and nnode is low. 
		if ((double)max(nnode,ntnew)/min(nnode,ntnew)<=2){
			
			res = finishRecursion(node,smallerSet,new_set_left,new_set_left,ufds,new_repr);	
				assert(ufds->accessSet(node)->getNChildren()>=2);
			
		}else{
			uint n1 = ufds->accessSet(node)->getChild(0)->getValue().content->id;
			uint n2 = ufds->accessSet(node)->getChild(1)->getValue().content->id;
			uint nt1 = ufds->accessSet(n1)->getValue().content->nterm;
			uint nt2 = ufds->accessSet(n2)->getValue().content->nterm;
			
			
			if (nt1==nt2){
				//heuristic to select the node
				if (ntnew == nt1){
					//we directly merge both trees, n1 or n2 and ntnew
					if (new_set_left){
						res = finishRecursion(n1,smallerSet,new_set_left,true,ufds,new_repr);	
							assert(ufds->accessSet(node)->getNChildren()>=2);
						copyTerminals(res,ufds->accessSet(n2)->getValue().content->id,node,ufds);
							assert(ufds->accessSet(node)->getNChildren()>=2);

					}else{
						res = finishRecursion(n2,smallerSet,new_set_left,false,ufds,new_repr);	
							assert(ufds->accessSet(node)->getNChildren()>=2);
						copyTerminals(ufds->accessSet(n1)->getValue().content->id,res,node,ufds);
							assert(ufds->accessSet(node)->getNChildren()>=2);
					}
				}else{
					if (new_set_left){
						res = recInsert(n1,smallerSet,new_set_left,true,ufds,new_repr);	
							assert(ufds->accessSet(node)->getNChildren()>=2);
						copyTerminals(res,ufds->accessSet(n2)->getValue().content->id,node,ufds);
							assert(ufds->accessSet(node)->getNChildren()>=2);

					}else{
						res = recInsert(n2,smallerSet,new_set_left,false,ufds,new_repr);	
							assert(ufds->accessSet(node)->getNChildren()>=2);
						copyTerminals(ufds->accessSet(n1)->getValue().content->id,res,node,ufds);
							assert(ufds->accessSet(node)->getNChildren()>=2);
					}
				}
				res = node;
			//}else if (abs((int)nt1+ntnew-nt2)>2)
			}else if ((nt1+ntnew)>2*nt2){
			//}else if (!balanceCondition(nt1+ntnew,nt2)){ //(nt1+ntnew)>2*nt2){
				//insert below nt2
				res = recInsert(ufds->accessSet(n2)->getValue().content->id,smallerSet,new_set_left,false,ufds,new_repr);
				copyTerminals(ufds->accessSet(n1)->getValue().content->id,res,node,ufds);
				assert(ufds->accessSet(node)->getNChildren()>=2);
				res = node;
			}else{
				res = recInsert(ufds->accessSet(n1)->getValue().content->id,smallerSet,new_set_left,true,ufds,new_repr);
				//se have to propagate upwards the changes we introduce in the children (the new symbols we added)
				copyTerminals(res,ufds->accessSet(n2)->getValue().content->id,node,ufds);
				assert(ufds->accessSet(node)->getNChildren()>=2);
				res = node;
				//insert below nt1
			}
		}
		return res;
	}
/**
*  Heuristic with the number of terminals of each subtree

**/
	void SequenceWTRP::buildCodesBottomUp(uint total, uint *lrs, DisjointSet2<ufn*> *ufds, uint alpha){
		newSets = 0;
		//iterate over the rules making pairs and merging sets
		for (uint i=0;i<total;i++){
			uint *lr = lrs+2*i;
			//cerr << "i+alpha: " << i+alpha << endl;
			//find the set representatives of each symbol left/right.
			uint set_repr[2];;
			set_repr[0] = ufds->findSet(lr[0]);
			set_repr[1] = ufds->findSet(lr[1]);
			if (set_repr[0]!=set_repr[1] && set_repr[0]!=lrs[0] && set_repr[1]!=lrs[1])
				newSets++;
			//cerr << "i+alpha: " << i+alpha << ",l0:" << lr[0] << ",l1:" << lr[1] << ",rep0:" << set_repr[0] << ",rep1:"<<set_repr[1]<< endl;
			//if both symbols share their representative it is because they were already 
			//merged -> we do nothing.
			/*if (i+alpha==8058){
				printRule(set_repr[0],ufds,0);
				printRule(set_repr[1],ufds,0);
			}*/
			if (set_repr[0]==set_repr[1]){ 
				if (ufds->accessSet(i+alpha)==NULL){
					//we create a new set for the new rule containing only 
					ufn *new_rep   = new ufn(i+alpha);	
					ufds->makeSet(i+alpha,new_rep);
					if (set_repr[0]<alpha){
						//the rules is linking two elements that doesn't belong to a set
						ufds->addSet(i+alpha,set_repr[0]);	
					}else{
						//if links to elements that already belong to a set ->we add the new rule (i+alph)
						//as a child of the set representative of lr[0]. In this way, when we find the 
						//representative of (i+alpha), set_repr[0] will be returned (note that set_repr[0] is 
						//the one that contains lr[0]).
						ufds->addSet(set_repr[0],new_rep->id);
					}
					lrs[i]=ufds->findSet(i+alpha);
				}
				//cerr << "REP: " << i+alpha << " rep: " << set_repr[0] << endl;
			}else{

				uint nterms = ufds->accessSet(set_repr[0])->getValue().content->nterm +
							  ufds->accessSet(set_repr[1])->getValue().content->nterm;		
				
				int nt1 = ufds->accessSet(set_repr[0])->getValue().content->nterm;
				int nt2 = ufds->accessSet(set_repr[1])->getValue().content->nterm;
				
				ufn *new_repr = new ufn((uint)i+alpha,nterms);
				new_repr->nterm=0;
				uint res = i+alpha;
				
				if ((double)max(nt1,nt2)/min(nt1,nt2)<=2){
					regularMerge(set_repr[0],set_repr[1],ufds,new_repr);
					assert(ufds->accessSet(i+alpha));
					//i+alpha is a new node -> it has exactly two children
					assert(ufds->accessSet(i+alpha)->getNChildren()==2);
				}else{
					if (nt1>nt2){
						res = recInsert(set_repr[0],set_repr[1],false,true,ufds,new_repr);
					}else{
						res = recInsert(set_repr[1],set_repr[0],true,false,ufds,new_repr);
					}
					if(ufds->accessSet(ufds->findSet(i+alpha))->getNChildren()<2)
						cerr << endl;
				}
				//testCodeLengths(ufds->findSet(res),ufds);
				//testConsistencyNTerms(ufds->findSet(res),ufds);
				lrs[i] = res;
			}
				/*if (i>total/2 && i<(total/2+20)){
					printRule(lrs[i],ufds,0);
					cerr << endl<<endl<<endl<<endl;
				}*/
		}
	}
	
	void SequenceWTRP::printRule(uint rule, DisjointSet2<ufn*>*ufds, uint tabs){
		if (ufds->accessSet(rule)->getNChildren()==0){
			//for (uint i=0;i<tabs;i++) cerr << "\t";	
			cerr << "Leaf: "<< rule << endl;

		}else{
			rule = getValidSet(rule,ufds);
			if (ufds->accessSet(rule)->getNChildren()<2)
				cerr << endl;
			assert(ufds->accessSet(rule)->getNChildren()>=2);
			//for (uint i=0;i<tabs;i++) cerr << "\t";
			cerr << "rule: " << rule << ", findSet: "<< ufds->findSet(rule) << endl;
			for (uint i=0;i<tabs;i++) cerr << "\t";
				cerr << "L: ";
				printRule(ufds->accessSet(rule)->getChild(0)->getValue().id,ufds,tabs+1);
			for (uint i=0;i<tabs;i++) cerr << "\t";
				cerr << "R: ";
				printRule(ufds->accessSet(rule)->getChild(1)->getValue().id,ufds,tabs+1);
		}
	}
	uint SequenceWTRP::getValidSet(uint set_repr, DisjointSet2<ufn*> *ufds){
		 if (ufds->accessSet(set_repr)->getNChildren()==1){
			return getValidSet(ufds->accessSet(set_repr)->getChild(0)->getValue().id,ufds);
		}else{
			return set_repr;
		}
	}	


	bool SequenceWTRP::testCodeLengths(uint set_repr,DisjointSet2<ufn*> *ufds){
		while (ufds->accessSet(set_repr)->getNChildren()==1){
			set_repr = ufds->accessSet(set_repr)->getChild(0)->getValue().id;
		}
		for (uint i=0;i<ufds->accessSet(set_repr)->getValue().content->nterm;i++){
			assert(ufds->accessSet(set_repr)->getValue().content->term[i].len>0);
		}
		/*for (uint i=0;i<ufds->accessSet(set_repr)->getValue().content->nterm-1;i++){
			for (uint j=i+1;j<ufds->accessSet(set_repr)->getValue().content->nterm;j++){
				assert(balanceCondition(ufds->accessSet(set_repr)->getValue().content->term[i].len,ufds->accessSet(set_repr)->getValue().content->term[j].len));
			}	
		}*/
		return true;
	}
	uint SequenceWTRP::testConsistencyNTerms(uint set_repr,DisjointSet2<ufn*> *ufds){
		if (ufds->accessSet(set_repr)->getNChildren()==0){
			return ufds->accessSet(set_repr)->getValue().content->nterm;
		}else if (ufds->accessSet(set_repr)->getNChildren()==1){
			return testConsistencyNTerms(ufds->accessSet(set_repr)->getChild(0)->getValue().content->id,ufds);
		}else{
			uint l,r;
			l = testConsistencyNTerms(ufds->accessSet(set_repr)->getChild(0)->getValue().id,ufds);
			r = testConsistencyNTerms(ufds->accessSet(set_repr)->getChild(1)->getValue().id,ufds);
			assert( ufds->accessSet(set_repr)->getValue().content->nterm == (l+r) );
			if ((double)max(l,r)/min(l,r)>2){
				cerr << (double)max(l,r)/min(l,r) << endl;
				cerr << endl;
			}
			assert((double)max(l,r)/min(l,r)<=2);
			return l+r;
		}
		
	}
	uint SequenceWTRP::testCountLeaves(uint rule, DisjointSet2<ufn*>*ufds){
		if (ufds->accessSet(rule)->getNChildren()==0){
			return 1;
		}
		if (ufds->accessSet(rule)->getNChildren()<2){
			return 0;
		}
		return testCountLeaves(ufds->accessSet(rule)->getChild(0)->getValue().id,ufds)+testCountLeaves(ufds->accessSet(rule)->getChild(1)->getValue().id,ufds);
	}

	uint SequenceWTRP::getMaxCodeLength(uint rule, DisjointSet2<ufn*>*ufds){
		if (ufds->accessSet(rule)->getNChildren()==0){
			return 0;
		}
		if (ufds->accessSet(rule)->getNChildren()<2){
			return 0;
		}
		return 1+max(getMaxCodeLength(ufds->accessSet(rule)->getChild(0)->getValue().id,ufds),getMaxCodeLength(ufds->accessSet(rule)->getChild(1)->getValue().id,ufds));
	}
	void SequenceWTRP::getListCodesDFS(uint rule, DisjointSet2<ufn*>*ufds, uint *&list){
		if (ufds->accessSet(rule)->getNChildren()==0){
			list[0]=ufds->accessSet(rule)->getValue().id;
			list++;
			return;
		}
		if (ufds->accessSet(rule)->getNChildren()<2){
			return ;
		}
		getListCodesDFS(ufds->accessSet(rule)->getChild(0)->getValue().id,ufds,list);
		getListCodesDFS(ufds->accessSet(rule)->getChild(1)->getValue().id,ufds,list);
		return;
	} 

	void SequenceWTRP::getListCodes(uint rule, DisjointSet2<ufn*>*ufds, tuple *&list, uint &code, uint level){
		if (ufds->accessSet(rule)->getNChildren()==0){
			list[0].symbol = ufds->accessSet(rule)->getValue().id;
			list[0].code = code;
			list[0].len = level;
			list++;
			return;
		}
		if (ufds->accessSet(rule)->getNChildren()<2){
			return ;
		}
		bitclean(&code,level);
		getListCodes(ufds->accessSet(rule)->getChild(0)->getValue().id,ufds,list,code,level+1);
		bit_set(&code, level);
		getListCodes(ufds->accessSet(rule)->getChild(1)->getValue().id,ufds,list,code,level+1);
		bitclean(&code,level);
		return;
	}

	void SequenceWTRP::buildCodesTD(uint *rules, uint total,Repair *rp, uint *input, uint inputLen, vector<bool> &codes, double factor){
	

	uint *vocLeft = new uint[rp->getAlpha()];
	uint *vocRight = new uint[rp->getAlpha()];

	uint totalLeft=0,totalRight=0;
	
	uint max_v=0;
	for (uint i=0;i<inputLen;i++) {
		max_v = max(max_v,input[i]);
	}
	vector<uint> almostHitsL(max_v+1,0);
	vector<uint> almostHitsR(max_v+1,0);


	double limLeft, limRight;
	limLeft = rp->getAlpha()*factor;
	limRight = limLeft;//ceil(rp->getAlpha()*(1-factor));

	uint **voc = new uint*[rp->getNTerm()+rp->getNRules()];
	for (int i=0;i<rp->getNTerm()+rp->getNRules();i++)voc[i]=nullptr;

	vector<bool> bmpMarks(rp->getNTerm()+rp->getNRules(),false);
	vector<bool> seen(rp->getNTerm()+rp->getNRules(),false);
	for (uint i=0;i<total;i++){
		if (!seen[rules[i]]){
			seen[rules[i]]=true;
			//cerr << "Rule: "<< rules[i]<< endl;
			// uint r;
			// r = (rules[i]>=rp->getAlpha())?(rules[i]-rp->getAlpha()):rules[i];
			//cerr << "\t"<<"L: " << rp->getLeft(r) << endl;
			//cerr << "\t"<<"R: " << rp->getRight(r) << endl;
			voc[rules[i]] = new uint[rp->getLength(rules[i])];
			//if (i<5)assert(testMarkTerms(input,ini,fin,bmpMarks,rp->getNTerm()+rp->getNRules(),rp,voc[rules[i]]));
			//uint nterms = markTerms(input,ini,fin,bmpMarks,rp->getNTerm()+rp->getNRules(),rp,voc[rules[i]]);
			uint nterms = markTerms(rules[i],rp,voc[rules[i]]);
			if (nterms<limLeft){//2/3){
				if (totalLeft==0){
					//assert(testMerge());
					//copies voc[rep->..] into vocLeft since we say length vocLeft is 0
					mergeLists(vocLeft,voc[rules[i]],totalLeft,nterms);
				}else{
					uint interR = intersection(vocRight,voc[rules[i]],totalRight,nterms);
					if (interR==0){
						//if it has no intersection with the right branch it could be pasted in the left one (if it meets the balance rule)
						uint inter = intersection(vocLeft,voc[rules[i]],totalLeft,nterms);
						if(inter+totalLeft+nterms-2*inter < limLeft){
							mergeLists(vocLeft,voc[rules[i]],totalLeft,nterms);
						}else{
							for (uint k=0;k<nterms;k++){
								almostHitsL[voc[rules[i]][k]]++;
							}
						}
					}else{
						uint inter = intersection(vocLeft,voc[rules[i]],totalLeft,nterms);
						if (inter==0){
							//we can only consider the right branch if the symbols have void intersection with those in the left branch
							//try in the right branch
							inter = intersection(vocRight,voc[rules[i]],totalRight,nterms);
							if (inter+totalRight+nterms-2*inter < limRight){
								mergeLists(vocRight,voc[rules[i]],totalRight,nterms);
							}else{	
								for (uint k=0;k<nterms;k++){
									almostHitsR[voc[rules[i]][k]]++;
								}
							}
						}
					}
				}
			}	
		}
		assert(totalLeft<=limLeft);
		assert(totalRight<=limRight);
	}
	//add those symbols not yet assigned
	codes.reserve(rp->getAlpha());
		vector<bool> bmpVoc(rp->getAlpha(),false);
	for (uint i=0;i<totalLeft;i++){
		codes[vocLeft[i]]=false;
		bmpVoc[vocLeft[i]]=true;
	}
	for (uint i=0;i<totalRight;i++){
		codes[vocRight[i]]=true;
		bmpVoc[vocRight[i]]=true;
	}
	for (uint i=0;i<(uint)rp->getAlpha();i++){
		if (!bmpVoc[i]){
			// if (almostHitsL[i]>almostHitsR[i]){
			// 	vocLeft[totalLeft++]=i;
			// 	codes[i]=0;
			// }else{
			// 	vocRight[totalRight++]=i;
			// 	codes[i]=0;
			// }

			if (totalLeft < limLeft || totalLeft<totalRight){
				vocLeft[totalLeft++]=i;
				codes[i]=0;
			}else{
				vocRight[totalRight++]=i;
				codes[i]=1;
			}
		}
	}
	for (int i=0;i<rp->getAlpha();i++)
		delete [] voc[i];
	delete [] voc;

	assert(totalLeft+totalRight==(uint)rp->getAlpha());
}

	
	void SequenceWTRP::buildRecTopDown(uint *symbols, size_t inputLen, BitSequenceBuilder * bmb, uint **bmps, uint *pointerLevel, uint level, double factor){
		if (inputLen==0) return;
		//if (level==8) return;
		uint sigma=0;
		Mapper *mapper = new MapperCont((const uint*)symbols,(size_t)inputLen, BitSequenceBuilderRG(32));
		mapper->use();
		uint first = symbols[0];
		bool should_continue = false;
		for (uint i=0;i<inputLen;i++){
			symbols[i]=mapper->map(symbols[i])-1;
			sigma=max(sigma,symbols[i]);
			should_continue |= (symbols[i]!=first);
		}
		mapper->unuse();
		if (!should_continue) 
			return;
		cerr << "level:" << level << endl;
		cerr << "|sigma|:" << sigma+1 << endl;
		Repair *rp = new Repair((int*)symbols,inputLen);

		//SORT BY NOCCS RULE
		// uint *occs_rules = new uint[rp->getNRules()];
		// for (uint i=0;i<rp->getNRules();i++){
		// 	occs_rules[i]=0;
		// }
		// for (uint i=0;i<rp->getNRules();i++){
		// 	countOccsRules(i+rp->getAlpha(), rp, occs_rules);
		// }
		// for (uint i=0;i<rp->getNRules();i++){
		// 	assert(occs_rules[i]);
		// }
		// pair<uint,int> *_rules = new pair<uint,int>[rp->getNRules()];
		// for (int i=0;i<rp->getNRules();i++){
		// 	_rules[i].first = i;
		// 	_rules[i].second = occs_rules[i];
		// }
		// qsort(_rules,rp->getNRules(),sizeof(pair<int,int>),comparePair);
		// assert((rp->getNRules()>=2 && _rules[0].second > _rules[1].second) || rp->getNRules()<2);


		//SORT BY RULE LENGTH
		pair<uint,int> *_rules = new pair<uint,int>[rp->getNRules()];
		for (int i=0;i<rp->getNRules();i++){
			_rules[i].first = i;
			_rules[i].second = rp->getLength(i+rp->getAlpha());
		}
		qsort(_rules,rp->getNRules(),sizeof(pair<int,int>),comparePair);
		if (!((rp->getNRules()>=2 && _rules[0].second >= _rules[1].second) || rp->getNRules()<2)){
			cerr << endl;
		}
		assert((rp->getNRules()>=2 && _rules[0].second >= _rules[1].second) || rp->getNRules()<2);


		uint *rules = new uint[rp->getNRules()];
		for (int i=0;i<rp->getNRules();i++)rules[i]=_rules[i].first;
		delete [] _rules;
		vector<bool> *codes = new vector<bool>();
		buildCodesTD(rules,rp->getNRules(),rp,symbols,inputLen,*codes,factor);
		
		delete [] rules;
		delete rp;
		uint toLeft=0,toRight=0;
		if (bmps[level]==NULL){
			createEmptyBitmap(bmps+level,n);
		}
		for (size_t i=0;i<inputLen;i++){
			if ((*codes)[symbols[i]]){
				bit_set(bmps[level], pointerLevel[level]);
				toRight++;
			}else{
				toLeft++;
			}
			pointerLevel[level]++;
		}
		uint *symbolsLeft = new uint[toLeft];
		uint *symbolsRight = new uint[toRight];
		for (size_t i=0;i<inputLen;i++){
			if ((*codes)[symbols[i]]){
				symbolsRight[0]=symbols[i];
				symbolsRight++;
			}else{
				symbolsLeft[0]=symbols[i];
				symbolsLeft++;
			}
		}
		symbolsLeft-=toLeft;
		symbolsRight-=toRight;
		assert(toLeft+toRight==inputLen);
		buildRecTopDown(symbolsLeft,toLeft,bmb,bmps,pointerLevel,level+1,factor);
		buildRecTopDown(symbolsRight,toRight,bmb,bmps,pointerLevel,level+1,factor);
		
	}


	SequenceWTRP::SequenceWTRP():Sequence(0) {
		bitstring = NULL;
		// occ = NULL;
		am = NULL;
	}

	SequenceWTRP::~SequenceWTRP() {
		//~ if(bitstring) {
			//~ for(uint i=0;i<height;i++)
				//~ if(bitstring[i])
					//~ delete bitstring[i];
			//~ delete [] bitstring;
		//~ }
		// if(occ)
		// 	delete occ;
		if(am)
			am->unuse();
		//~ delete [] C;
	}

	void SequenceWTRP::save(ofstream & fp) const
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

	SequenceWTRP * SequenceWTRP::load(ifstream & fp) {
		uint rd = loadValue<uint>(fp);
		if(rd!=WVMATRIX_HDR) return NULL;
		SequenceWTRP * ret = new SequenceWTRP();
		ret->n = loadValue<size_t>(fp);
		ret->length = ret->n;
		ret->max_v = loadValue<uint>(fp);
		ret->height = loadValue<uint>(fp);
		ret->C = loadValue<uint>(fp, ret->height);
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

	bool SequenceWTRP::is_set(uint val, uint ind) const
	{
		assert(ind<height);
		return (val & (1<<(height-ind-1)))!=0;
	}

	uint SequenceWTRP::set(uint val, uint ind) const
	{
		assert(ind<=height);
		return val | (1<<(height-ind-1));
	}

	uint SequenceWTRP::access(size_t pos) const
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

	size_t SequenceWTRP::rank(uint symbol, size_t pos) const
	{
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
	
	size_t SequenceWTRP::select(uint symbol, size_t j) const
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


	size_t SequenceWTRP::getSize() const
	{
		size_t ptrs = sizeof(SequenceWTRP)+height*sizeof(Sequence*);
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


	void SequenceWTRP::build_level(uint **bm, uint *symbols, uint length, uint level, uint *sizeBitmaps, tuple *codes) {
	
		if (length>0 && (codes[symbols[0]].len) == level){
			 delete [] symbols;
			 return;
		 }
		uint left=0,right=0;
		for (uint i=0;i<length;i++){
			if ((codes[symbols[i]].len <= level)) continue;

			if (bitget(&(codes[symbols[i]].code), level) ) {
				bit_set(bm[level], sizeBitmaps[level]);
				right++;
			}else{
				left++;
			}
			sizeBitmaps[level]++;
		} 
		if (!(left || right)){
			delete [] symbols;
			return;
		}
		assert(sizeBitmaps[level]<=nCodesLevel[level]);
		uint *symbLeft, *symbRight, pl=0, pr=0;

		symbLeft = new uint[left];
		symbRight = new uint[right];
		
		for (uint i=0;i<length;i++){
			if ((codes[symbols[i]].len <= level)) continue;
			if (bitget(&(codes[symbols[i]].code), level) ) {
				symbRight[pr++] = symbols[i];
			}else{
				symbLeft[pl++] = symbols[i];			
			}
		}
		assert(pl==left);
		assert(pr==right);
		
		delete [] symbols;
		build_level(bm,symbLeft,left,level+1,sizeBitmaps,codes);
		build_level(bm,symbRight,right,level+1,sizeBitmaps,codes);

	}

	void SequenceWTRP::build_level(uint **bm, uint *symbols, uint length, uint level, uint *sizeBitmaps, double factor) {
		if (level==maxLevelTest) return;
		
	     Mapper *mapper = new MapperCont(symbols,length, BitSequenceBuilderRG(32));
	     for (uint i=0;i<length;i++){
	     	symbols[i] = mapper->map(symbols[i])-1;
	     }
	     delete mapper;
		 uint sigma = symbols[0];
		 for (uint i=0;i<length;i++){
		 	sigma = max(sigma,symbols[i]);
		 }

		 if (!bm[level]){
		 	createEmptyBitmap(bm+level,n);
		 }
		if (length==0){
			delete [] symbols;
			return;
		}

		uint limit = static_cast<uint>(factor*sigma);
		size_t left,right;
		left = 0UL;right=0UL;
		for (uint i=0;i<length;i++){
			if (symbols[i]<limit){
				left++;
			}else{
				right++;
				bit_set(bm[level], sizeBitmaps[level]);
			}
			sizeBitmaps[level]++;
		}
		uint *symbLeft = new uint[left];
		uint *symbRight =new uint[right ];
		left = 0UL;right=0UL;
		for (size_t i=0UL;i<length;i++){
			if (symbols[i]<limit){
				symbLeft[left++]=symbols[i];
			}else{
				symbRight[right++]=symbols[i];
			}
		}
		delete [] symbols;
		build_level(bm,symbLeft,left,level+1,sizeBitmaps,factor);
		build_level(bm,symbRight,right,level+1,sizeBitmaps,factor);

	}
		
	uint SequenceWTRP::max_value(uint *symbols, size_t n) {
		uint max_v = 0;
		for(size_t i=0;i<n;i++)
			max_v = max(symbols[i],max_v);
		return max_v;
	}

	uint SequenceWTRP::bits(uint val) {
		uint ret = 0;
		while(val!=0) {
			ret++;
			val >>= 1;
		}
		return ret;
	}
	
	bool SequenceWTRP::statistics(DisjointSet2<ufn*> *ufds, Repair *rp, uint sigma, uint limitSets, uint *occs){
		uint nsets = 0;
		uint totalSymbolsRepair = rp->getTermsSize()+rp->getNTerm();
		for (uint i=rp->getTermsSize();i<totalSymbolsRepair;i++)
			if (ufds->accessSet(i)) nsets++;
		assert(nsets<=limitSets);
		
		uint totalOccsAll=0;
		uint totalSymbolsAll=0;
		double h0_rem = 0.0;
		uint totalRem = 0;
		uint nRem = 0;
		double h0_rep=0.0;
		
		for (int i=0;i<rp->getTermsSize();i++){
				uint res = ufds->findSet(i);
				if ((int)res==i){
					nRem++;
					assert(ufds->accessSet(i)->getValue().content->term[0].noccs>0);
					totalRem+=ufds->accessSet(i)->getValue().content->term[0].noccs;
				}else{
					totalOccsAll+=ufds->accessSet(i)->getValue().content->term[0].noccs;
				}
			
		}
		//now compute the entropy of the remainin symbols
		for (int i=0;i<rp->getTermsSize();i++){
				uint res = ufds->findSet(i);
				if ((int)res==i){
					
					double pi = (double)occs[i]/totalRem;
					assert(pi!=0);
					h0_rem += pi * log (pi) / log(2);
				}else{
					double pi = (double)occs[i]/totalOccsAll;
					assert(pi!=0);
					h0_rep+= pi * log (pi) / log(2);
				}
			
		}
		cerr << "Entropy of remaining symbols: " << h0_rem << endl;
		cerr << "Entropy of rep symbols: " << h0_rep << endl;
		cerr << "log2(nrem): " << bits(nRem) << endl;
		cerr << "log2(rep): " << bits(rp->getTermsSize()-nRem) << endl;
		
		totalOccsAll=0;
		for (uint i=rp->getTermsSize();i<totalSymbolsRepair;i++){
			if (ufds->accessSet(i)){
				uint res = ufds->findSet(i);
				if (res!=i) continue;
				uint totalOccs=0;
				for (uint j=0;j<ufds->accessSet(i)->getValue().content->nterm;j++){
					cerr << "id: " << ufds->accessSet(i)->getValue().content->term[j].id <<
					", code: " << ufds->accessSet(i)->getValue().content->term[j].code << 
					", len: " << ufds->accessSet(i)->getValue().content->term[j].len << 
					",nocc: " << ufds->accessSet(i)->getValue().content->term[j].noccs << endl;		
					totalOccs+=ufds->accessSet(i)->getValue().content->term[j].noccs;
				}
				totalOccsAll+=totalOccs;
				totalSymbolsAll+=ufds->accessSet(i)->getValue().content->nterm;
				cerr << "totalRule/n (%)=" << (double)totalOccs/n*100 << endl;
				cerr << "TotalSymbols: " << ufds->accessSet(i)->getValue().content->nterm << endl;
			}
		}
		cerr << "totalRule/n (%)=" << (double)totalOccsAll/n*100 << endl;
		cerr << "totalSymbols= " << (double)totalSymbolsAll/max_v*100 << endl;
		return true;
	}

	void SequenceWTRP::buildRecTopDown(uint *symbols, size_t inputLen, BitSequenceBuilder * bmb, uint **bmps, uint *pointerLevel, uint level, double factor, pair<uint,uint> *my_codes, uint nc){
		if (maxLevelTest==level) return;
		uint sigma=0;
		Mapper *mapper = new MapperCont((const uint*)symbols,(size_t)inputLen, BitSequenceBuilderRG(32));
		mapper->use();
		for (uint i=0;i<inputLen;i++){
			symbols[i]=mapper->map(symbols[i])-1;
			sigma=max(sigma,symbols[i]);
		}
		mapper->unuse();
		cerr << "level:" << level << endl;
		cerr << "|sigma|:" << sigma+1 << endl;
		Repair *rp = new Repair((int*)symbols,inputLen);
		vector<bool> bmpMarks(rp->getNTerm()+rp->getNRules(),false);
		//SORT BY NOCCS RULE

		// uint *occs_rules = new uint[rp->getNRules()];
		// for (uint i=0;i<rp->getNRules();i++){
		// 	occs_rules[i]=0;
		// }
		// for (uint i=0;i<rp->getNRules();i++){
		// 	countOccsRules(i+rp->getAlpha(), rp, occs_rules);
		// }
		// for (uint i=0;i<rp->getNRules();i++){
		// 	assert(occs_rules[i]);
		// }
		// pair<uint,int> *_rules = new pair<uint,int>[rp->getNRules()];
		// for (int i=0;i<rp->getNRules();i++){
		// 	_rules[i].first = i;
		// 	_rules[i].second = occs_rules[i];
		// }
		// qsort(_rules,rp->getNRules(),sizeof(pair<int,int>),comparePair);
		// assert((rp->getNRules()>=2 && _rules[0].second > _rules[1].second) || rp->getNRules()<2);

		//SORT BY RULE LENGTH
		pair<uint,int> *_rules = new pair<uint,int>[rp->getNRules()];
		for (int i=0;i<rp->getNRules();i++){
			_rules[i].first = i;
			_rules[i].second = rp->getLength(i+rp->getAlpha());
		}
		qsort(_rules,rp->getNRules(),sizeof(pair<int,int>),comparePair);
		assert((rp->getNRules()>=2 && _rules[0].second > _rules[1].second) || rp->getNRules()<2);

		uint *rules = new uint[rp->getNRules()];
		for (int i=0;i<rp->getNRules();i++)
			rules[i]=_rules[i].first;
		delete [] _rules;
		vector<bool> *codes = new vector<bool>();
		buildCodesTD(rules,rp->getNRules(),rp,symbols,inputLen,*codes,factor);
		for (uint i=0;i<nc;i++){
			(*codes)[my_codes[i].first]=((my_codes[i].second==0)?false:true);
		}
		delete [] rules;
		delete rp;
		if (bmps[level]==nullptr){
			createEmptyBitmap(bmps+level,n);
		}
		vector<uint> symbolsLeft;
		vector<uint> symbolsRight;
		for (size_t i=0;i<inputLen;i++){
			if ((*codes)[symbols[i]]){
				bit_set(bmps[level], pointerLevel[level]);
				symbolsRight.push_back(symbols[i]);
			}else{
				symbolsLeft.push_back(symbols[i]);
			}
			pointerLevel[level]++;
		}
		delete codes;
		assert(symbolsLeft.size()+symbolsRight.size()==inputLen);
		buildRecTopDown(&symbolsLeft[0],symbolsLeft.size(),bmb,bmps,pointerLevel,level+1,factor,my_codes,nc);
		buildRecTopDown(&symbolsRight[0],symbolsRight.size(),bmb,bmps,pointerLevel,level+1,factor,my_codes,nc);

	}


};
