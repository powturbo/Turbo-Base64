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

// http://0x80.pl/articles/index.html#base64-algorithm-update
// https://arxiv.org/abs/1704.00605

#include <immintrin.h>
#include "conf.h"
#include "turbob64.h"

#define PREFETCH(_ip_,_i_,_rw_) __builtin_prefetch(_ip_+(_i_),_rw_)

unsigned tb64avx2dec(const unsigned char *in, unsigned inlen, unsigned char *out) {
  const unsigned char *ip=in;
        unsigned char *op=out; 
		
  const __m256i lut_lo   = _mm256_set_epi8(26, 27, 27, 27, 26, 19, 17, 17,   17, 17, 17, 17, 17, 17, 17, 21,
									       26, 27, 27, 27, 26, 19, 17, 17,   17, 17, 17, 17, 17, 17, 17, 21),
                lut_hi   = _mm256_set_epi8(16, 16, 16, 16, 16, 16, 16, 16,    8,  4,  8,  4,  2,  1, 16, 16,
										   16, 16, 16, 16, 16, 16, 16, 16,    8,  4,  8,  4,  2,  1, 16, 16),
                lut_roll = _mm256_set_epi8( 0,  0,  0,  0,  0,  0,  0,  0,  -71,-71,-65,-65,  4, 19, 16,  0,
											0,  0,  0,  0,  0,  0,  0,  0,  -71,-71,-65,-65,  4, 19, 16,  0),
                cpv      = _mm256_set_epi8(-1, -1, -1, -1, 12, 13, 14,  8,    9, 10,  4,  5,  6,  0,  1,  2,
                                           -1, -1, -1, -1, 12, 13, 14,  8,    9, 10,  4,  5,  6,  0,  1,  2), 
                cv_2f    = _mm256_set1_epi8(0x2f);
				
  if(inlen >= 32+4) {
	const unsigned char *ie = in+(inlen-(32+4));
    for(op = out; ip < ie; ip += 32, op += 24) {			PREFETCH(ip,1024,0);
      __m256i          v  = _mm256_loadu_si256((__m256i *)ip);    
      __m256i  hi_nibbles = _mm256_srli_epi32(v, 4); 				//fromascii: https://arxiv.org/abs/1704.00605 P.15
               hi_nibbles = _mm256_and_si256(hi_nibbles, cv_2f);
      const __m256i roll  = _mm256_shuffle_epi8(lut_roll, _mm256_add_epi8(_mm256_cmpeq_epi8(v, cv_2f), hi_nibbles));

      __m256i lo_nibbles  = _mm256_and_si256(v, cv_2f);
                        v = _mm256_add_epi8(v, roll);
																//dec_reshuffle: https://arxiv.org/abs/1704.00605 P.17
      const __m256i merge_ab_and_bc = _mm256_maddubs_epi16(v,            _mm256_set1_epi32(0x01400140));
                                v = _mm256_madd_epi16(merge_ab_and_bc, _mm256_set1_epi32(0x00011000));								
      v = _mm256_shuffle_epi8(v, cpv);
	
      _mm_storeu_si128((__m128i*) op,       _mm256_castsi256_si128(v));
      _mm_storeu_si128((__m128i*)(op + 12), _mm256_extracti128_si256(v, 1));							
      if(!_mm256_testz_si256(_mm256_shuffle_epi8(lut_lo, lo_nibbles), _mm256_shuffle_epi8(lut_hi, hi_nibbles))) break;    
    }
    if(ip < ie) return 0;
  }
  unsigned rc;
  if(!(rc=tb64xdec(ip, inlen-(ip-in), op))) return 0;
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
