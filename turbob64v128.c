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
//  Turbo-Base64: ssse3 + arm neon functions (see also turbob64v256)

#include <string.h>

  #if defined(__AVX__)
#include <immintrin.h>
#define FUNPREF tb64v128a
  #elif defined(__SSE4_1__)
#include <smmintrin.h>
#define FUNPREF tb64v128
  #elif defined(__SSSE3__)
    #ifdef __powerpc64__
#define __SSE__   1
#define __SSE2__  1
#define __SSE3__  1
#define NO_WARN_X86_INTRINSICS 1
    #endif
#define FUNPREF tb64v128
#include <tmmintrin.h>
  #elif defined(__SSE2__)
#include <emmintrin.h>
  #elif defined(__ARM_NEON)
#include <arm_neon.h>
  #endif
  
#include "turbob64_.h"
#include "turbob64.h"

#ifdef __ARM_NEON  //----------------------------------- arm neon --------------------------------

#define _ 0xff // invald entry
static const unsigned char lut[] = {
 _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,
 _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,
 _, _, _, _, _, _, _, _, _, _, _,62, _, _, _,63,
52,53,54,55,56,57,58,59,60,61, _, _, _, _, _, _,
 _, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
15,16,17,18,19,20,21,22,23,24,25, _, _, _, _, _,
 _,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
41,42,43,44,45,46,47,48,49,50,51, _, _, _, _, _,
};
#undef _

#if defined(__GNUC__) && !defined(__clang__) && (__GNUC__ == 10 && __GNUC_MINOR__ <= 2 || \
                                                 __GNUC__ ==  9 && __GNUC_MINOR__ <= 3 || \
                                                 __GNUC__ ==  8 && __GNUC_MINOR__ <= 4 || \
												 __GNUC__ <= 7)
static inline uint8x16x4_t vld1q_u8_x4(const uint8_t *lut) {
  uint8x16x4_t v;
  v.val[0] = vld1q_u8(lut);
  v.val[1] = vld1q_u8(lut+16);
  v.val[2] = vld1q_u8(lut+32);
  v.val[3] = vld1q_u8(lut+48);
  return v;
}
  #endif

#define B64D(iv, ov) {\
    iv.val[0] = vqtbx4q_u8(vqtbl4q_u8(vlut1, veorq_u8(iv.val[0], cv40)), vlut0, iv.val[0]);\
    iv.val[1] = vqtbx4q_u8(vqtbl4q_u8(vlut1, veorq_u8(iv.val[1], cv40)), vlut0, iv.val[1]);\
    iv.val[2] = vqtbx4q_u8(vqtbl4q_u8(vlut1, veorq_u8(iv.val[2], cv40)), vlut0, iv.val[2]);\
    iv.val[3] = vqtbx4q_u8(vqtbl4q_u8(vlut1, veorq_u8(iv.val[3], cv40)), vlut0, iv.val[3]);\
\
	ov.val[0] = vorrq_u8(vshlq_n_u8(iv.val[0], 2), vshrq_n_u8(iv.val[1], 4));\
	ov.val[1] = vorrq_u8(vshlq_n_u8(iv.val[1], 4), vshrq_n_u8(iv.val[2], 2));\
	ov.val[2] = vorrq_u8(vshlq_n_u8(iv.val[2], 6),            iv.val[3]    );\
}

#define _B64CHK128(iv, xv) xv = vorrq_u8(xv, vorrq_u8(vorrq_u8(iv.val[0], iv.val[1]), vorrq_u8(iv.val[2], iv.val[3])))

