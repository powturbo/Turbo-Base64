/**
    Copyright (C) powturbo 2016-2023
    SPDX-License-Identifier: GPL v3 License

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

    - homepage : https://sites.google.com/site/powturbo/
    - github   : https://github.com/powturbo
    - twitter  : https://twitter.com/powturbo
    - email    : powturbo [_AT_] gmail [_DOT_] com
**/
// Turbo-Base64: avx2 encode/decode

// SSE + AVX2 Based on:
// http://0x80.pl/articles/index.html#base64-algorithm-update
// https://arxiv.org/abs/1704.00605
// https://gist.github.com/aqrit/a2ccea48d7cac7e9d4d99f19d4759666 (decode)

#include <immintrin.h>
#include "turbob64.h"
#include  "conf.h" //AS
#include "turbob64_.h"

//--------------------- Decode ----------------------------------------------------------------------
#define BITPACK256V8_6(v,cpv) {\
  const __m256i merge_ab_bc = _mm256_maddubs_epi16(v,            _mm256_set1_epi32(0x01400140));\
                          v = _mm256_madd_epi16(merge_ab_bc, _mm256_set1_epi32(0x00011000));\
                          v = _mm256_shuffle_epi8(v, cpv);\
}

#define BITMAP256V8_6(iv, shifted, delta_asso, delta_values, ov) { /*map 8-bits ascii to 6-bits bin*/\
                shifted    = _mm256_srli_epi32(iv, 3);\
  const __m256i delta_hash = _mm256_avg_epu8(_mm256_shuffle_epi8(delta_asso, iv), shifted);\
                        ov = _mm256_add_epi8(_mm256_shuffle_epi8(delta_values, delta_hash), iv);\
}
//----------------------------
#define BITPACK256V8_6x(v0,v1,cpv) {\
  const __m256i merge_ab_bc0 = _mm256_maddubs_epi16(v0,        _mm256_set1_epi32(0x01400140));\
  const __m256i merge_ab_bc1 = _mm256_maddubs_epi16(v1,        _mm256_set1_epi32(0x01400140));\
                          v0 = _mm256_madd_epi16(merge_ab_bc0, _mm256_set1_epi32(0x00011000));\
                          v1 = _mm256_madd_epi16(merge_ab_bc1, _mm256_set1_epi32(0x00011000));\
                          v0 = _mm256_shuffle_epi8(v0, cpv);\
                          v1 = _mm256_shuffle_epi8(v1, cpv);\
}

#define BITMAP256V8_6x(iv0, shifted0, iv1, shifted1, delta_asso, delta_values, ov0, ov1) { /*map 8-bits ascii to 6-bits bin*/\
  __m256i delta_hash0 = _mm256_shuffle_epi8(delta_asso, iv0);\
  __m256i delta_hash1 = _mm256_shuffle_epi8(delta_asso, iv1);\
          shifted0    = _mm256_srli_epi32(iv0, 3);\
          delta_hash0 = _mm256_avg_epu8(delta_hash0, shifted0);\
          shifted1    = _mm256_srli_epi32(iv1, 3);\
          delta_hash1 = _mm256_avg_epu8(delta_hash1, shifted1);\
                  ov0 = _mm256_add_epi8(_mm256_shuffle_epi8(delta_values, delta_hash0), iv0);\
                  ov1 = _mm256_add_epi8(_mm256_shuffle_epi8(delta_values, delta_hash1), iv1);\
}

//--------------------------
#define B64CHK256(iv, shifted, check_asso, check_values, vx) {\
  const __m256i check_hash = _mm256_avg_epu8( _mm256_shuffle_epi8(check_asso, iv), shifted);\
  const __m256i        chk = _mm256_adds_epi8(_mm256_shuffle_epi8(check_values, check_hash), iv);\
                        vx = _mm256_or_si256(vx, chk);\
}

