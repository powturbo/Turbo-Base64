/**
    Copyright (C) powturbo 2016-2022
    GPL v3 License

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

#define UA_MEMCPY
#include "conf.h"
#include "turbob64.h"
#include "turbob64_.h"

//--------------------- Decode ----------------------------------------------------------------------
#define MM256_PACK8TO6(v,cpv) {\
  const __m256i merge_ab_and_bc = _mm256_maddubs_epi16(v,            _mm256_set1_epi32(0x01400140));\
                              v = _mm256_madd_epi16(merge_ab_and_bc, _mm256_set1_epi32(0x00011000));\
                              v = _mm256_shuffle_epi8(v, cpv);\
}

#define MM256_MAP8TO6(iv, shifted, delta_asso, delta_values, ov) { /*map 8-bits ascii to 6-bits bin*/\
                shifted    = _mm256_srli_epi32(iv, 3);\
  const __m256i delta_hash = _mm256_avg_epu8(_mm256_shuffle_epi8(delta_asso, iv), shifted);\
                        ov = _mm256_add_epi8(_mm256_shuffle_epi8(delta_values, delta_hash), iv);\
}

#define MM256_B64CHK(iv, shifted, check_asso, check_values, vx) {\
  const __m256i check_hash = _mm256_avg_epu8( _mm256_shuffle_epi8(check_asso, iv), shifted);\
  const __m256i        chk = _mm256_adds_epi8(_mm256_shuffle_epi8(check_values, check_hash), iv);\
                        vx = _mm256_or_si256(vx, chk);\
}

#define DS256(_i_) {\
  __m256i iv0 = _mm256_loadu_si256((__m256i *)(ip+64+_i_*128+0)),    \
          iv1 = _mm256_loadu_si256((__m256i *)(ip+64+_i_*128+32));\
\
  __m256i ou0,shiftedu0; MM256_MAP8TO6(iu0, shiftedu0, delta_asso, delta_values, ou0); MM256_PACK8TO6(ou0, cpv);\
  __m256i ou1,shiftedu1; MM256_MAP8TO6(iu1, shiftedu1, delta_asso, delta_values, ou1); MM256_PACK8TO6(ou1, cpv);\
      \
  CHECK0(MM256_B64CHK(iu0,shiftedu0, check_asso, check_values, vx));\
          iu0 = _mm256_loadu_si256((__m256i *)(ip+64+_i_*128+64));\
  CHECK1(MM256_B64CHK(iu1,shiftedu1, check_asso, check_values, vx));\
          iu1 = _mm256_loadu_si256((__m256i *)(ip+64+_i_*128+96));\
  _mm_storeu_si128((__m128i*)(op+_i_*96   ), _mm256_castsi256_si128(  ou0   ));\
  _mm_storeu_si128((__m128i*)(op+_i_*96+12), _mm256_extracti128_si256(ou0, 1));\
  _mm_storeu_si128((__m128i*)(op+_i_*96+24), _mm256_castsi256_si128(  ou1   ));\
  _mm_storeu_si128((__m128i*)(op+_i_*96+36), _mm256_extracti128_si256(ou1, 1));\
  \
  __m256i ov0,shiftedv0; MM256_MAP8TO6(iv0, shiftedv0, delta_asso, delta_values, ov0); MM256_PACK8TO6(ov0, cpv);\
  __m256i ov1,shiftedv1; MM256_MAP8TO6(iv1, shiftedv1, delta_asso, delta_values, ov1); MM256_PACK8TO6(ov1, cpv); \
      \
  CHECK1(MM256_B64CHK(iv0, shiftedv0, check_asso, check_values, vx));\
  CHECK1(MM256_B64CHK(iv1, shiftedv1, check_asso, check_values, vx));\
  _mm_storeu_si128((__m128i*)(op+_i_*96+48), _mm256_castsi256_si128(  ov0   ));\
  _mm_storeu_si128((__m128i*)(op+_i_*96+60), _mm256_extracti128_si256(ov0, 1));\
  _mm_storeu_si128((__m128i*)(op+_i_*96+72), _mm256_castsi256_si128(  ov1   ));\
  _mm_storeu_si128((__m128i*)(op+_i_*96+84), _mm256_extracti128_si256(ov1, 1));\
}