size_t tb64v128dec(const unsigned char *in, size_t inlen, unsigned char *out) {
  const unsigned char *ip;
        unsigned char *op; 
  const uint8x16x4_t vlut0 = vld1q_u8_x4( lut),
                     vlut1 = vld1q_u8_x4(&lut[64]);
  const uint8x16_t    cv40 = vdupq_n_u8(0x40);
        uint8x16_t      xv = vdupq_n_u8(0);
  #define DN 256
  for(ip = in, op = out; ip != in+(inlen&~(DN-1)); ip += DN, op += (DN/4)*3) { PREFETCH(ip,256,0);	
    uint8x16x4_t iv0 = vld4q_u8(ip),
                 iv1 = vld4q_u8(ip+64);                                                    
	uint8x16x3_t ov0,ov1; 
    B64D(iv0, ov0);
      #if DN > 128
	CHECK1(_B64CHK128(iv0,xv));
      #else
	CHECK0(_B64CHK128(iv0,xv));
      #endif
	B64D(iv1, ov1); CHECK1(_B64CHK128(iv1,xv));
      #if DN > 128
    iv0 = vld4q_u8(ip+128);
    iv1 = vld4q_u8(ip+192);              
      #endif
	vst3q_u8(op,    ov0);       
	vst3q_u8(op+48, ov1);                                                                                                                                                                       
      #if DN > 128
	B64D(iv0,ov0);	CHECK1(_B64CHK128(iv0,xv));
	B64D(iv1,ov1); 
	vst3q_u8(op+ 96, ov0);       
	vst3q_u8(op+144, ov1);                                                                                                                                                                       
	CHECK0(_B64CHK128(iv1,xv));
      #endif
  }
  for(                 ; ip != in+(inlen&~(64-1)); ip += 64, op += (64/4)*3) { 	
    uint8x16x4_t iv = vld4q_u8(ip);
	uint8x16x3_t ov; B64D(iv,ov);
	vst3q_u8(op, ov);                                                                                                                          
	CHECK0(xv = vorrq_u8(xv, vorrq_u8(vorrq_u8(iv.val[0], iv.val[1]), vorrq_u8(iv.val[2], iv.val[3]))));
  }
  size_t rc = 0, r = inlen&(64-1); 
  if(r && !(rc=tb64xdec(ip, r, op)) || vaddvq_u8(vshrq_n_u8(xv,7))) { return 0; }//decode all
  return (op - out) + rc; 
}

//--------------------------------------------------------------------------------------------------
#define B64E(iv, ov) {\
  ov.val[0] =                                             vshrq_n_u8(iv.val[0], 2);\
  ov.val[1] = vandq_u8(vorrq_u8(vshlq_n_u8(iv.val[0], 4), vshrq_n_u8(iv.val[1], 4)), cv3f);\
  ov.val[2] = vandq_u8(vorrq_u8(vshlq_n_u8(iv.val[1], 2), vshrq_n_u8(iv.val[2], 6)), cv3f);\
  ov.val[3] = vandq_u8(                    iv.val[2],                                cv3f);\
\
  ov.val[0] = vqtbl4q_u8(vlut, ov.val[0]);\
  ov.val[1] = vqtbl4q_u8(vlut, ov.val[1]);\
  ov.val[2] = vqtbl4q_u8(vlut, ov.val[2]);\
  ov.val[3] = vqtbl4q_u8(vlut, ov.val[3]);\
}

