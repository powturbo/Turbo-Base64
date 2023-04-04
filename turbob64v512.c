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
// SSE + AVX2 Based on:
// http://0x80.pl/articles/index.html#base64-algorithm-update
// https://arxiv.org/abs/1704.00605
// https://gist.github.com/aqrit/a2ccea48d7cac7e9d4d99f19d4759666 (decode)

#include <immintrin.h>
#include "conf.h"
#include "turbob64.h"

#include "turbob64_.h"

#define PREFETCH(_ip_,_i_,_rw_) __builtin_prefetch(_ip_+(_i_),_rw_)

//--------------------- Decode ----------------------------------------------------------------------
#define CHECK0(a) a
  #ifdef B64CHECK
#define CHECK1(a) a
  #else
#define CHECK1(a)
  #endif

#define BITPACK512V8_6_(v) {\
  const __m512i merge_ab_and_bc = _mm512_maddubs_epi16(v,            _mm512_set1_epi32(0x01400140));\
                              v = _mm512_madd_epi16(merge_ab_and_bc, _mm512_set1_epi32(0x00011000));\
                              v = _mm512_shuffle_epi8(v, cpv);\
}

#define BITMAP256V8_6_(iv, shifted, ov) { /*map 8-bits ascii to 6-bits bin*/\
                shifted    = _mm512_srli_epi32(iv, 3);\
  const __m512i delta_hash = _mm512_avg_epu8(_mm512_shuffle_epi8(delta_asso, iv), shifted);\
                        ov = _mm512_add_epi8(_mm512_shuffle_epi8(delta_values, delta_hash), iv);\
}

#define B64CHK_(iv, shifted, vx) {\
  const __m512i check_hash = _mm512_avg_epu8( _mm512_shuffle_epi8(check_asso, iv), shifted);\
  const __m512i        chk = _mm512_adds_epi8(_mm512_shuffle_epi8(check_values, check_hash), iv);\
                        vx = _mm512_or_si512(vx, chk);\
}