#define DS128(_i_) {\
  __m256i iv0 = _mm256_loadu_si256((__m256i *)(ip+64+_i_*128+0)), \
          iv1 = _mm256_loadu_si256((__m256i *)(ip+64+_i_*128+32));\
                                                                  \
  __m256i ou0,shiftedu0; /*BITMAP256V8_6(iu0, shiftedu0, delta_asso, delta_values, ou0); BITPACK256V8_6(ou0, cpv);*/\
  __m256i ou1,shiftedu1; /*BITMAP256V8_6(iu1, shiftedu1, delta_asso, delta_values, ou1); BITPACK256V8_6(ou1, cpv);*/\
  BITMAP256V8_6x(iu0, shiftedu0, iu1, shiftedu1, delta_asso, delta_values, ou0, ou1);\
  BITPACK256V8_6x(ou0, ou1, cpv);\
  CHECK0(B64CHK256(iu0,shiftedu0, check_asso, check_values, vx)); \
          iu0 = _mm256_loadu_si256((__m256i *)(ip+64+_i_*128+64));\
  CHECK1(B64CHK256(iu1,shiftedu1, check_asso, check_values, vx)); \
          iu1 = _mm256_loadu_si256((__m256i *)(ip+64+_i_*128+96));\
  _mm_storeu_si128((__m128i*)(op+_i_*96   ), _mm256_castsi256_si128(  ou0   ));\
  _mm_storeu_si128((__m128i*)(op+_i_*96+12), _mm256_extracti128_si256(ou0, 1));\
  _mm_storeu_si128((__m128i*)(op+_i_*96+24), _mm256_castsi256_si128(  ou1   ));\
  _mm_storeu_si128((__m128i*)(op+_i_*96+36), _mm256_extracti128_si256(ou1, 1));\
                                                                               \
  __m256i ov0,shiftedv0; /*BITMAP256V8_6(iv0, shiftedv0, delta_asso, delta_values, ov0); BITPACK256V8_6(ov0, cpv);*/\
  __m256i ov1,shiftedv1; /*BITMAP256V8_6(iv1, shiftedv1, delta_asso, delta_values, ov1); BITPACK256V8_6(ov1, cpv);*/\
  BITMAP256V8_6x(iv0, shiftedv0, iv1, shiftedv1, delta_asso, delta_values, ov0, ov1);\
  BITPACK256V8_6x(ov0, ov1, cpv);\
                                                                  \
  CHECK1(B64CHK256(iv0, shiftedv0, check_asso, check_values, vx));\
  CHECK1(B64CHK256(iv1, shiftedv1, check_asso, check_values, vx));\
  _mm_storeu_si128((__m128i*)(op+_i_*96+48), _mm256_castsi256_si128(  ov0   ));\
  _mm_storeu_si128((__m128i*)(op+_i_*96+60), _mm256_extracti128_si256(ov0, 1));\
  _mm_storeu_si128((__m128i*)(op+_i_*96+72), _mm256_castsi256_si128(  ov1   ));\
  _mm_storeu_si128((__m128i*)(op+_i_*96+84), _mm256_extracti128_si256(ov1, 1));\
}

