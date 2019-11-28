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
// TubeBase64: ssse3 + arm neon functions 

  #if defined(__AVX__)
#include <immintrin.h>
#define FUNPREF tb64avx
  #elif defined(__SSE4_1__)
#include <smmintrin.h>
#define FUNPREF tb64sse
  #elif defined(__SSSE3__)
    #ifdef __powerpc64__
#define __SSE__   1
#define __SSE2__  1
#define __SSE3__  1
#define NO_WARN_X86_INTRINSICS 1
    #endif
#define FUNPREF tb64sse
#include <tmmintrin.h>
  #elif defined(__SSE2__)
#include <emmintrin.h>
  #elif defined(__ARM_NEON)
#include <arm_neon.h>
  #endif
  
#include "conf.h"
#include "turbob64.h"

#define PREFETCH(_ip_,_i_,_rw_) __builtin_prefetch(_ip_+(_i_),_rw_)

//#define B64CHECK

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

  #ifndef vld1q_u8_x4
static inline uint8x16x4_t vld1q_u8_x4(const uint8_t *lut) {
  uint8x16x4_t v;
  v.val[0] = vld1q_u8(lut);
  v.val[1] = vld1q_u8(lut+16);
  v.val[2] = vld1q_u8(lut+32);
  v.val[3] = vld1q_u8(lut+48);
  return v;
}
  #endif

#define CHECK0(a) a
  #ifdef B64CHECK
#define CHECK1(a) a
  #else
#define CHECK1(a) 
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

unsigned tb64ssedec(const unsigned char *in, unsigned inlen, unsigned char *out) {
  const unsigned char *ip;
        unsigned char *op; 
      uint8x16x4_t vlut0 = vld1q_u8_x4( lut),
                   vlut1 = vld1q_u8_x4(&lut[64]);
  const uint8x16_t  cv40 = vdupq_n_u8(0x40);
        uint8x16_t    xv = vdupq_n_u8(0);
		  
  for(ip = in, op = out; ip != in+(inlen&~(128-1)); ip += 128, op += (128/4)*3) {
    uint8x16x4_t iv0 = vld4q_u8(ip),
                 iv1 = vld4q_u8(ip+64);                                   

	uint8x16x3_t ov0; B64D(iv0,ov0);
	CHECK0(xv = vorrq_u8(xv, vorrq_u8(vorrq_u8(iv0.val[0], iv0.val[1]), vorrq_u8(iv0.val[2], iv0.val[3]))));
	uint8x16x3_t ov1; B64D(iv1,ov1);

	vst3q_u8(op,    ov0);       
	vst3q_u8(op+48, ov1);                                                                                                                                                       
	CHECK1(xv = vorrq_u8(xv, vorrq_u8(vorrq_u8(iv1.val[0], iv1.val[1]), vorrq_u8(iv1.val[2], iv1.val[3]))));
  }
  if(vaddvq_u8(vshrq_n_u8(xv,7))) return 0;	
  if(!tb64xdec(ip, inlen&(128-1), op)) return 0;
  return inlen; 
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

unsigned tb64sseenc(const unsigned char* in, unsigned inlen, unsigned char *out) {
  static unsigned char lut[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  const unsigned char *ip; 
        unsigned char *op;
  const unsigned      outlen = (inlen/3)*4;
  const uint8x16x4_t vlut = vld1q_u8_x4(lut);
  const uint8x16_t   cv3f = vdupq_n_u8(0x3f);
  
  for(ip = in, op = out; op != out+(outlen&~(128-1)); op += 128, ip += (128/4)*3) { 	 								
    const uint8x16x3_t iv0 = vld3q_u8(ip);
    const uint8x16x3_t iv1 = vld3q_u8(ip+48);
   
    uint8x16x4_t ov0; B64E(iv0, ov0); 
    uint8x16x4_t ov1; B64E(iv1, ov1);

	vst4q_u8(op,    ov0);                                                       
	vst4q_u8(op+64, ov1);                                                       
  }
  tb64xenc(ip, outlen&(128-1), op);
  return TURBOB64LEN(inlen);
}

#elif defined(__SSSE3__) //----------------- SSSE3 / SSE4.1 / AVX (derived from the AVX2 functions ) -----------------------------------------------------------------

#define DEC_RESHUFFLE(v) {\
  const __m128i merge_ab_and_bc = _mm_maddubs_epi16(v,            _mm_set1_epi32(0x01400140));  /*/dec_reshuffle: https://arxiv.org/abs/1704.00605 P.17*/\
                              v = _mm_madd_epi16(merge_ab_and_bc, _mm_set1_epi32(0x00011000));\
                              v = _mm_shuffle_epi8(v, cpv);\
}

#define ASCII2BIN(hi_nibbles, lo_nibbles, iv, ov) {\
  hi_nibbles = _mm_and_si128(_mm_srli_epi32(iv, 4), cv2f);   	/*fromascii: https://arxiv.org/abs/1704.00605 P.15*/\
  lo_nibbles = _mm_and_si128(iv, cv2f);\
          ov = _mm_add_epi8(iv, _mm_shuffle_epi8(lut_roll, _mm_add_epi8(_mm_cmpeq_epi8(iv, cv2f), hi_nibbles)));\
}