size_t tb64v128enc(const unsigned char *__restrict in, size_t inlen, unsigned char *__restrict out) {
  static unsigned char lut[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  const size_t      outlen = TB64ENCLEN(inlen);
  const unsigned char *ip, *out_ = out+outlen; 
        unsigned char *op;
  const uint8x16x4_t  vlut = vld1q_u8_x4(lut);
  const uint8x16_t    cv3f = vdupq_n_u8(0x3f);

  #define EN 128 // 256//
  for(ip = in, op = out; op != out+(outlen&~(EN-1)); op += EN, ip += (EN/4)*3) { 	 							
          uint8x16x3_t iv0 = vld3q_u8(ip),
                       iv1 = vld3q_u8(ip+48);                   

    uint8x16x4_t ov0,ov1; B64E(iv0, ov0); B64E(iv1, ov1);                                       
      #if EN > 128 
    iv0 = vld3q_u8(ip+ 96);
    iv1 = vld3q_u8(ip+144);                   
      #endif
	vst4q_u8(op,    ov0);                                                       
	vst4q_u8(op+64, ov1);                          	//PREFETCH(ip,256,0);                                                  
      #if EN > 128 
                         B64E(iv0, ov0); B64E(iv1, ov1);                                             
 	vst4q_u8(op+128, ov0);                                                       
	vst4q_u8(op+192, ov1);                          	                                            
          #endif
  }
  for(                 ; op != out+(outlen&~(64-1)); op += 64, ip += (64/4)*3) { 								
    const uint8x16x3_t iv = vld3q_u8(ip);
    uint8x16x4_t       ov; 
    B64E(iv, ov); 
	vst4q_u8(op,ov);                                                       
  } 
  EXTAIL();
  return outlen;
}

#elif defined(__SSSE3__) //----------------- SSSE3 / SSE4.1 / AVX (derived from the AVX2 functions ) -----------------------------------------------------------------
                //--------------- decode -------------------
#define DS64(_i_) {\
  __m128i iv0 = _mm_loadu_si128((__m128i *)(ip+32+_i_*64   )),\
          iv1 = _mm_loadu_si128((__m128i *)(ip+32+_i_*64+16));\
  \
  __m128i ou0,shifted0; BITMAP128V8_6(iu0, shifted0,delta_asso, delta_values, ou0); BITPACK128V8_6(ou0, cpv);\
  __m128i ou1,shifted1; BITMAP128V8_6(iu1, shifted1,delta_asso, delta_values, ou1); BITPACK128V8_6(ou1, cpv);\
  _mm_storeu_si128((__m128i*)(op+_i_*48)   , ou0);\
  _mm_storeu_si128((__m128i*)(op+_i_*48+12), ou1);\
  CHECK0(B64CHK128(iu0, shifted0, check_asso, check_values, vx));\
  CHECK1(B64CHK128(iu1, shifted1, check_asso, check_values, vx));\
  \
          iu0 = _mm_loadu_si128((__m128i *)(ip+32+_i_*64+32));\
          iu1 = _mm_loadu_si128((__m128i *)(ip+32+_i_*64+48));\
  \
  __m128i ov2,shifted2; BITMAP128V8_6(iv0, shifted2,delta_asso, delta_values, ov2); BITPACK128V8_6(ov2, cpv);\
  __m128i ov3,shifted3; BITMAP128V8_6(iv1, shifted3,delta_asso, delta_values, ov3); BITPACK128V8_6(ov3, cpv);\
  _mm_storeu_si128((__m128i*)(op+_i_*48+24), ov2);\
  _mm_storeu_si128((__m128i*)(op+_i_*48+36), ov3);\
  CHECK1(B64CHK128(iv0, shifted2, check_asso, check_values, vx));\
  CHECK1(B64CHK128(iv1, shifted3, check_asso, check_values, vx));\
}	

size_t T2(FUNPREF, dec)(const unsigned char *__restrict in, size_t inlen, unsigned char *__restrict out) {
  if(inlen&3) return 0;                                              

  const unsigned char *ip = in, *in_ = in+inlen;									  
        unsigned char *op = out;		
  __m128i vx = _mm_setzero_si128();	  
  const __m128i delta_asso   = _mm_setr_epi8(0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,  0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x0f);
  const __m128i delta_values = _mm_setr_epi8(0x00, 0x00, 0x00, 0x13, 0x04, 0xbf, 0xbf, 0xb9,  0xb9, 0x00, 0x10, 0xc3, 0xbf, 0xbf, 0xb9, 0xb9);
    #ifndef NB64CHECK
  const __m128i check_asso   = _mm_setr_epi8(0x0d, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,  0x01, 0x01, 0x03, 0x07, 0x0b, 0x0b, 0x0b, 0x0f);
  const __m128i check_values = _mm_setr_epi8(0x80, 0x80, 0x80, 0x80, 0xcf, 0xbf, 0xd5, 0xa6,  0xb5, 0x86, 0xd1, 0x80, 0xb1, 0x80, 0x91, 0x80);    
    #endif
  const __m128i          cpv = _mm_set_epi8( -1, -1, -1, -1, 12, 13, 14,  8,    9, 10,  4,  5,  6,  0,  1,  2);
  
  if(inlen >= 32+64+4) {
    __m128i iu0 = _mm_loadu_si128((__m128i *) ip    ),
            iu1 = _mm_loadu_si128((__m128i *)(ip+16));	     									
    for(; ip < in_-(32+2*64+4); ip += 128, op += 128*3/4) { DS64(0); DS64(1); }						
    if(   ip < in_-(32+  64+4)) { DS64(0); ip += 64, op += 64*3/4; }						
  } else if(!inlen) return 0;
  
  for(; ip < in_-(16+4); ip += 16, op += 16*3/4) { 											
    __m128i iv = _mm_loadu_si128((__m128i *)ip), ov, shifted0;     								
	BITMAP128V8_6(iv, shifted0, delta_asso, delta_values, ov);
	BITPACK128V8_6(ov, cpv);
    _mm_storeu_si128((__m128i*) op, ov); 														                                              
    CHECK0(B64CHK128(iv, shifted0, check_asso, check_values, vx));
  } 

  unsigned cx =  _mm_movemask_epi8(vx);
  size_t rc = 0, r = in_ - ip;
  if(r && !(rc = _tb64xd(ip, r, op)) || cx) 
	return 0;
  return (op - out)+rc;
}

                         //---------------------- encode ------------------