size_t tb64v256dec(const unsigned char *__restrict in, size_t inlen, unsigned char *__restrict out) {
  if(inlen&3) 
	return 0;                               
  
  const unsigned char *ip = in, *in_ = in + inlen;
        unsigned char *op = out;
        __m256i vx           = _mm256_setzero_si256();
  const __m256i delta_asso   = _mm256_setr_epi8(0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,   0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x0f,
                                                0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,   0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x0f),
                delta_values = _mm256_setr_epi8(0x00, 0x00, 0x00, 0x13, 0x04, 0xbf, 0xbf, 0xb9,   0xb9, 0x00, 0x10, 0xc3, 0xbf, 0xbf, 0xb9, 0xb9,
                                                0x00, 0x00, 0x00, 0x13, 0x04, 0xbf, 0xbf, 0xb9,   0xb9, 0x00, 0x10, 0xc3, 0xbf, 0xbf, 0xb9, 0xb9),
    #ifndef NB64CHECK
                check_asso   = _mm256_setr_epi8(0x0d, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,   0x01, 0x01, 0x03, 0x07, 0x0b, 0x0b, 0x0b, 0x0f,
                                                0x0d, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,   0x01, 0x01, 0x03, 0x07, 0x0b, 0x0b, 0x0b, 0x0f),
                check_values = _mm256_setr_epi8(0x80, 0x80, 0x80, 0x80, 0xcf, 0xbf, 0xd5, 0xa6,   0xb5, 0x86, 0xd1, 0x80, 0xb1, 0x80, 0x91, 0x80,
                                                0x80, 0x80, 0x80, 0x80, 0xcf, 0xbf, 0xd5, 0xa6,   0xb5, 0x86, 0xd1, 0x80, 0xb1, 0x80, 0x91, 0x80),
    #endif
                         cpv = _mm256_set_epi8( -1, -1, -1, -1, 12, 13, 14,  8,    9, 10,  4,  5,  6,  0,  1,  2,
                                                -1, -1, -1, -1, 12, 13, 14,  8,    9, 10,  4,  5,  6,  0,  1,  2);												  
        __m128i          _vx;	
											
  if(likely(inlen >= 64+128+4)) { 																			
    __m256i iu0 = _mm256_loadu_si256((__m256i *) ip    ),   
	        iu1 = _mm256_loadu_si256((__m256i *)(ip+32));    
    for(;ip < in_-(64+2*128+4); ip += 256, op += 256*3/4) { DS128(0); DS128(1); }	
    if(  ip < in_-(64+  128+4))                           { DS128(0); ip += 128; op += 128*3/4; }
    CHECK0(_vx = _mm_or_si128(_mm256_extracti128_si256(vx, 1), _mm256_castsi256_si128(vx)));
  } else { 
	CHECK0(_vx = _mm_setzero_si128());
    if(!inlen) return 0; 
  }
  
  for(;ip < in_-(16+4); ip += 16, op += 16*3/4) {
    __m128i iv = _mm_loadu_si128((__m128i *) ip), ov, vsh; 
	BITMAP128V8_6(iv, vsh, _mm256_castsi256_si128(delta_asso), _mm256_castsi256_si128(delta_values), ov); 
	BITPACK128V8_6(ov, _mm256_castsi256_si128(cpv));
    _mm_storeu_si128((__m128i*) op, ov);    
    CHECK0(B64CHK128(iv, vsh, _mm256_castsi256_si128(check_asso), _mm256_castsi256_si128(check_values), _vx));
  }
 
  size_t rc = 0, r = in_- ip; 
  if(r && !(rc = _tb64xd(ip, r, op)) CHECK0(|| _mm_movemask_epi8(_vx))) 
	return 0;                                                                      //AC(op+rc == out+tb64declen(in, inlen), "#4 out"); AC(ip+r == in+inlen, "#5 in");
  return (op - out)+rc;
}

//-------------------- Encode ----------------------------------------------------------------------

#define ES128(_i_) {\
  __m256i v0 = _mm256_castsi128_si256(    _mm_loadu_si128((__m128i *)(ip+48+_i_*96+ 0))  );\
          v0 = _mm256_inserti128_si256(v0,_mm_loadu_si128((__m128i *)(ip+48+_i_*96+12)),1);\
  __m256i v1 = _mm256_castsi128_si256(    _mm_loadu_si128((__m128i *)(ip+48+_i_*96+24))  );\
          v1 = _mm256_inserti128_si256(v1,_mm_loadu_si128((__m128i *)(ip+48+_i_*96+36)),1);\
                                                                                           \
  u0 = _mm256_shuffle_epi8(u0, vh); u0 = bitunpack256v8_6(u0); u0 = bitmap256v8_6(u0);\
  u1 = _mm256_shuffle_epi8(u1, vh); u1 = bitunpack256v8_6(u1); u1 = bitmap256v8_6(u1);\
       _mm256_storeu_si256((__m256i*)(op+_i_*128),    u0);                              \
       _mm256_storeu_si256((__m256i*)(op+_i_*128+32), u1);                              \
		                                                                                   \
          u0 = _mm256_castsi128_si256(    _mm_loadu_si128((__m128i *)(ip+48+_i_*96+48))  );\
          u0 = _mm256_inserti128_si256(u0,_mm_loadu_si128((__m128i *)(ip+48+_i_*96+60)),1);\
          u1 = _mm256_castsi128_si256(    _mm_loadu_si128((__m128i *)(ip+48+_i_*96+72))  );\
          u1 = _mm256_inserti128_si256(u1,_mm_loadu_si128((__m128i *)(ip+48+_i_*96+84)),1); \
                                                                                           \
  v0 = _mm256_shuffle_epi8(v0, vh); v0 = bitunpack256v8_6(v0); v0 = bitmap256v8_6(v0);\
  v1 = _mm256_shuffle_epi8(v1, vh); v1 = bitunpack256v8_6(v1); v1 = bitmap256v8_6(v1); \
       _mm256_storeu_si256((__m256i*)(op+_i_*128+64), v0);\
       _mm256_storeu_si256((__m256i*)(op+_i_*128+96), v1);\
}

