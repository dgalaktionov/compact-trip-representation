//#include "basics.h"
//#include "math_utils.h"
#include "../utils/basics.h"  // *basics.h* //

/* encoding/decoding with gamma-codes*/
uint encodeGamma(uint* output, uint pos, uint value);
uint decodeGamma(uint* input, uint pos, uint* value);

uint encodeGammaEdu(uint* output, uint pos, uint value);
uint decodeGammaEdu(uint* input, uint pos, uint* value);
uint decodeGammaEduPos(uint* input, uint pointer, uint* value);

/* encoding/decoding with delta-codes using bitget/bitset/bitclean ops. */
uint encodeDelta(uint* output, uint pos, uint value);
uint decodeDelta(uint* input, uint pos, uint* value);

uint riceSize(uint value, uint l);

/* encoding/decoding with delta-codes. Avoids the use of
 * bitget() for decoding. Word-alignment is done prior to
 * decoding into a 32-bit integer: --> much faster decoding*/
uint encodeDeltaEdu(uint* output, uint pos, uint value);
uint decodeDeltaEdu(uint* input, uint pos, uint* value);

/* Exactly the same as decodeDeltaEdu, but returns the next
 * offset in "input" instead of the numberOfDecoded bits */
uint decodeDeltaEduPos(uint* input, uint pos, uint* value);


#define decodeDeltaEduPosMacro(input, pointer, code) \
  {register uint p,k; \
  code = (input[pointer/32] << (pointer%32)) | ((pointer%32 != 0) * (input[pointer/32+1] >> (32-(pointer%32)))); \
  p = 1; \
  while(!(code & 0x80000000)) {code <<= 1; p++;} \
  k = code >> (32-p); pointer += 2*p-1; \
  code = (input[pointer/32] << (pointer%32)) |((pointer%32 != 0) * (input[pointer/32+1] >> (32-(pointer%32)))); \
  code = ((code >> 1) | 0x80000000) >> (32-k); \
  pointer += k-1; \
  }



#define decodeGammaEduPosMacro(input, pointer, code) \
  {register uint p; \
  code = (input[pointer/32] << (pointer%32)) | ((pointer%32 != 0) * (input[pointer/32+1] >> (32-(pointer%32)))); \
  p = 1; \
  while(!(code & 0x80000000)) {code <<= 1; p++;} \
  pointer += p-1; \
  code = (input[pointer/32] << (pointer%32)) | ((pointer%32 != 0) * (input[pointer/32+1] >> (32-(pointer%32)))); \
  code >>= 32-p; \
  pointer +=p; \
}