#define ES64(_i_) {\
      __m128i v0 = _mm_loadu_si128((__m128i*)(ip+24+_i_*48+ 0)),\
              v1 = _mm_loadu_si128((__m128i*)(ip+24+_i_*48+12));\
\
              u0 = _mm_shuffle_epi8(u0, shuf);\
              u1 = _mm_shuffle_epi8(u1, shuf);\
              u0 = bitunpack128v8_6(u0);\
              u1 = bitunpack128v8_6(u1);\
              u0 = bitmap128v8_6(u0);\
              u1 = bitmap128v8_6(u1);\
      _mm_storeu_si128((__m128i*)(op+_i_*64+ 0), u0);\
      _mm_storeu_si128((__m128i*)(op+_i_*64+16), u1);\
\
              u0 = _mm_loadu_si128((__m128i*)(ip+24+_i_*48+24));\
              u1 = _mm_loadu_si128((__m128i*)(ip+24+_i_*48+36));\
\
              v0 = _mm_shuffle_epi8(v0, shuf);\
              v1 = _mm_shuffle_epi8(v1, shuf);\
              v0 = bitunpack128v8_6(v0);\
              v1 = bitunpack128v8_6(v1); \
              v0 = bitmap128v8_6(v0);\
              v1 = bitmap128v8_6(v1);\
      _mm_storeu_si128((__m128i*)(op+_i_*64+32), v0);\
      _mm_storeu_si128((__m128i*)(op+_i_*64+48), v1);\
}  

size_t T2(FUNPREF, enc)(const unsigned char *__restrict in, size_t inlen, unsigned char *__restrict out) { 
  const size_t        outlen = TB64ENCLEN(inlen); 
  const unsigned char *ip = in, *out_ = out+outlen; 
        unsigned char *op = out;

  const __m128i shuf = _mm_set_epi8(10,11,  9,10,  7, 8, 6, 7,    4, 5, 3, 4, 1, 2, 0, 1);
  
  if(outlen >= (24+48+4)*4/3) {
      __m128i u0 = _mm_loadu_si128((__m128i*) ip),
              u1 = _mm_loadu_si128((__m128i*)(ip+12)); 
    for(; op < out_-(24+2*48+4)*4/3; op += 128, ip += 128*3/4) { ES64(0); ES64(1); }  
    if(   op < out_-(24+  48+4)*4/3) { ES64(0); op +=  64; ip +=  64*3/4; }		          						   
  }
  
  for(; op < out_- (12+4)*4/3; op += 16, ip += 16*3/4) {
	__m128i v = _mm_loadu_si128((__m128i*)ip);
            v = _mm_shuffle_epi8(v, shuf);
            v =  bitunpack128v8_6(v);
            v =  bitmap128v8_6(v);
    _mm_storeu_si128((__m128i*)op, v);
  }					
  
  EXTAIL(3); 															 							 
  return outlen;
}
#endif
//-------------------------------------------------------------------------------------------------------------------
#ifndef __AVX__ //include only 1 time
size_t tb64memcpy(const unsigned char* in, size_t inlen, unsigned char *out) {
  memcpy(out, in, inlen);
  return inlen;
}
 
static unsigned _cpuisa;
//--------------------- CPU detection -------------------------------------------
    #if defined(__i386__) || defined(__x86_64__)
      #if _MSC_VER >=1300
#include <intrin.h>
      #elif defined (__INTEL_COMPILER)
#include <x86intrin.h>
      #endif

static inline void cpuid(int reg[4], int id) {
      #if defined (_MSC_VER) //|| defined (__INTEL_COMPILER)
  __cpuidex(reg, id, 0);
      #elif defined(__i386__) || defined(__x86_64__)
  __asm("cpuid" : "=a"(reg[0]),"=b"(reg[1]),"=c"(reg[2]),"=d"(reg[3]) : "a"(id),"c"(0) : );
      #endif
}