size_t tb64v256enc(const unsigned char *__restrict in, size_t inlen, unsigned char *__restrict out) {
            size_t outlen = TB64ENCLEN(inlen);
  const unsigned char *ip = in, *out_ = out+outlen; 
        unsigned char *op = out;

  const __m256i vh = _mm256_set_epi8(10,11, 9,10, 7, 8, 6, 7, 4,   5, 3, 4, 1, 2, 0, 1,
                                     10,11, 9,10, 7, 8, 6, 7, 4,   5, 3, 4, 1, 2, 0, 1);
  if(outlen >= (48+96+4)*4/3) {
    __m256i u0 = _mm256_castsi128_si256(    _mm_loadu_si128((__m128i *) ip    )  );   
            u0 = _mm256_inserti128_si256(u0,_mm_loadu_si128((__m128i *)(ip+12)),1);   
    __m256i u1 = _mm256_castsi128_si256(    _mm_loadu_si128((__m128i *)(ip+24))  );      
            u1 = _mm256_inserti128_si256(u1,_mm_loadu_si128((__m128i *)(ip+36)),1);   
    for(; op < out_ - (48+2*96+4)*4/3; ip += 256*3/4, op += 256) { ES128(0); ES128(1); }		    
    if(   op < out_ - (48+  96+4)*4/3) { ES128(0); ip += 128*3/4; op += 128;  }		    
  } 
  
  for(; op < out_- (24+4)*4/3; op += 32, ip += 32*3/4) {
    __m256i v = _mm256_castsi128_si256(   _mm_loadu_si128((__m128i *) ip    )  );      
            v = _mm256_inserti128_si256(v,_mm_loadu_si128((__m128i *)(ip+12)),1);   
            v = _mm256_shuffle_epi8(v, vh); 
			v = bitunpack256v8_6(v); 
			v = bitmap256v8_6(v);                                                                                                           
            _mm256_storeu_si256((__m256i*) op, v);                                                 
  }
  EXTAIL(7);
  return outlen;
}

//------- optimized functions for short strings only --------------------------
// OVHD=0 : unsafe, can read beyond the input buffer end, therefore input buffer size must be 32 bytes larger than input length
#define OVHD 0
//#define OVHD 4 
#define _CHECK0(a) CHECK0(a)
#define _CHECK1(a) CHECK1(a)