size_t tb64v256dec(const unsigned char *__restrict in, size_t inlen, unsigned char *__restrict out) {
  if(!inlen || (inlen&3)) return 0; 
  if(inlen <= 56) return _tb64xd(in, inlen, out);
  
  const unsigned char *ip = in;
        unsigned char *op = out;
        __m256i vx           = _mm256_setzero_si256();
  const __m256i delta_asso   = _mm256_setr_epi8(0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,   0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x0f,
                                                0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,   0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x0f);
  const __m256i delta_values = _mm256_setr_epi8(0x00, 0x00, 0x00, 0x13, 0x04, 0xbf, 0xbf, 0xb9,   0xb9, 0x00, 0x10, 0xc3, 0xbf, 0xbf, 0xb9, 0xb9,
                                                0x00, 0x00, 0x00, 0x13, 0x04, 0xbf, 0xbf, 0xb9,   0xb9, 0x00, 0x10, 0xc3, 0xbf, 0xbf, 0xb9, 0xb9);
    #ifndef NB64CHECK
  const __m256i check_asso   = _mm256_setr_epi8(0x0d, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,   0x01, 0x01, 0x03, 0x07, 0x0b, 0x0b, 0x0b, 0x0f,
                                                0x0d, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,   0x01, 0x01, 0x03, 0x07, 0x0b, 0x0b, 0x0b, 0x0f);
  const __m256i check_values = _mm256_setr_epi8(0x80, 0x80, 0x80, 0x80, 0xcf, 0xbf, 0xd5, 0xa6,   0xb5, 0x86, 0xd1, 0x80, 0xb1, 0x80, 0x91, 0x80,
                                                0x80, 0x80, 0x80, 0x80, 0xcf, 0xbf, 0xd5, 0xa6,   0xb5, 0x86, 0xd1, 0x80, 0xb1, 0x80, 0x91, 0x80);
    #endif
        __m256i          cpv = _mm256_set_epi8( -1, -1, -1, -1, 12, 13, 14,  8,    9, 10,  4,  5,  6,  0,  1,  2,
                                                -1, -1, -1, -1, 12, 13, 14,  8,    9, 10,  4,  5,  6,  0,  1,  2);												  
  __m128i                _vx = _mm_setzero_si128();												
  if(inlen > 56+128) { 																			
    __m256i iu0 = _mm256_loadu_si256((__m256i *) ip    ),   
	        iu1 = _mm256_loadu_si256((__m256i *)(ip+32));    
	#define DN 256 //128  // 
    for(; ip < (in+inlen)-(64+DN); ip += DN, op += DN*3/4) { 
      DS256(0);	
	    #if DN > 128
	  DS256(1); 	
        #endif	  
																								PREFETCH(ip,384,0);
    }	
    for(; ip < (in+inlen)-32-4; ip += 32, op += 32*3/4) {
      __m256i iv = _mm256_loadu_si256((__m256i *)ip);      
      __m256i ov,shifted0; MM256_MAP8TO6(iv, shifted0, delta_asso, delta_values, ov); MM256_PACK8TO6(ov, cpv);
      
      _mm_storeu_si128((__m128i*) op,       _mm256_castsi256_si128(ov));
      _mm_storeu_si128((__m128i*)(op + 12), _mm256_extracti128_si256(ov, 1));                          
     
      CHECK1(MM256_B64CHK(iv, shifted0, check_asso, check_values, vx));
    }
    _vx = _mm_or_si128(_mm256_extracti128_si256(vx, 1), _mm256_castsi256_si128(vx));
  }
  for(;ip < (in+inlen)-16-4;ip += 16, op += 16*3/4) {
    __m128i iv0 = _mm_loadu_si128((__m128i *) ip);
    __m128i ov0, shifted0; MM_MAP8TO6( iv0, shifted0,_mm256_castsi256_si128(delta_asso),_mm256_castsi256_si128(delta_values), ov0); 
	                       MM_PACK8TO6(ov0, _mm256_castsi256_si128(cpv));
    _mm_storeu_si128((__m128i*) op, ov0);  
    
    CHECK0(MM_B64CHK(iv0, shifted0, _mm256_castsi256_si128(check_asso), _mm256_castsi256_si128(check_values), _vx));
  }
  unsigned cx = _mm_movemask_epi8(_vx);
  size_t rc = tb64xdec(ip, inlen-(ip-in), op);
  if(!rc || cx) return 0;
  return (op-out)+rc;
}

