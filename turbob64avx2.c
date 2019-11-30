/**
Copyright (c) 2016-2019, Powturbo
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    - homepage : https://sites.google.com/site/powturbo/
    - github   : https://github.com/powturbo
    - twitter  : https://twitter.com/powturbo
    - email    : powturbo [_AT_] gmail [_DOT_] com
**/
// SSE + AVX2 Based on:
// http://0x80.pl/articles/index.html#base64-algorithm-update
// https://arxiv.org/abs/1704.00605
// https://gist.github.com/aqrit/a2ccea48d7cac7e9d4d99f19d4759666 (decode)

#include <immintrin.h>
#include "conf.h"
#include "turbob64.h"

#define PREFETCH(_ip_,_i_,_rw_) __builtin_prefetch(_ip_+(_i_),_rw_)

#define DEC_RESHUFFLE(v) {\
  const __m256i merge_ab_and_bc = _mm256_maddubs_epi16(v,            _mm256_set1_epi32(0x01400140));\
                              v = _mm256_madd_epi16(merge_ab_and_bc, _mm256_set1_epi32(0x00011000));\
                              v = _mm256_shuffle_epi8(v, cpv);\
}

#define ASCII2BIN(iv,ov,shifted) { /*Convert ascii input bytes to 6-bit values*/\
                shifted    = _mm256_srli_epi32(iv, 3);\
  const __m256i delta_hash = _mm256_avg_epu8(_mm256_shuffle_epi8(delta_asso, iv), shifted);\
	                    ov = _mm256_add_epi8(_mm256_shuffle_epi8(delta_values, delta_hash), iv);\
}

#define B64CHECK(iv0,vx) {\
  const __m256i check_hash = _mm256_avg_epu8( _mm256_shuffle_epi8(check_asso, iv0),          shifted0);\
  const __m256i        chk = _mm256_adds_epi8(_mm256_shuffle_epi8(check_values, check_hash), iv0);\
                        vx = _mm256_or_si256(vx, chk);\
}

#define CHECK0(a) a
  #ifdef B64CHECK
#define CHECK1(a) a
  #else
#define CHECK1(a)
  #endif

unsigned tb64avx2dec(const unsigned char *in, unsigned inlen, unsigned char *out) {
  const unsigned char *ip = in;
        unsigned char *op = out; 
		
  __m256i vx = _mm256_setzero_si256();
  #define ND 64
  if(inlen >= ND+4) {
    const __m256i delta_asso   = _mm256_setr_epi8(0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,	  0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x0f,
	  									          0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,	  0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x0f);
    const __m256i delta_values = _mm256_setr_epi8(0x00, 0x00, 0x00, 0x13, 0x04, 0xbf, 0xbf, 0xb9,	  0xb9, 0x00, 0x10, 0xc3, 0xbf, 0xbf, 0xb9, 0xb9,
										          0x00, 0x00, 0x00, 0x13, 0x04, 0xbf, 0xbf, 0xb9,	  0xb9, 0x00, 0x10, 0xc3, 0xbf, 0xbf, 0xb9, 0xb9);
    const __m256i check_asso   = _mm256_setr_epi8(0x0d, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,   0x01, 0x01, 0x03, 0x07, 0x0b, 0x0b, 0x0b, 0x0f,
											      0x0d, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,   0x01, 0x01, 0x03, 0x07, 0x0b, 0x0b, 0x0b, 0x0f);
    const __m256i check_values = _mm256_setr_epi8(0x80, 0x80, 0x80, 0x80, 0xcf, 0xbf, 0xd5, 0xa6,   0xb5, 0x86, 0xd1, 0x80, 0xb1, 0x80, 0x91, 0x80,
										          0x80, 0x80, 0x80, 0x80, 0xcf, 0xbf, 0xd5, 0xa6,   0xb5, 0x86, 0xd1, 0x80, 0xb1, 0x80, 0x91, 0x80),
                           cpv = _mm256_set_epi8( -1, -1, -1, -1, 12, 13, 14,  8,    9, 10,  4,  5,  6,  0,  1,  2,
                                                  -1, -1, -1, -1, 12, 13, 14,  8,    9, 10,  4,  5,  6,  0,  1,  2);

    for(op = out; ip < in+(inlen-(ND+4)); ip += ND, op += (ND/4)*3) {			PREFETCH(ip,1024,0);
      __m256i          iv0 = _mm256_loadu_si256((__m256i *)ip);    
      __m256i          iv1 = _mm256_loadu_si256((__m256i *)(ip+32));    
	  __m256i ov0,shifted0; ASCII2BIN(iv0, ov0,shifted0); DEC_RESHUFFLE(ov0);
	  __m256i ov1,shifted1; ASCII2BIN(iv1, ov1,shifted1); DEC_RESHUFFLE(ov1);
      
      _mm_storeu_si128((__m128i*) op,       _mm256_castsi256_si128(ov0));
      _mm_storeu_si128((__m128i*)(op + 12), _mm256_extracti128_si256(ov0, 1));							
      _mm_storeu_si128((__m128i*)(op + 24), _mm256_castsi256_si128(ov1));
      _mm_storeu_si128((__m128i*)(op + 36), _mm256_extracti128_si256(ov1, 1));							
 
      CHECK0(B64CHECK(iv0,vx));
      CHECK1(B64CHECK(iv1,vx));
    }
  }
  unsigned rc;
  if(!(rc = tb64xdec(ip, inlen-(ip-in), op)) || _mm256_movemask_epi8(vx)) return 0;
  return (op-out)+rc; 
}

