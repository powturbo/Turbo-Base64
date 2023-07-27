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
// Turbo-Base64: internal include
//#define UA_MEMCPY // Force replace unaligned stores with memcpy (see "conf.h")
#include "conf.h"

size_t _tb64xdec( const unsigned char *in, size_t inlen, unsigned char *out);
size_t tb64memcpy(const unsigned char *in, size_t inlen, unsigned char *out);  // testing only

#define PREFETCH(_ip_,_i_,_rw_) __builtin_prefetch(_ip_+(_i_),_rw_)

  #if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define BSWAP32(a) a 
#define BSWAP64(a) a 
  #else
#define BSWAP32(a) bswap32(a)
#define BSWAP64(a) bswap64(a)
  #endif  

  #ifdef NB64CHECK  // decoding without checking
#define CHECK0(a)
#define CHECK1(a)
  #else             // decoding incl. checking 
#define CHECK0(a) a
    #ifdef B64CHECK // Full check
#define CHECK1(a) a
    #else
#define CHECK1(a)
    #endif
  #endif

//------- Encode: scalar helper macros & functions ----------------------------------------------------------
extern unsigned char tb64lutse[];

#define SU32(_u_) (tb64lutse[(_u_>> 8) & 0x3f] << 24 |\
                   tb64lutse[(_u_>>14) & 0x3f] << 16 |\
                   tb64lutse[(_u_>>20) & 0x3f] <<  8 |\
                   tb64lutse[(_u_>>26) & 0x3f])

#define ETAIL()\
  unsigned _l = (in+inlen) - ip;   AS(ip <= in+inlen, "ETAIL:Fatal %d\n", (unsigned)(ip - (in+inlen)));\
       if(_l == 3) { unsigned _u = ip[0]<<24 | ip[1]<<16 | ip[2]<<8; stou32(op, SU32(_u)); op+=4; }\
  else if(_l == 2) { op[0] = tb64lutse[(ip[0]>>2)&0x3f]; op[1] = tb64lutse[(ip[0] & 0x3) << 4 | (ip[1] & 0xf0) >> 4]; op[2] = tb64lutse[(ip[1] & 0xf) << 2]; op[3] = '='; op+=4; }\
  else if(_l)      { op[0] = tb64lutse[(ip[0]>>2)&0x3f]; op[1] = tb64lutse[(ip[0] & 0x3) << 4],                       op[2] = '=';                           op[3] = '='; op+=4; }
  
extern const unsigned short tb64lute[];
#define XU32(_u_) (tb64lute[(_u_ >>  8) & 0xfff] << 16 |\
                   tb64lute[ _u_ >> 20])

#define EXTAIL(_n_) for(; op < out_-4; op += 4, ip += 3) { unsigned _u = BSWAP32(ctou32(ip)); stou32(op, XU32(_u)); } ETAIL()

//------- Decode: scalar helper macros & functions ----------------------------------------------------------
extern const unsigned tb64lutd0[];
extern const unsigned tb64lutd1[];
extern const unsigned tb64lutd2[];
extern const unsigned tb64lutd3[];

#define DU32(_u_) (tb64lutd0[(unsigned char)(_u_     )] |\
                   tb64lutd1[(unsigned char)(_u_>>  8)] |\
                   tb64lutd2[(unsigned char)(_u_>> 16)] |\
                   tb64lutd3[                _u_>> 24 ] )

#define DXTAILC(ip,out,op,_check_) {\
       if(ip[3] != '=') { unsigned u = ctou32(ip); u = DU32(u);                                op[0] = u; op[1] = u>>8; op[2] = u>>16; op+=3; _check_; } /*4->3*/\
  else if(ip[2] != '=') { unsigned u = tb64lutd0[ip[0]] | tb64lutd1[ip[1]] | tb64lutd2[ip[2]]; op[0] = u; op[1] = u>>8; op+=2;                _check_; } /*3->2*/\
  else if(ip[1] != '=') { unsigned u = tb64lutd0[ip[0]] | tb64lutd1[ip[1]];                    *op++ = u;                                     _check_; } /*2->1*/\
  else                  { unsigned u = tb64lutd0[ip[0]];                                       *op++ = u;                                     _check_; } /*1->1*/\
}
				   
#define DXTAIL(ip,out,op) {\
       if(ip[3] != '=') { unsigned u = ctou32(ip); u = DU32(u);                                op[0] = u; op[1] = u>>8; op[2] = u>>16; op+=3;} /*4->3*/\
  else if(ip[2] != '=') { uint16_t u = tb64lutd0[ip[0]] | tb64lutd1[ip[1]] | tb64lutd2[ip[2]]; op[0] = u; op[1] = u>>8; op+=2;               } /*3->2*/\
  else if(ip[1] != '=') {                                                                      *op++ = tb64lutd0[ip[0]] | tb64lutd1[ip[1]];  } /*2->1*/\
  else                  {                                                                      *op++ = tb64lutd0[ip[0]];                     } /*1->1*/\
}