//-------------------- Encode ----------------------------------------------------------------------
static ALWAYS_INLINE __m256i mm256_map6to8(const __m256i v) { 					//map 6-bits bin to 8-bits ascii (https://arxiv.org/abs/1704.00605) 
  __m256i vidx = _mm256_subs_epu8(v,   _mm256_set1_epi8(51));
          vidx = _mm256_sub_epi8(vidx, _mm256_cmpgt_epi8(v, _mm256_set1_epi8(25)));

  const __m256i offsets = _mm256_set_epi8(0, 0, -16, -19, -4, -4, -4, -4,   -4, -4, -4, -4, -4, -4, 71, 65,
                                          0, 0, -16, -19, -4, -4, -4, -4,   -4, -4, -4, -4, -4, -4, 71, 65);
  return _mm256_add_epi8(v, _mm256_shuffle_epi8(offsets, vidx));
}

static ALWAYS_INLINE __m256i mm256_unpack6to8(__m256i v) { 						//https://arxiv.org/abs/1704.00605 p.12
  __m256i va = _mm256_mulhi_epu16(_mm256_and_si256(v, _mm256_set1_epi32(0x0fc0fc00)), _mm256_set1_epi32(0x04000040));
  __m256i vb = _mm256_mullo_epi16(_mm256_and_si256(v, _mm256_set1_epi32(0x003f03f0)), _mm256_set1_epi32(0x01000010));
  return _mm256_or_si256(va, vb);
}

#define ES256(_i_) {\
  __m256i v0 = _mm256_castsi128_si256(    _mm_loadu_si128((__m128i *)(ip+48+_i_*96+ 0))  );\
          v0 = _mm256_inserti128_si256(v0,_mm_loadu_si128((__m128i *)(ip+48+_i_*96+12)),1);\
  __m256i v1 = _mm256_castsi128_si256(    _mm_loadu_si128((__m128i *)(ip+48+_i_*96+24))  );\
          v1 = _mm256_inserti128_si256(v1,_mm_loadu_si128((__m128i *)(ip+48+_i_*96+36)),1);\
                                                                                           \
  u0 = _mm256_shuffle_epi8(u0, shuf); u0 = mm256_unpack6to8(u0); u0 = mm256_map6to8(u0);\
  u1 = _mm256_shuffle_epi8(u1, shuf); u1 = mm256_unpack6to8(u1); u1 = mm256_map6to8(u1);\
       _mm256_storeu_si256((__m256i*)(op+_i_*128),    u0);                              \
       _mm256_storeu_si256((__m256i*)(op+_i_*128+32), u1);                              \
		                                                                                   \
          u0 = _mm256_castsi128_si256(    _mm_loadu_si128((__m128i *)(ip+48+_i_*96+48))  );\
          u0 = _mm256_inserti128_si256(u0,_mm_loadu_si128((__m128i *)(ip+48+_i_*96+60)),1);\
          u1 = _mm256_castsi128_si256(    _mm_loadu_si128((__m128i *)(ip+48+_i_*96+72))  );\
          u1 = _mm256_inserti128_si256(u1,_mm_loadu_si128((__m128i *)(ip+48+_i_*96+84)),1); \
                                                                                           \
  v0 = _mm256_shuffle_epi8(v0, shuf); v0 = mm256_unpack6to8(v0); v0 = mm256_map6to8(v0);\
  v1 = _mm256_shuffle_epi8(v1, shuf); v1 = mm256_unpack6to8(v1); v1 = mm256_map6to8(v1); \
       _mm256_storeu_si256((__m256i*)(op+_i_*128+64), v0);\
       _mm256_storeu_si256((__m256i*)(op+_i_*128+96), v1);\
}