static ALWAYS_INLINE __m256i _toascii(const __m256i v) {
  __m256i vidx = _mm256_subs_epu8(v,   _mm256_set1_epi8(51));
          vidx = _mm256_sub_epi8(vidx, _mm256_cmpgt_epi8(v, _mm256_set1_epi8(25)));

  const __m256i offsets = _mm256_set_epi8(0, 0, -16, -19, -4, -4, -4, -4,   -4, -4, -4, -4, -4, -4, 71, 65,
                                          0, 0, -16, -19, -4, -4, -4, -4,   -4, -4, -4, -4, -4, -4, 71, 65);
  return _mm256_add_epi8(v, _mm256_shuffle_epi8(offsets, vidx));
}

unsigned tb64avx2enc(const unsigned char* in, unsigned inlen, unsigned char *out) {
  const unsigned char *ip; 
        unsigned char *op=out;
        unsigned      outlen = (inlen/3)*4;

  const __m256i shuf = _mm256_set_epi8(10,11, 9,10, 7, 8, 6, 7, 4,   5, 3, 4, 1, 2, 0, 1,
                                       10,11, 9,10, 7, 8, 6, 7, 4,   5, 3, 4, 1, 2, 0, 1);
  
  if(outlen >= 32+4)
    for(ip = in, op = out; op < out+(outlen-(32+4)); op += 32, ip += (32/4)*3) { 	 					PREFETCH(ip,1024,0);			
	  __m256i v = _mm256_castsi128_si256(    _mm_loadu_si128((__m128i *) ip));      
	          v = _mm256_inserti128_si256(v, _mm_loadu_si128((__m128i *)(ip+12)),1);	
			                                                            
              v  = _mm256_shuffle_epi8(v, shuf);							 // enc_reshuffle: https://arxiv.org/abs/1704.00605 p.12
      __m256i va = _mm256_mulhi_epu16(_mm256_and_si256(v, _mm256_set1_epi32(0x0fc0fc00)), _mm256_set1_epi32(0x04000040));
      __m256i vb = _mm256_mullo_epi16(_mm256_and_si256(v, _mm256_set1_epi32(0x003f03f0)), _mm256_set1_epi32(0x01000010));
              v  = _mm256_or_si256(va, vb);						
              v  = _toascii(v);
																
      _mm256_storeu_si256((__m256i*)op, v);											
    }
  tb64xenc(ip, inlen-(ip-in), op);
  return TURBOB64LEN(inlen);
}