size_t tb64v512dec0(const unsigned char *in, size_t inlen, unsigned char *out) {
  const unsigned char *ip = in;
        unsigned char *op = out; 
        
  __m512i vx = _mm512_setzero_si512();
  if(inlen >= 128+4) {
    const __m512i delta_asso   = _mm512_set_epi8(0x0f, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00,   0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                                                 0x0f, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00,   0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
												 0x0f, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00,   0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                                                 0x0f, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00,   0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01);
    const __m512i delta_values = _mm512_set_epi8(0xb9, 0xb9, 0xbf, 0xbf, 0xc3, 0x10, 0x00, 0xb9,   0xb9, 0xbf, 0xbf, 0x04, 0x13, 0x00, 0x00, 0x00, 
												 0xb9, 0xb9, 0xbf, 0xbf, 0xc3, 0x10, 0x00, 0xb9,   0xb9, 0xbf, 0xbf, 0x04, 0x13, 0x00, 0x00, 0x00, 
												 0xb9, 0xb9, 0xbf, 0xbf, 0xc3, 0x10, 0x00, 0xb9,   0xb9, 0xbf, 0xbf, 0x04, 0x13, 0x00, 0x00, 0x00, 
												 0xb9, 0xb9, 0xbf, 0xbf, 0xc3, 0x10, 0x00, 0xb9,   0xb9, 0xbf, 0xbf, 0x04, 0x13, 0x00, 0x00, 0x00);
    const __m512i check_asso   = _mm512_set_epi8(0x0f, 0x0b, 0x0b, 0x0b, 0x07, 0x03, 0x01, 0x01,   0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x0d,     
												 0x0f, 0x0b, 0x0b, 0x0b, 0x07, 0x03, 0x01, 0x01,   0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x0d,     
												 0x0f, 0x0b, 0x0b, 0x0b, 0x07, 0x03, 0x01, 0x01,   0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x0d,     
												 0x0f, 0x0b, 0x0b, 0x0b, 0x07, 0x03, 0x01, 0x01,   0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x0d);
    const __m512i check_values = _mm512_set_epi8(0x80, 0x91, 0x80, 0xb1, 0x80, 0xd1, 0x86, 0xb5,   0xa6, 0xd5, 0xbf, 0xcf, 0x80, 0x80, 0x80, 0x80,    
												 0x80, 0x91, 0x80, 0xb1, 0x80, 0xd1, 0x86, 0xb5,   0xa6, 0xd5, 0xbf, 0xcf, 0x80, 0x80, 0x80, 0x80,    
												 0x80, 0x91, 0x80, 0xb1, 0x80, 0xd1, 0x86, 0xb5,   0xa6, 0xd5, 0xbf, 0xcf, 0x80, 0x80, 0x80, 0x80,    
												 0x80, 0x91, 0x80, 0xb1, 0x80, 0xd1, 0x86, 0xb5,   0xa6, 0xd5, 0xbf, 0xcf, 0x80, 0x80, 0x80, 0x80),
                           cpv = _mm512_set_epi8( -1, -1, -1, -1, 12, 13, 14,  8,    9, 10,  4,  5,  6,  0,  1,  2,
                                                  -1, -1, -1, -1, 12, 13, 14,  8,    9, 10,  4,  5,  6,  0,  1,  2,
												  -1, -1, -1, -1, 12, 13, 14,  8,    9, 10,  4,  5,  6,  0,  1,  2,
                                                  -1, -1, -1, -1, 12, 13, 14,  8,    9, 10,  4,  5,  6,  0,  1,  2);

    for(        ; ip < in+(inlen-(128+4)); ip += 128, op += (128/4)*3) {           PREFETCH(ip,384,0);
      __m512i          iv0 = _mm512_loadu_si512((__m512i *) ip);    
      __m512i          iv1 = _mm512_loadu_si512((__m512i *)(ip+64)); 
   
      __m512i ov0,shifted0; BITMAP256V8_6_(iv0, shifted0, ov0); BITPACK512V8_6_(ov0);
      __m512i ov1,shifted1; BITMAP256V8_6_(iv1, shifted1, ov1); BITPACK512V8_6_(ov1);
      
      _mm_storeu_si128((__m128i*) op,       _mm512_castsi512_si128(   ov0   ));
      _mm_storeu_si128((__m128i*)(op + 12), _mm512_extracti32x4_epi32(ov0, 1));                          
      _mm_storeu_si128((__m128i*)(op + 24), _mm512_extracti32x4_epi32(ov0, 2));                          
      _mm_storeu_si128((__m128i*)(op + 36), _mm512_extracti32x4_epi32(ov0, 3));                          

      _mm_storeu_si128((__m128i*)(op + 48), _mm512_castsi512_si128(   ov1   ));
      _mm_storeu_si128((__m128i*)(op + 60), _mm512_extracti32x4_epi32(ov1, 1));                          
      _mm_storeu_si128((__m128i*)(op + 72), _mm512_extracti32x4_epi32(ov1, 2));                          
      _mm_storeu_si128((__m128i*)(op + 84), _mm512_extracti32x4_epi32(ov1, 3));                          
 
      CHECK0(B64CHK_(iv0, shifted0, vx));
      CHECK1(B64CHK_(iv1, shifted1, vx));
    }
  }
  unsigned rc, r = inlen-(ip-in); 
  if(r && !(rc=tb64xdec(ip, r, op)) || _mm512_movepi8_mask(vx)) return 0;
  return (op-out)+rc; 
}

//----------------------------------------------------------
#define BITMAP256V8_6(iv, ov) ov = _mm512_permutex2var_epi8(vlut0, iv, vlut1);  //AVX-512_VBMI

#define BITPACK512V8_6(v) {\
  __m512i merge_ab_bc = _mm512_maddubs_epi16(v,        _mm512_set1_epi32(0x01400140)),\
                   vm = _mm512_madd_epi16(merge_ab_bc, _mm512_set1_epi32(0x00011000));\
                   v  = _mm512_permutexvar_epi8(vp, vm);\
}

#define B64CHK(iv, ov, vx) vx = _mm512_ternarylogic_epi32(vx, ov, iv, 0xfe)