size_t _tb64v256dec(const unsigned char *__restrict in, size_t inlen, unsigned char *__restrict out) {  AS((inlen&3)==0, "inlen not multiple of 4\n");
  
  if(inlen >= 16+OVHD) {
    const unsigned char *ip = in, *in_ = in + inlen;
          unsigned char *op = out;
    const __m256i delta_asso   = _mm256_setr_epi8(0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,   0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x0f,
                                                  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,   0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x0f),
                  delta_values = _mm256_setr_epi8(0x00, 0x00, 0x00, 0x13, 0x04, 0xbf, 0xbf, 0xb9,   0xb9, 0x00, 0x10, 0xc3, 0xbf, 0xbf, 0xb9, 0xb9,
                                                  0x00, 0x00, 0x00, 0x13, 0x04, 0xbf, 0xbf, 0xb9,   0xb9, 0x00, 0x10, 0xc3, 0xbf, 0xbf, 0xb9, 0xb9),
      #ifndef NB64CHECK
                  check_asso   = _mm256_setr_epi8(0x0d, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,   0x01, 0x01, 0x03, 0x07, 0x0b, 0x0b, 0x0b, 0x0f,
                                                  0x0d, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,   0x01, 0x01, 0x03, 0x07, 0x0b, 0x0b, 0x0b, 0x0f),
                  check_values = _mm256_setr_epi8(0x80, 0x80, 0x80, 0x80, 0xcf, 0xbf, 0xd5, 0xa6,   0xb5, 0x86, 0xd1, 0x80, 0xb1, 0x80, 0x91, 0x80,
                                                  0x80, 0x80, 0x80, 0x80, 0xcf, 0xbf, 0xd5, 0xa6,   0xb5, 0x86, 0xd1, 0x80, 0xb1, 0x80, 0x91, 0x80),
      #endif
                           cpv = _mm256_set_epi8( -1, -1, -1, -1, 12, 13, 14,  8,    9, 10,  4,  5,  6,  0,  1,  2,
                                                  -1, -1, -1, -1, 12, 13, 14,  8,    9, 10,  4,  5,  6,  0,  1,  2);												  
          __m256i           vx = _mm256_setzero_si256();

    for(; ip < in_-(32+OVHD); ip += 32, op += 32*3/4) {
      __m256i iv = _mm256_loadu_si256((__m256i *)ip), ov, vsh; 
	  BITMAP256V8_6(iv, vsh, delta_asso, delta_values, ov); 
	  BITPACK256V8_6(ov, cpv);
      
      _mm_storeu_si128((__m128i*) op,       _mm256_castsi256_si128(ov));
      _mm_storeu_si128((__m128i*)(op + 12), _mm256_extracti128_si256(ov, 1)); 
      _CHECK1(B64CHK256(iv, vsh, check_asso, check_values, vx));
    }
	
    unsigned cx; 
    if(ip < in_-(16+OVHD)) {
      __m128i iv = _mm_loadu_si128((__m128i *) ip), ov, vsh; 
	  ip += 16; 
	    #ifdef B64CHECK
      __m128i _vx = _mm_or_si128(_mm256_extracti128_si256(vx, 1), _mm256_castsi256_si128(vx));
	    #else
      __m128i _vx = _mm_setzero_si128();
		#endif
	  BITMAP128V8_6(iv, vsh, _mm256_castsi256_si128(delta_asso), _mm256_castsi256_si128(delta_values), ov); 
	  BITPACK128V8_6(ov, _mm256_castsi256_si128(cpv));
      _mm_storeu_si128((__m128i*) op, ov);                        
	  op += 16*3/4; 
      _CHECK0(B64CHK128(iv, vsh, _mm256_castsi256_si128(check_asso), _mm256_castsi256_si128(check_values), _vx));
      _CHECK0(cx = _mm_movemask_epi8(_vx));
    } _CHECK0(else cx = _mm256_movemask_epi8(vx));
    size_t rc = 0, r = in_- ip; 
    if(r && !(rc = _tb64xd(ip, r, op)) _CHECK0(|| cx)) 
	  return 0;                                                                      //AC(op+rc == out+tb64declen(in, inlen), "#4 out"); AC(ip+r == in+inlen, "#5 in");
    return (op - out)+rc;
  } else if(!inlen) return 0;
  return _tb64xd(in, inlen, out);
}

size_t _tb64v256enc(const unsigned char *__restrict in, size_t inlen, unsigned char *__restrict out) {
            size_t outlen = TB64ENCLEN(inlen);
  const unsigned char *ip = in, *out_ = out+outlen; 
        unsigned char *op = out;

  const __m256i vh = _mm256_set_epi8(10,11, 9,10, 7, 8, 6, 7, 4,   5, 3, 4, 1, 2, 0, 1,
                                     10,11, 9,10, 7, 8, 6, 7, 4,   5, 3, 4, 1, 2, 0, 1);
									 
  for(; op < out_- (24+4)*4/3; op += 32, ip += 32*3/4) {
    __m256i v = _mm256_castsi128_si256(   _mm_loadu_si128((__m128i *) ip    )  );      
            v = _mm256_inserti128_si256(v,_mm_loadu_si128((__m128i *)(ip+12)),1);   
            v = _mm256_shuffle_epi8(v, vh); 
			v = bitunpack256v8_6(v); 
			v = bitmap256v8_6(v);                                                                                                           
            _mm256_storeu_si256((__m256i*) op, v);                                                 
  }
  if(op <= out_-(12+4)*4/3) {
    __m128i v0 = _mm_loadu_si128((__m128i*)ip);
            v0 = _mm_shuffle_epi8(v0, _mm256_castsi256_si128(vh));
            v0 = bitunpack128v8_6(v0);
            v0 = bitmap128v8_6(v0);
    _mm_storeu_si128((__m128i*) op, v0);                                          
    op += 16; ip += (16/4)*3;
  }
  EXTAIL(3);
  return outlen;
}
