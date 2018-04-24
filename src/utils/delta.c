#include "delta.h"

uint logb2(uint value) {
	register uint k = 0;
	while(value != 0) {
		value = value >> 1;
		k++;
	}
	return k - 1;
}

uint riceSize(uint value, uint l) {
	return value/(1 << (l+1))+1+l;
}

uint encodeGamma(uint* output, uint pos, uint value) {
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

uint decodeGamma(uint* input, uint pos, uint* value) {
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

	*value = lValue;
	return 2*nbits+1;
}



uint encodeGammaEdu(uint* output, uint pos, uint value) {

  register uint code, gammaCodePos, p;

	// p é a lonxitude en binario do valor a codificar
	code = value;
	p = 0;
	while(code) {
	  code >>= 1;
	  p++;
	}

	// Primeiro metemos os p-1 0's iniciais
	gammaCodePos = pos+p-1;

	// Agora metemos a representación binaria do valor
	code = value;
	if( ((gammaCodePos%32) + p) > 32 ) {
	  output[gammaCodePos/32] |= (code>>((gammaCodePos%32)+p-32));
	  output[gammaCodePos/32+1] = (code<<(64-(gammaCodePos%32)-p));
	} else {
	  output[gammaCodePos/32] |= (code<<(32-p-(gammaCodePos%32)));
	}

	return 2*p-1;

}


uint decodeGammaEdu(uint* input, uint pos, uint* value) {

  register uint code, p;
  register uint gammaCodePos = pos;

  code = (input[gammaCodePos/32] << (gammaCodePos%32)) |
		  ((gammaCodePos%32 != 0) * (input[gammaCodePos/32+1] >> (32-(gammaCodePos%32))));

	//printf("Codigo = %x\n", code);

  //Ahora contamos o n�mero de ceros (p) que hai nas posicions da esquerda de code
  p = 1;
  while(!(code & 0x80000000)) {
	  code <<= 1;
	  p++;
  }

	gammaCodePos += p-1;
  code = (input[gammaCodePos/32] << (gammaCodePos%32)) |
		  ((gammaCodePos%32 != 0) * (input[gammaCodePos/32+1] >> (32-(gammaCodePos%32))));
	code >>= 32-p;

  *value = code;
  return 2*p-1;

}


uint decodeGammaEduPos(uint* input, uint pointer, uint* value) {

  register uint code, p;

  code = (input[pointer/32] << (pointer%32)) |
		  ((pointer%32 != 0) * (input[pointer/32+1] >> (32-(pointer%32))));

	//printf("Codigo = %x\n", code);

  //Ahora contamos o n�mero de ceros (p) que hai nas posicions da esquerda de code
  p = 1;
  while(!(code & 0x80000000)) {
	  code <<= 1;
	  p++;
  }

  pointer += p-1;

  code = (input[pointer/32] << (pointer%32)) |
		  ((pointer%32 != 0) * (input[pointer/32+1] >> (32-(pointer%32))));
  code >>= 32-p;
  pointer +=p;

  *value = code;
  return pointer;

}





uint encodeDelta(uint* output, uint pos, uint value) {
	//value++;
	uint l = logb2(value);
	uint encodedBits = 0;
	encodedBits += encodeGamma(output, pos, l+1);
	pos += encodedBits;
	uint i;
	for(i = 0; i < l; i++) {
		if(value & (1 << i))
			bitset(output,pos);
		else
			bitclean(output,pos);
		encodedBits++;
		pos++;
	}
	return encodedBits;
}

uint decodeDelta(uint* input, uint pos, uint* value) {
	uint nbits,decodedBits;
	uint lValue = 0;
	decodedBits = decodeGamma(input, pos, &nbits);
	nbits--;
	pos += decodedBits;
	register uint i;
	for(i = 0; i < nbits; i++) {
		if(bitget(input,pos)) lValue += (1 << i);
		pos++;
	}
	lValue |= (1 << nbits);
	*value = lValue ;//- 1;
	return decodedBits+nbits;
}




uint encodeDeltaEdu(uint* output, uint pos, uint value) {
  register uint k,p,deltaCodesPos;
  register uint aux,code,codeLenght;
  deltaCodesPos=pos;

	  k = 0;
	  aux = value;
	  while(aux) {
		  aux >>= 1;
		  k++;
	  }
	  aux = k;
	  p = 0;
	  while(aux) {
		  aux >>= 1;
		  p++;
	  }

	  code = value & ((1<<(k-1))-1);
	  codeLenght = 2*p+k-2;

	  // Primeiro metemos os p-1 0's iniciais
	  deltaCodesPos += p-1;

	  // Agora metemos os p bits de k

	  if( ((deltaCodesPos%32) + p) > 32 ) {
		  output[deltaCodesPos/32] |= (k>>((deltaCodesPos%32)+p-32));
		  output[deltaCodesPos/32+1] = (k<<(64-(deltaCodesPos%32)-p));
	  } else {
		  output[deltaCodesPos/32] |= (k<<(32-p-(deltaCodesPos%32)));
	  }
	  deltaCodesPos += p;

	  // Por �ltimo metemos os k-1 bits de code (sen o 1 inicial)
	  if( ((deltaCodesPos%32) + (k-1)) > 32 ) {
		  output[deltaCodesPos/32] |= (code>>((deltaCodesPos%32)+(k-1)-32));
		  output[deltaCodesPos/32+1] = (code<<(64-(deltaCodesPos%32)-(k-1)));
	  } else {
		  output[deltaCodesPos/32] |= (code<<(32-(k-1)-(deltaCodesPos%32)));
	  }
	  deltaCodesPos += k-1;

	return codeLenght;
}

uint decodeDeltaEdu(uint* input, uint pos, uint* value) {
  register uint code,p,k;
  // Collemos o n�mero ceros iniciais
  // Po�emos o inicio do c�digo nun enteiro (code) alineado a esquerda
  // Non importa que non colla todo o c�digo, pero si temos asegurado que
  // colle p e k (k<=32 (6bits), p<=5bits)

  register uint pointer = pos;

  code = (input[pointer/32] << (pointer%32)) |
		  ((pointer%32 != 0) * (input[pointer/32+1] >> (32-(pointer%32))));

  //Ahora contamos o n�mero de ceros (p) que hai nas posicions da esquerda de code
  p = 1;
  while(!(code & 0x80000000)) {
	  code <<= 1;
	  p++;
  }

  // Ahora calculamos o numero de digitos da representacion binaria do codigo (k)
  k = code >> (32-p);

  // Actualizamos o punteiro global sobre deltaCodes
  pointer += 2*p-1;

  // Ponhemos a representacion binaria do codigo nun enteiro (code) alineado a esquerda
  code = (input[pointer/32] << (pointer%32)) |
		  ((pointer%32 != 0) * (input[pointer/32+1] >> (32-(pointer%32))));
  code = ((code >> 1) | 0x80000000) >> (32-k);
  pointer += k-1;

  *value = code;
  return pointer-pos;
}





/**************************************************************************************/
uint decodeDeltaEduPos(uint* input, uint pointer, uint* value) {
  register uint code,p,k;
  // Collemos o n�mero ceros iniciais
  // Po�emos o inicio do c�digo nun enteiro (code) alineado a esquerda
  // Non importa que non colla todo o c�digo, pero si temos asegurado que
  // colle p e k (k<=32 (6bits), p<=5bits)

  //register uint pointer = pos;

  code = (input[pointer/32] << (pointer%32)) |
		  ((pointer%32 != 0) * (input[pointer/32+1] >> (32-(pointer%32))));

  //Ahora contamos o n�mero de ceros (p) que hai nas posicions da esquerda de code
  p = 1;
  while(!(code & 0x80000000)) {
	  code <<= 1;
	  p++;
  }

  // Ahora calculamos o numero de digitos da representacion binaria do codigo (k)
  k = code >> (32-p);

  // Actualizamos o punteiro global sobre deltaCodes
  pointer += 2*p-1;

  // Ponhemos a representacion binaria do codigo nun enteiro (code) alineado a esquerda
  code = (input[pointer/32] << (pointer%32)) |
		  ((pointer%32 != 0) * (input[pointer/32+1] >> (32-(pointer%32))));
  code = ((code >> 1) | 0x80000000) >> (32-k);
  pointer += k-1;

  *value = code;
  //return pointer-pos;
  return pointer;
}