#define DS512(_i_) { \
      __m512i          iv0 = _mm512_loadu_si512((__m512i *)(ip+128+_i_*256)),    \
                       iv1 = _mm512_loadu_si512((__m512i *)(ip+128+_i_*256+64)); \
      __m512i ou0; BITMAP256V8_6(iu0, ou0); CHECK0(B64CHK(iu0, ou0, vx)); BITPACK512V8_6(ou0);\
      __m512i ou1; BITMAP256V8_6(iu1, ou1); CHECK1(B64CHK(iu1, ou1, vx)); BITPACK512V8_6(ou1);\
	                   iu0 = _mm512_loadu_si512((__m512i *)(ip+128+_i_*256+128)),\
                       iu1 = _mm512_loadu_si512((__m512i *)(ip+128+_i_*256+192));\
      _mm512_storeu_si512((__m128i*)(op+_i_*192), ou0);\
      _mm512_storeu_si512((__m128i*)(op+_i_*192+48), ou1);\
      __m512i ov0; BITMAP256V8_6(iv0, ov0); CHECK0(B64CHK(iv0, ov0, vx)); BITPACK512V8_6(ov0);\
      __m512i ov1; BITMAP256V8_6(iv1, ov1); CHECK1(B64CHK(iv1, ov1, vx)); BITPACK512V8_6(ov1);\
      _mm512_storeu_si512((__m128i*)(op+_i_*192+ 96), ov0);\
      _mm512_storeu_si512((__m128i*)(op+_i_*192+144), ov1);\
}

//-----------------------------------------------
size_t tb64v512dec(const unsigned char *in, size_t inlen, unsigned char *out) {
  const unsigned char *ip = in;
        unsigned char *op = out; 
  #define DN 512     
  __m512i vx = _mm512_setzero_si512();
  if(inlen > 56+128) { 																			
    const __m512i vlut0 = _mm512_setr_epi32(0x80808080, 0x80808080, 0x80808080, 0x80808080,
                                            0x80808080, 0x80808080, 0x80808080, 0x80808080,
                                            0x80808080, 0x80808080, 0x3e808080, 0x3f808080,
                                            0x37363534, 0x3b3a3938, 0x80803d3c, 0x80808080),
                  vlut1 = _mm512_setr_epi32(0x02010080, 0x06050403, 0x0a090807, 0x0e0d0c0b,
                                            0x1211100f, 0x16151413, 0x80191817, 0x80808080,
                                            0x1c1b1a80, 0x201f1e1d, 0x24232221, 0x28272625,
                                            0x2c2b2a29, 0x302f2e2d, 0x80333231, 0x80808080),
                     vp = _mm512_setr_epi32(0x06000102, 0x090a0405, 0x0c0d0e08, 0x16101112,
                                            0x191a1415, 0x1c1d1e18, 0x26202122, 0x292a2425,
                                            0x2c2d2e28, 0x36303132, 0x393a3435, 0x3c3d3e38,
                                            0x00000000, 0x00000000, 0x00000000, 0x00000000);												  

      __m512i          iu0 = _mm512_loadu_si512((__m512i *) ip),    
                       iu1 = _mm512_loadu_si512((__m512i *)(ip+64)); 
    for(        ; ip < in+(inlen-(DN+4)); ip += DN, op += (DN/4)*3) {           PREFETCH(ip,384,0);
	  DS512(0);
	    #if DN > 256
	  DS512(1);
		#endif
    }
	for(; ip < (in+inlen)-64-4; ip += 64, op += 64*3/4) {
      __m512i iv = _mm512_loadu_si512((__m512i *) ip), ov;
      BITMAP256V8_6(iv, ov); CHECK0(B64CHK(iv, ov, vx)); BITPACK512V8_6(ov);
      _mm512_storeu_si512((__m128i*) op, ov);
    }
  }
  unsigned rc, r = inlen-(ip-in); 
  if(r && !(rc=tb64xdec(ip, r, op)) || _mm512_movepi8_mask(vx)) return 0;
  return (op-out)+rc; 
}