static inline uint64_t xgetbv (int ctr) {
      #if(defined _MSC_VER && (_MSC_FULL_VER >= 160040219) || defined __INTEL_COMPILER)
  return _xgetbv(ctr);
      #elif defined(__i386__) || defined(__x86_64__)
  unsigned a, d;
  __asm("xgetbv" : "=a"(a),"=d"(d) : "c"(ctr) : );
  return (uint64_t)d << 32 | a;
      #else
  unsigned a=0, d=0;
  return (uint64_t)d << 32 | a;
      #endif
}
    #endif

#define AVX512F     0x001
#define AVX512DQ    0x002
#define AVX512IFMA  0x004
#define AVX512PF    0x008
#define AVX512ER    0x010
#define AVX512CD    0x020
#define AVX512BW    0x040
#define AVX512VL    0x080
#define AVX512VNNI  0x100
#define AVX512VBMI  0x200
#define AVX512VBMI2 0x400

#define IS_SSE       0x10
#define IS_SSE2      0x20
#define IS_SSE3      0x30
#define IS_SSSE3     0x32
#define IS_POWER9    0x34 // powerpc
#define IS_NEON      0x38 // arm neon
#define IS_SSE41     0x40
#define IS_SSE41x    0x41 //+popcount
#define IS_SSE42     0x42
#define IS_AVX       0x50
#define IS_AVX2      0x60
#define IS_AVX512    0x800

unsigned cpuisa(void) {
  int c[4] = {0};
  if(_cpuisa) return _cpuisa;
  _cpuisa++;
    #if defined(__i386__) || defined(__x86_64__)
  cpuid(c, 0);
  if(c[0]) {
    cpuid(c, 1);
    //family = ((c >> 8) & 0xf) + ((c >> 20) & 0xff)
    //model  = ((c >> 4) & 0xf) + ((c >> 12) & 0xf0)
    if( c[3] & (1 << 25)) {         _cpuisa  = IS_SSE;
    if( c[3] & (1 << 26)) {         _cpuisa  = IS_SSE2;
    if( c[2] & (1 <<  0)) {         _cpuisa  = IS_SSE3;
      //                            _cpuisa  = IS_SSE3SLOW; // Atom SSSE3 slow
    if( c[2] & (1 <<  9)) {         _cpuisa  = IS_SSSE3;
    if( c[2] & (1 << 19)) {         _cpuisa  = IS_SSE41;
    if( c[2] & (1 << 23)) {         _cpuisa  = IS_SSE41x; // +popcount
    if( c[2] & (1 << 20)) {         _cpuisa  = IS_SSE42;  // SSE4.2
    if((c[2] & (1 << 28)) &&
       (c[2] & (1 << 27)) &&                           // OSXSAVE
       (c[2] & (1 << 26)) &&                           // XSAVE
       (xgetbv(0) & 6)==6) {        _cpuisa  = IS_AVX; // AVX
      if(c[2]& (1 <<  3))           _cpuisa |= 1;      // +FMA3
      if(c[2]& (1 << 16))           _cpuisa |= 2;      // +FMA4
      if(c[2]& (1 << 25))           _cpuisa |= 4;      // +AES
      cpuid(c, 7);
      if(c[1] & (1 << 5)) {         _cpuisa = IS_AVX2;
        if(c[1] & (1 << 16)) {
          cpuid(c, 0xd);
          if((c[0] & 0x60)==0x60) { _cpuisa = IS_AVX512;
            cpuid(c, 7);
            if(c[1] & (1<<16))      _cpuisa |= AVX512F;
            if(c[1] & (1<<17))      _cpuisa |= AVX512DQ;
            if(c[1] & (1<<21))      _cpuisa |= AVX512IFMA;
            if(c[1] & (1<<26))      _cpuisa |= AVX512PF;
            if(c[1] & (1<<27))      _cpuisa |= AVX512ER;
            if(c[1] & (1<<28))      _cpuisa |= AVX512CD;
            if(c[1] & (1<<30))      _cpuisa |= AVX512BW;
            if(c[1] & (1u<<31))     _cpuisa |= AVX512VL;
            if(c[2] & (1<< 1))      _cpuisa |= AVX512VBMI;
            if(c[2] & (1<<11))      _cpuisa |= AVX512VNNI;
            if(c[2] & (1<< 6))      _cpuisa |= AVX512VBMI2;
      }}}
    }}}}}}}}}
    #elif defined(__powerpc64__)
  _cpuisa = IS_POWER9; // power9
    #elif defined(__ARM_NEON)
  _cpuisa = IS_NEON; // ARM_NEON
    #endif
  return _cpuisa;
}