#define CHECK0
  #ifdef B64CHECK
#define CHECK1
  #endif
unsigned TEMPLATE2(FUNPREF, dec)(const unsigned char *in, unsigned inlen, unsigned char *out) {
  const unsigned char *ip=in;
        unsigned char *op=out; 
  const __m128i lut_lo  = _mm_set_epi8( 26, 27, 27, 27, 26, 19, 17, 17,   17, 17, 17, 17, 17, 17, 17, 21),
                lut_hi  = _mm_set_epi8( 16, 16, 16, 16, 16, 16, 16, 16,    8,  4,  8,  4,  2,  1, 16, 16),
               lut_roll = _mm_set_epi8(  0,  0,  0,  0,  0,  0,  0,  0,  -71,-71,-65,-65,  4, 19, 16,  0),			                             
                    cpv = _mm_set_epi8( -1, -1, -1, -1, 12, 13, 14,  8,    9, 10,  4,  5,  6,  0,  1,  2), 
                   cv2f = _mm_set1_epi8(0x2f);
  __m128i vx = _mm_setzero_si128();
  
  if(inlen >= 32+4) {
	const unsigned char *ie = in+(inlen-(32+4));
    for(op = out; ip < ie; ip += 32, op += (32/4)*3) {  	
      __m128i iv0 = _mm_loadu_si128((__m128i *) ip);
      __m128i iv1 = _mm_loadu_si128((__m128i *)(ip+16));				
                																
      __m128i hi_nibbles0, lo_nibbles0, ov0;    ASCII2BIN(hi_nibbles0, lo_nibbles0, iv0,ov0); DEC_RESHUFFLE(ov0);
      __m128i hi_nibbles1, lo_nibbles1, ov1;    ASCII2BIN(hi_nibbles1, lo_nibbles1, iv1,ov1); DEC_RESHUFFLE(ov1);
	
      _mm_storeu_si128((__m128i*) op,     ov0);											         
      _mm_storeu_si128((__m128i*)(op+12), ov1);												PREFETCH(ip,1024,0);										
      
        #ifdef CHECK0
	  hi_nibbles0 = _mm_shuffle_epi8(lut_hi, hi_nibbles0);    lo_nibbles0 = _mm_shuffle_epi8(lut_lo, lo_nibbles0);										
      vx = _mm_or_si128(vx, _mm_and_si128(lo_nibbles0, hi_nibbles0));
        #endif
        #ifdef CHECK1
      hi_nibbles1 = _mm_shuffle_epi8(lut_hi, hi_nibbles1);    lo_nibbles1 = _mm_shuffle_epi8(lut_lo, lo_nibbles1);
      vx = _mm_or_si128(vx, _mm_and_si128(lo_nibbles1, hi_nibbles1));
        #endif
    }
    if(_mm_movemask_epi8(_mm_cmpgt_epi8(vx, _mm_setzero_si128()))) return 0;
  }
  unsigned rc;
  if(!(rc = tb64xdec(ip, inlen-(ip-in), op))) return 0;
  return (op-out)+rc; 
}

static ALWAYS_INLINE __m128i bin2ascii(const __m128i v) {
  const __m128i offsets = _mm_set_epi8(0, 0, -16, -19, -4, -4, -4, -4,   -4, -4, -4, -4, -4, -4, 71, 65);

  __m128i vidx = _mm_subs_epu8(v,   _mm_set1_epi8(51));
          vidx = _mm_sub_epi8(vidx, _mm_cmpgt_epi8(v, _mm_set1_epi8(25)));
  return _mm_add_epi8(v, _mm_shuffle_epi8(offsets, vidx));
}

static ALWAYS_INLINE __m128i enc_reshuffle(__m128i v) {
  __m128i va = _mm_mulhi_epu16(_mm_and_si128(v, _mm_set1_epi32(0x0fc0fc00)), _mm_set1_epi32(0x04000040));
  __m128i vb = _mm_mullo_epi16(_mm_and_si128(v, _mm_set1_epi32(0x003f03f0)), _mm_set1_epi32(0x01000010));
  return       _mm_or_si128(va, vb);						
}

unsigned TEMPLATE2(FUNPREF, enc)(const unsigned char* in, unsigned inlen, unsigned char *out) { 
  const unsigned char *ip=in; 
        unsigned char *op=out;
        unsigned      outlen = (inlen/3)*4;

  const __m128i shuf    = _mm_set_epi8(10,11,  9, 10,  7,  8,  6,  7,    4,  5,  3,  4,  1,  2,  0,  1);
  const __m128i offsets = _mm_set_epi8( 0, 0,-16,-19, -4, -4, -4, -4,   -4, -4, -4, -4, -4, -4, 71, 65);
  
  if(outlen >= 32+4)
    for(ip = in, op = out; op <= out+(outlen-(32+4)); op += 32, ip += (32/4)*3) { 	 					PREFETCH(ip,1024,0);			
	  __m128i v0 = _mm_loadu_si128((__m128i*)ip);      
	  __m128i v1 = _mm_loadu_si128((__m128i*)(ip+12)); 

              v0 = _mm_shuffle_epi8(v0, shuf);
              v1 = _mm_shuffle_epi8(v1, shuf);
	          v0 = enc_reshuffle(v0);
			  v1 = enc_reshuffle(v1);
              v0 = bin2ascii(v0);
              v1 = bin2ascii(v1);
			  
      _mm_storeu_si128((__m128i*) op,     v0);											
      _mm_storeu_si128((__m128i*)(op+16), v1);											
    }
  tb64xenc(ip, inlen-(ip-in), op);
  return TURBOB64LEN(inlen);
}
#endif