size_t tb64v256enc(const unsigned char *__restrict in, size_t inlen, unsigned char *__restrict out) {
  const unsigned char *ip = in; 
        unsigned char *op = out;
            size_t outlen = TB64ENCLEN(inlen);
  #define EN 256
       const __m256i shuf = _mm256_set_epi8(10,11, 9,10, 7, 8, 6, 7, 4,   5, 3, 4, 1, 2, 0, 1,
                                            10,11, 9,10, 7, 8, 6, 7, 4,   5, 3, 4, 1, 2, 0, 1);
  if(outlen > 64+128) {
      __m256i u0 = _mm256_castsi128_si256(    _mm_loadu_si128((__m128i *) ip    )  );   
              u0 = _mm256_inserti128_si256(u0,_mm_loadu_si128((__m128i *)(ip+12)),1);   
      __m256i u1 = _mm256_castsi128_si256(    _mm_loadu_si128((__m128i *)(ip+24))  );      
              u1 = _mm256_inserti128_si256(u1,_mm_loadu_si128((__m128i *)(ip+36)),1);   
    for(; op < (out+outlen)-(64+EN); op += EN, ip += EN*3/4) {
      ES256(0); 
	    #if EN > 128
	  ES256(1);
	    #endif
	  PREFETCH(ip, 384, 0);
    }
	  #if EN > 128
    if(op < (out+outlen)-(64+128)) { ES256(0); op += 128; ip += 128*3/4; }
      #endif	
  }  
  for(; op < out+outlen-32; op += 32, ip += 32*3/4) {
    __m256i v = _mm256_castsi128_si256(   _mm_loadu_si128((__m128i *) ip    )  );      
            v = _mm256_inserti128_si256(v,_mm_loadu_si128((__m128i *)(ip+12)),1);   
            v = _mm256_shuffle_epi8(v, shuf); v = mm256_unpack6to8(v); v = mm256_map6to8(v);                                                                                                           
                _mm256_storeu_si256((__m256i*) op, v);                                                 
  }
  /*for(; op <= (out+outlen)-16; op += 16, ip += (16/4)*3) {
    const __m128i    shuf = _mm_set_epi8(10,11, 9,10, 7, 8, 6, 7, 4,   5, 3, 4, 1, 2, 0, 1);
    __m128i v0 = _mm_loadu_si128((__m128i*)ip);
            v0 = _mm_shuffle_epi8(v0, shuf);
            v0 = mm_unpack6to8(v0);
            v0 = mm_map6to8(v0);
    _mm_storeu_si128((__m128i*) op, v0);                                          
  }*/
  EXTAIL();
  return outlen;
}

//------- optimized functions for short strings only --------------------------
// can read beyond the input buffer end, 
// therefore input buffer size must be 32 bytes larger than input length