unsigned cpuini(unsigned cpuisa) { if(cpuisa) _cpuisa = cpuisa; return _cpuisa; }

char *cpustr(unsigned cpuisa) {
  if(!cpuisa) cpuisa = _cpuisa;
    #if defined(__i386__) || defined(__x86_64__)
  if(cpuisa >= IS_AVX512) {
    if(cpuisa & AVX512VBMI2) return "avx512vbmi2";
    if(cpuisa & AVX512VBMI)  return "avx512vbmi";
    if(cpuisa & AVX512VNNI)  return "avx512vnni";
    if(cpuisa & AVX512VL)    return "avx512vl";
    if(cpuisa & AVX512BW)    return "avx512bw";
    if(cpuisa & AVX512CD)    return "avx512cd";
    if(cpuisa & AVX512ER)    return "avx512er";
    if(cpuisa & AVX512PF)    return "avx512pf";
    if(cpuisa & AVX512IFMA)  return "avx512ifma";
    if(cpuisa & AVX512DQ)    return "avx512dq";
    if(cpuisa & AVX512F)     return "avx512f";
    return "avx512";
  }
  else if(cpuisa >= IS_AVX2)    return "avx2";
  else if(cpuisa >= IS_AVX)
    switch(cpuisa&0xf) {
      case 1: return "avx+fma3";
      case 2: return "avx+fma4";
      case 4: return "avx+aes";
      case 5: return "avx+fma3+aes";
      default:return "avx";
    }
  else if(cpuisa >= IS_SSE42)   return "sse4.2";
  else if(cpuisa >= IS_SSE41x)  return "sse4.1+popcnt";
  else if(cpuisa >= IS_SSE41)   return "sse4.1";
  else if(cpuisa >= IS_SSSE3)   return "ssse3";
  else if(cpuisa >= IS_SSE3)    return "sse3";
  else if(cpuisa >= IS_SSE2)    return "sse2";
  else if(cpuisa >= IS_SSE)     return "sse";
     #elif defined(__powerpc64__)
  if(cpuisa >= IS_POWER9)       return "power9";
    #elif defined(__ARM_NEON)
  if(cpuisa >= IS_NEON)         return "arm_neon";
    #endif
  return "none";
}

//---------------------------------------------------------------------------------
TB64FUNC _tb64e = tb64xenc;
TB64FUNC _tb64d = tb64xdec;

static int tb64set;
 
void tb64ini(unsigned id, unsigned isshort) { 
  int i; 
  if(tb64set) return; 
  tb64set++;   
  i = id?id:cpuisa();
    #if defined(__i386__) || defined(__x86_64__)
      #ifndef NAVX512
  if(i >= IS_AVX512) {  
    _tb64e = i >= (IS_AVX512|AVX512VBMI)?tb64v512enc:tb64v256enc; 
    _tb64d = i >= (IS_AVX512|AVX512VBMI)?tb64v512dec:tb64v256dec;
  } else 
      #endif
      #ifndef NAVX2
  if(i >= IS_AVX2) {  
    _tb64e = isshort?_tb64v256enc:tb64v256enc; 
    _tb64d = isshort?_tb64v256dec:tb64v256dec;
  } else 
      #endif
      #ifndef NAVX
    if(i >= IS_AVX) {  
    _tb64e = tb64v128aenc; 
    _tb64d = tb64v128adec;
  } else 
      #endif
    #endif
    #if defined(__i386__) || defined(__x86_64__) || defined(__ARM_NEON) || defined(__powerpc64__)
      #ifndef NSSE
  if(i >= IS_SSSE3) {  
    _tb64e = tb64v128enc; 
    _tb64d = tb64v128dec;
  }
      #endif
    #endif
}

size_t tb64enc(const unsigned char *in, size_t inlen, unsigned char *out) {
  if(!tb64set) tb64ini(0,0);
  return _tb64e(in,inlen,out);
}
size_t tb64dec(const unsigned char *in, size_t inlen, unsigned char *out) {
  if(!tb64set) tb64ini(0,0);
  return _tb64d(in,inlen,out);
}
#endif