static ALWAYS_INLINE size_t _tb64xd(const unsigned char *in, size_t inlen, unsigned char *out) { 
  const unsigned char *ip = in, *in_ = in+inlen;
        unsigned char *op = out;
    #ifdef B64CHECK
  unsigned       cu = 0;
  for(; ip < in_-4; ip += 4, op += 3) { unsigned u = ctou32(ip); u = DU32(u); stou32(op, u); cu |= u; }
  DXTAILC(ip,out,op, cu |= u);
  return (cu == -1)?0:(op-out);
    #else
  for(; ip < in_-4; ip += 4, op += 3) { unsigned u = ctou32(ip); u = DU32(u); stou32(op, u); } 
  DXTAIL(ip,out,op)
  return op - out;
    #endif		
}

//------- SSE helper macros & functions ----------------------------------------------------------
#if defined(__SSSE3__)
#include <tmmintrin.h>
#define BITPACK128V8_6(v, cpv) {\
  const __m128i merge_ab_bc = _mm_maddubs_epi16(v,            _mm_set1_epi32(0x01400140));  /*dec_reshuffle: https://arxiv.org/abs/1704.00605 P.17*/\
                          v = _mm_madd_epi16(merge_ab_bc, _mm_set1_epi32(0x00011000));\
                          v = _mm_shuffle_epi8(v, cpv);\
}

#define BITMAP128V8_6(iv, shifted, delta_asso, delta_values, ov) { /*map 8-bits ascii to 6-bits binary*/\
                shifted    = _mm_srli_epi32(iv, 3);\
  const __m128i delta_hash = _mm_avg_epu8(_mm_shuffle_epi8(delta_asso, iv), shifted);\
                        ov = _mm_add_epi8(_mm_shuffle_epi8(delta_values, delta_hash), iv);\
}

#define B64CHK128(iv, shifted, check_asso, check_values, vx) {\
  const __m128i check_hash = _mm_avg_epu8( _mm_shuffle_epi8(check_asso, iv), shifted);\
  const __m128i        chk = _mm_adds_epi8(_mm_shuffle_epi8(check_values, check_hash), iv);\
                        vx = _mm_or_si128(vx, chk);\
}

static ALWAYS_INLINE __m128i bitmap128v8_6(const __m128i v) { /*map 8-bits ascii to 6-bits binary*/
  const __m128i offsets = _mm_set_epi8( 0, 0,-16,-19, -4,-4,-4,-4,   -4,-4,-4,-4, -4,-4,71,65);

  __m128i vidx = _mm_subs_epu8(v,   _mm_set1_epi8(51));
          vidx = _mm_sub_epi8(vidx, _mm_cmpgt_epi8(v, _mm_set1_epi8(25)));
  return _mm_add_epi8(v, _mm_shuffle_epi8(offsets, vidx));
}

static ALWAYS_INLINE __m128i bitunpack128v8_6(__m128i v) { /* unpack 6 -> 8 */
  __m128i va = _mm_mulhi_epu16(_mm_and_si128(v, _mm_set1_epi32(0x0fc0fc00)), _mm_set1_epi32(0x04000040));
  __m128i vb = _mm_mullo_epi16(_mm_and_si128(v, _mm_set1_epi32(0x003f03f0)), _mm_set1_epi32(0x01000010));
  return       _mm_or_si128(va, vb);                        
}
#endif
//------- avx2 helper macros & functions ----------------------------------------------------------
#ifdef __AVX2__
static ALWAYS_INLINE __m256i bitmap256v8_6(const __m256i v) { 					//map 8-bits ascii to 6-bits binary (https://arxiv.org/abs/1704.00605) 
  __m256i vidx = _mm256_subs_epu8(v,   _mm256_set1_epi8(51));
          vidx = _mm256_sub_epi8(vidx, _mm256_cmpgt_epi8(v, _mm256_set1_epi8(25)));

  const __m256i offsets = _mm256_set_epi8(0, 0, -16, -19, -4, -4, -4, -4,   -4, -4, -4, -4, -4, -4, 71, 65,
                                          0, 0, -16, -19, -4, -4, -4, -4,   -4, -4, -4, -4, -4, -4, 71, 65);
  return _mm256_add_epi8(v, _mm256_shuffle_epi8(offsets, vidx));
}

static ALWAYS_INLINE __m256i bitunpack256v8_6(__m256i v) { 						//https://arxiv.org/abs/1704.00605 p.12
  __m256i va = _mm256_mulhi_epu16(_mm256_and_si256(v, _mm256_set1_epi32(0x0fc0fc00)), _mm256_set1_epi32(0x04000040));
  __m256i vb = _mm256_mullo_epi16(_mm256_and_si256(v, _mm256_set1_epi32(0x003f03f0)), _mm256_set1_epi32(0x01000010));
  return _mm256_or_si256(va, vb);
}
#endif