size_t _tb64v256dec(const unsigned char *in, size_t inlen, unsigned char *out) {
  if(inlen >= 16) { 
    const unsigned char *ip;
          unsigned char *op; 
    const __m256i delta_asso   = _mm256_setr_epi8(0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,   0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x0f,
                                                  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,   0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x0f);
    const __m256i delta_values = _mm256_setr_epi8(0x00, 0x00, 0x00, 0x13, 0x04, 0xbf, 0xbf, 0xb9,   0xb9, 0x00, 0x10, 0xc3, 0xbf, 0xbf, 0xb9, 0xb9,
                                                  0x00, 0x00, 0x00, 0x13, 0x04, 0xbf, 0xbf, 0xb9,   0xb9, 0x00, 0x10, 0xc3, 0xbf, 0xbf, 0xb9, 0xb9);
      #ifndef NB64CHECK
    const __m256i check_asso   = _mm256_setr_epi8(0x0d, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,   0x01, 0x01, 0x03, 0x07, 0x0b, 0x0b, 0x0b, 0x0f,
                                                  0x0d, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,   0x01, 0x01, 0x03, 0x07, 0x0b, 0x0b, 0x0b, 0x0f);
    const __m256i check_values = _mm256_setr_epi8(0x80, 0x80, 0x80, 0x80, 0xcf, 0xbf, 0xd5, 0xa6,   0xb5, 0x86, 0xd1, 0x80, 0xb1, 0x80, 0x91, 0x80,
                                                  0x80, 0x80, 0x80, 0x80, 0xcf, 0xbf, 0xd5, 0xa6,   0xb5, 0x86, 0xd1, 0x80, 0xb1, 0x80, 0x91, 0x80);
      #endif
    const __m256i          cpv = _mm256_set_epi8( -1, -1, -1, -1, 12, 13, 14,  8,    9, 10,  4,  5,  6,  0,  1,  2,
                                                  -1, -1, -1, -1, 12, 13, 14,  8,    9, 10,  4,  5,  6,  0,  1,  2);
    __m256i vx = _mm256_setzero_si256();
    for(ip = in, op = out; ip < (in+inlen)-32; ip += 32, op += (32/4)*3) {
      __m256i          iv0 = _mm256_loadu_si256((__m256i *)ip);
      __m256i ov0,shifted0; MM256_MAP8TO6(iv0, shifted0, delta_asso, delta_values, ov0); MM256_PACK8TO6(ov0, cpv);
      
      _mm_storeu_si128((__m128i*) op,       _mm256_castsi256_si128(ov0));
      _mm_storeu_si128((__m128i*)(op + 12), _mm256_extracti128_si256(ov0, 1));                          
      CHECK0(MM256_B64CHK(iv0, shifted0, check_asso, check_values, vx));
    }

    unsigned cx;
    if(ip < (in+inlen)-16) {
      __m128i iv0 = _mm_loadu_si128((__m128i *) ip);
      __m128i _vx = _mm_or_si128(_mm256_extracti128_si256(vx, 1), _mm256_castsi256_si128(vx));
      __m128i ov0, shifted0; MM_MAP8TO6( iv0, shifted0,_mm256_castsi256_si128(delta_asso),_mm256_castsi256_si128(delta_values), ov0); 
	                         MM_PACK8TO6(ov0, _mm256_castsi256_si128(cpv));
      _mm_storeu_si128((__m128i*) op, ov0);  
      ip += 16; op += (16/4)*3;
      CHECK0(MM_B64CHK(iv0, shifted0, _mm256_castsi256_si128(check_asso), _mm256_castsi256_si128(check_values), _vx));
      cx = _mm_movemask_epi8(_vx);
    } else
      cx = _mm256_movemask_epi8(vx);

    size_t rc = _tb64xd(ip, inlen-(ip-in), op);
    if(!rc || cx) return 0;
    return (op-out)+rc;
  }
  return _tb64xd(in, inlen, out);
}

size_t _tb64v256enc(const unsigned char* in, size_t inlen, unsigned char *out) {
  const unsigned char *ip = in; 
        unsigned char *op = out;
        size_t   outlen = TB64ENCLEN(inlen);
  if(outlen >= 32+4) { 
    const __m256i    shuf = _mm256_set_epi8(10,11, 9,10, 7, 8, 6, 7, 4,   5, 3, 4, 1, 2, 0, 1,
                                            10,11, 9,10, 7, 8, 6, 7, 4,   5, 3, 4, 1, 2, 0, 1);

    for(; op <= (out+outlen)-32; op += 32, ip += (32/4)*3) {
      __m256i v0 = _mm256_castsi128_si256(    _mm_loadu_si128((__m128i *) ip));      
              v0 = _mm256_inserti128_si256(v0,_mm_loadu_si128((__m128i *)(ip+12)),1);   
      v0 = _mm256_shuffle_epi8(v0, shuf); v0 = mm256_unpack6to8(v0); v0 = mm256_map6to8(v0);                                                                                                           
      _mm256_storeu_si256((__m256i*) op,     v0);                                            
    }
  }
  if(op <= (out+outlen)-(16+4)) {
    const __m128i    shuf = _mm_set_epi8(10,11, 9,10, 7, 8, 6, 7, 4,   5, 3, 4, 1, 2, 0, 1);

    __m128i v0 = _mm_loadu_si128((__m128i*)ip);
            v0 = _mm_shuffle_epi8(v0, shuf);
            v0 = mm_unpack6to8(v0);
            v0 = mm_map6to8(v0);
    _mm_storeu_si128((__m128i*) op, v0);                                          
    op += 16; ip += (16/4)*3;
  }
  EXTAIL();
  return outlen;
}