//-------------------------------------------------------------------------------------------------------------------
#if !defined(__AVX__) //include only 1 time
static int _cpuisa;
  #if defined(__ARM_NEON) || defined(__SSE__) || defined(__powerpc64__)
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
  #endif

int cpuisa(void) {
  int c[4] = {0};                               
  if(_cpuisa) return _cpuisa;                                   
  _cpuisa++;      
    #if defined(__i386__) || defined(__x86_64__)    
  cpuid(c, 0);                                        
  if(c[0]) {              
    cpuid(c, 1);                                       
    if( c[3] & (1 << 25)) { _cpuisa = 10; // SSE
    if( c[3] & (1 << 26)) { _cpuisa = 20; // SSE2
    if( c[2] & (1 <<  0)) { _cpuisa = 30; // SSE3                                          
    if( c[2] & (1 <<  9)) { _cpuisa = 33; // SSSE3
    if( c[2] & (1 << 19)) { _cpuisa = 40; // SSE4.1 
    if( c[2] & (1 << 23)) { _cpuisa = 41; // +popcount       
    if( c[2] & (1 << 20)) { _cpuisa = 42; // SSE4.2
    if((c[2] & (1 << 28)) &&         
       (c[2] & (1 << 27)) && // OSXSAVE 
       (c[2] & (1 << 26)) && // XSAVE
       (xgetbv(0) & 6)==6){ _cpuisa = 50; // AVX
      if(c[2]& (1 << 25))   _cpuisa = 51; // +AES
      cpuid(c, 7);                                    
      if(c[1] & (1 << 5))   _cpuisa = 52; // AVX2
    }}}}}}}}}
	#elif defined(__powerpc64__)
  _cpuisa = 35; // power9 
    #elif defined(__ARM_NEON)
  _cpuisa = 34; // ARM_NEON 
    #endif 
  return _cpuisa;
}

int cpuini(int cpuisa) { if(cpuisa) _cpuisa = cpuisa; return _cpuisa; }

char *cpustr(int cpuisa) {
  if(!cpuisa) cpuisa = _cpuisa;
       if(cpuisa >= 52) return "avx2";
  else if(cpuisa >= 51) return "avx+aes";
  else if(cpuisa >= 50) return "avx";
  else if(cpuisa >= 42) return "sse4.2"; 
  else if(cpuisa >= 41) return "sse4.1x"; //+popcount
  else if(cpuisa >= 40) return "sse4.1";
  else if(cpuisa >= 35) return "power9";
  else if(cpuisa >= 34) return "arm_neon";
  else if(cpuisa >= 33) return "ssse3";
  else if(cpuisa >= 30) return "sse3";
  else if(cpuisa >= 20) return "sse2";
  else if(cpuisa >= 10) return "sse";
  else return "none";
}

//---------------------------------------------------------------------------------
typedef unsigned (*TPFUNC)(const unsigned char *in, unsigned n, unsigned char *out);

static TPFUNC _tb64e = tb64xenc;
static TPFUNC _tb64d = tb64xdec;

static int tb64set;
 
void tb64ini(int id) { 
  int i; 
  if(tb64set) return; 
  tb64set++;   
  i = id?id:cpuisa();
    #if defined(__i386__) || defined(__x86_64__)
      #ifndef NO_AVX2
  if(i >= 52) {  
    _tb64e = tb64avx2enc; 
    _tb64d = tb64avx2dec;
  } else 
      #endif
      #ifndef NO_AVX
    if(i >= 50) {  
    _tb64e = tb64avxenc; 
    _tb64d = tb64avxdec;
  } else 
      #endif
    #endif
    #if defined(__i386__) || defined(__x86_64__) || defined(__ARM_NEON) || defined(__powerpc64__)
      #ifndef NO_SSE
  if(i >= 33) {  
    _tb64e = tb64sseenc; 
    _tb64d = tb64ssedec;
  }
      #endif
    #endif
}

unsigned tb64enc(const unsigned char *in, unsigned inlen, unsigned char *out) {
  if(!tb64set) tb64ini(0);
  return _tb64e(in,inlen,out);
}
unsigned tb64dec(const unsigned char *in, unsigned inlen, unsigned char *out) {
  if(!tb64set) tb64ini(0);
  return _tb64d(in,inlen,out);
}
#endif