//-------------------- Encode ----------------------------------------------------------------------
//AVX512_VBMI: https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#expand=1276,5146,5146,5146&text=_mm512_multishift_epi64_epi8&avx512techs=AVX512_VBMI
//reference: http://0x80.pl/notesen/2016-04-03-avx512-base64.html#avx512vbmi
#define ES512(_i_) { __m512i v0,v1;\
  v0 = _mm512_loadu_si512((__m512i *)(ip+96+_i_*192) ),\
  v1 = _mm512_loadu_si512((__m512i *)(ip+96+_i_*192+48));\
  u0 = _mm512_permutexvar_epi8(_mm512_multishift_epi64_epi8(vs, _mm512_permutexvar_epi8(vf, u0)), vlut);\
  u1 = _mm512_permutexvar_epi8(_mm512_multishift_epi64_epi8(vs, _mm512_permutexvar_epi8(vf, u1)), vlut);\
  _mm512_storeu_si512((__m512i*)(op+_i_*256),     u0);\
  _mm512_storeu_si512((__m512i*)(op+_i_*256+64), u1);\
                                                  \
  u0 = _mm512_loadu_si512((__m512i *)(ip+96+_i_*192+ 96));\
  u1 = _mm512_loadu_si512((__m512i *)(ip+96+_i_*192+144));\
  v0 = _mm512_permutexvar_epi8(_mm512_multishift_epi64_epi8(vs, _mm512_permutexvar_epi8(vf, v0)), vlut);\
  v1 = _mm512_permutexvar_epi8(_mm512_multishift_epi64_epi8(vs, _mm512_permutexvar_epi8(vf, v1)), vlut);\
  _mm512_storeu_si512((__m512i*)(op+_i_*256+128), v0);\
  _mm512_storeu_si512((__m512i*)(op+_i_*256+192), v1);\
}

size_t tb64v512enc(const unsigned char* in, size_t inlen, unsigned char *out) {
  const unsigned char *ip = in, *op = out;
        unsigned   outlen = TB64ENCLEN(inlen);

  static const char *lut = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  const __m512i     vlut = _mm512_loadu_si512((const __m512i*)lut);
  const __m512i       vf = _mm512_setr_epi32(0x01020001, 0x04050304, 0x07080607, 0x0a0b090a,
                                             0x0d0e0c0d, 0x10110f10, 0x13141213, 0x16171516,
                                             0x191a1819, 0x1c1d1b1c, 0x1f201e1f, 0x22232122,
                                             0x25262425, 0x28292728, 0x2b2c2a2b, 0x2e2f2d2e);
  const __m512i       vs = _mm512_set1_epi64(0x3036242a1016040alu); // 48, 54, 36, 42, 16, 22, 4, 10

  #define EN 256		
  if(outlen >= 128+256) {
    __m512i u0 = _mm512_loadu_si512((__m512i *) ip );
    __m512i u1 = _mm512_loadu_si512((__m512i *)(ip+48));
    for(; op < (out+outlen)-(128+EN); op += EN, ip += EN*3/4) {
      ES512(0); 
	#if EN > 256
      ES512(1);
	#endif
      PREFETCH(ip, 384, 0);
    }
      #if EN > 256
    if(op < (out+outlen)-(128+256)) { ES256(0); op += 256; ip += 256*3/4; }
      #endif	
  }
  
  const __m256i vs = _mm256_set_epi8(10,11, 9,10, 7, 8, 6, 7, 4,   5, 3, 4, 1, 2, 0, 1,
                                     10,11, 9,10, 7, 8, 6, 7, 4,   5, 3, 4, 1, 2, 0, 1);
  for(; op < out+outlen-32; op += 32, ip += 32*3/4) {
    __m256i v = _mm256_castsi128_si256(   _mm_loadu_si128((__m128i *) ip    )  );      
            v = _mm256_inserti128_si256(v,_mm_loadu_si128((__m128i *)(ip+12)),1);   
            v = _mm256_shuffle_epi8(v, vs); v = bitunpack256v8_6(v); v = bitmap256v8_6(v);                                                                                                           
                _mm256_storeu_si256((__m256i*) op, v);                                                 
  }
  EXTAIL();
  return outlen;
}
