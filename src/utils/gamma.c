#include "gamma.h"

uint logb2(uint value) {
	register uint k = 0;
	while(value != 0) {
		value = value >> 1;
		k++;
	}
	return k - 1;
}

uint encode(uint* output, uint pos, uint value) {
	value++;
	uint l = logb2(value);
	uint i;
	for(i = 0; i < l; i++) {
		bitclean(output,pos);
		pos++;
	}
	bitset(output,pos);
	pos++;
	for(i = 0; i < l; i++) {
		if(value & (1 << i))
			bitset(output,pos);
		else
			bitclean(output,pos);
		pos++;
	}

	return 2*l+1;
}

uint decode(uint* input, uint pos, uint* value) {
	uint nbits = 0;
	uint lValue = 0;
	register uint i;
	while(!bitget(input,pos)) {
		nbits++;
		pos++;
	}
	pos++;
	for(i = 0; i < nbits; i++) {
		if(bitget(input,pos))
			lValue += 1 << i;
		pos++;
	}
	lValue |= (1 << nbits);

	*value = lValue - 1;
	return 2*nbits+1;
}
