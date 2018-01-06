/**
Copyright (c) 2016-2018, Powturbo
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

#include "conf.h"
#include "turbob64.h"

// Pre shifted lookup table: 3k+256 bytes
#define _ 0xff
static const unsigned char lut0[] = { 
   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,
   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,
   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,0xf8,   _,   _,   _,0xfc,
0xd0,0xd4,0xd8,0xdc,0xe0,0xe4,0xe8,0xec,0xf0,0xf4,   _,   _,   _,   _,   _,   _,
   _,0x00,0x04,0x08,0x0c,0x10,0x14,0x18,0x1c,0x20,0x24,0x28,0x2c,0x30,0x34,0x38,
0x3c,0x40,0x44,0x48,0x4c,0x50,0x54,0x58,0x5c,0x60,0x64,   _,   _,   _,   _,   _,
   _,0x68,0x6c,0x70,0x74,0x78,0x7c,0x80,0x84,0x88,0x8c,0x90,0x94,0x98,0x9c,0xa0,
0xa4,0xa8,0xac,0xb0,0xb4,0xb8,0xbc,0xc0,0xc4,0xc8,0xcc,   _,   _,   _,   _,   _,
   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,
   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,
   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,
   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,
   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,
   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,
   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,
   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _
};
#undef _

#define _ 0xffffff
static const unsigned lut1[] = {
     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,
     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,
     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,0xe003,     _,     _,     _,0xf003,
0x4003,0x5003,0x6003,0x7003,0x8003,0x9003,0xa003,0xb003,0xc003,0xd003,     _,     _,     _,     _,     _,     _,
     _,0x0000,0x1000,0x2000,0x3000,0x4000,0x5000,0x6000,0x7000,0x8000,0x9000,0xa000,0xb000,0xc000,0xd000,0xe000,
0xf000,0x0001,0x1001,0x2001,0x3001,0x4001,0x5001,0x6001,0x7001,0x8001,0x9001,     _,     _,     _,     _,     _,
     _,0xa001,0xb001,0xc001,0xd001,0xe001,0xf001,0x0002,0x1002,0x2002,0x3002,0x4002,0x5002,0x6002,0x7002,0x8002,
0x9002,0xa002,0xb002,0xc002,0xd002,0xe002,0xf002,0x0003,0x1003,0x2003,0x3003,     _,     _,     _,     _,     _,
     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,
     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,
     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,
     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,
     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,
     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,
     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,
     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _
};

static const unsigned lut2[] = {
       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,
       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,
       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,0x800f00,       _,       _,       _,0xc00f00,
0x000d00,0x400d00,0x800d00,0xc00d00,0x000e00,0x400e00,0x800e00,0xc00e00,0x000f00,0x400f00,       _,       _,       _,       _,       _,       _,
       _,0x000000,0x400000,0x800000,0xc00000,0x000100,0x400100,0x800100,0xc00100,0x000200,0x400200,0x800200,0xc00200,0x000300,0x400300,0x800300,
0xc00300,0x000400,0x400400,0x800400,0xc00400,0x000500,0x400500,0x800500,0xc00500,0x000600,0x400600,       _,       _,       _,       _,       _,
       _,0x800600,0xc00600,0x000700,0x400700,0x800700,0xc00700,0x000800,0x400800,0x800800,0xc00800,0x000900,0x400900,0x800900,0xc00900,0x000a00,
0x400a00,0x800a00,0xc00a00,0x000b00,0x400b00,0x800b00,0xc00b00,0x000c00,0x400c00,0x800c00,0xc00c00,       _,       _,       _,       _,       _,
       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,
       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,
       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,
       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,
       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,
       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,
       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,
       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _
};

static const unsigned lut3[] = {
       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,
       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,
       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,0x3e0000,       _,       _,       _,0x3f0000,
0x340000,0x350000,0x360000,0x370000,0x380000,0x390000,0x3a0000,0x3b0000,0x3c0000,0x3d0000,       _,       _,       _,       _,       _,       _,
       _,0x000000,0x010000,0x020000,0x030000,0x040000,0x050000,0x060000,0x070000,0x080000,0x090000,0x0a0000,0x0b0000,0x0c0000,0x0d0000,0x0e0000,
0x0f0000,0x100000,0x110000,0x120000,0x130000,0x140000,0x150000,0x160000,0x170000,0x180000,0x190000,       _,       _,       _,       _,       _,
       _,0x1a0000,0x1b0000,0x1c0000,0x1d0000,0x1e0000,0x1f0000,0x200000,0x210000,0x220000,0x230000,0x240000,0x250000,0x260000,0x270000,0x280000,
0x290000,0x2a0000,0x2b0000,0x2c0000,0x2d0000,0x2e0000,0x2f0000,0x300000,0x310000,0x320000,0x330000,       _,       _,       _,       _,       _,
       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,
       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,
       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,
       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,
       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,
       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,
       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,
       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _,       _
};
#undef _

#define DU32(_u_) (lut0[(unsigned char)(_u_)] | lut1[(unsigned char)(_u_>>8)] | lut2[(unsigned char)(_u_>>16)] | lut3[(unsigned char)(_u_>>24)])
#define DI32(_ip_,_op_,_i_) { unsigned _u = ux; ux = ctou32(_ip_+8+_i_*8);   ctou32(_op_+ _i_*6  ) = DU32(_u); \
                              unsigned _v = vx; vx = ctou32(_ip_+8+_i_*8+4); ctou32(_op_+ _i_*6+3) = DU32(_v); }

unsigned turbob64dec(unsigned char *in, unsigned inlen, unsigned char *out) {
  unsigned char *ip=in,*op=out;
  if(inlen >= 64) {
    unsigned ux = ctou32(ip), vx = ctou32(ip+4);
    for(; ip != in+(inlen&~(64-1))-64; ip+=64, op += 48) { 
      DI32(ip,op,0); DI32(ip,op,1); DI32(ip,op,2); DI32(ip,op,3);DI32(ip,op,4); DI32(ip,op,5); DI32(ip,op,6); DI32(ip,op,7);
	  __builtin_prefetch(ip+768, 0); 
    } 
  }
  for(; ip < in+inlen; ip+=4, op += 3) { unsigned u = ctou32(ip); ctou32(op) = DU32(u); } 
  return inlen;
}

  #if 0
#define DI32(_ip_,_op_,_i_) { unsigned _u = ctou32(_ip_+_i_*8);\
                              unsigned _v = ctou32(_ip_+_i_*8+4); ctou32(_op_+ _i_*6  ) = DU32(_u); ctou32(_op_+ _i_*6+3) = DU32(_v); }

unsigned turbob64dec(unsigned char *in, unsigned inlen, unsigned char *out) {
  unsigned char *ip,*op;
  for(ip = in,op = out; ip != in+(inlen&~(32-1)); ip+=32, op += 24) { 
    DI32(ip,op,0); DI32(ip,op,1); DI32(ip,op,2); DI32(ip,op,3);
	__builtin_prefetch(ip+256, 0); 
  } 
  for(; ip < in+inlen; ip+=4, op += 3) { unsigned u = ctou32(ip); ctou32(op) = DU32(u); } 
  return inlen;
}
  #endif
//-------------------
#define _ 0xff
static const unsigned char lut[] = {
 _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,
 _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,
 _, _, _, _, _, _, _, _, _, _, _,62, _, _, _,63,
52,53,54,55,56,57,58,59,60,61, _, _, _, _, _, _,
 _, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
15,16,17,18,19,20,21,22,23,24,25, _, _, _, _, _,
 _,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
41,42,43,44,45,46,47,48,49,50,51, _, _, _, _, _,
 _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,
 _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,
 _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,
 _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,
 _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,
 _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,
 _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,
 _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _ 
};
#undef _

#define LU32(_u_) bswap32(lut[(unsigned char)(_u_)]<<26 | lut[(unsigned char)(_u_>>8)]<<20 | lut[(unsigned char)(_u_>>16)]<<14 | lut[(unsigned char)(_u_>>24)]<<8)
#define LI32(_ip_,_op_,_i_) { unsigned _u = ctou32(_ip_+_i_*8);   _u = LU32(_u);\
                              unsigned _v = ctou32(_ip_+_i_*8+4); _v = LU32(_v); ctou32(_op_+ _i_*6  ) = _u; ctou32(_op_+ _i_*6+3) = _v; }

unsigned turbob64decs(unsigned char *in, unsigned inlen, unsigned char *out) {
  unsigned char *ip,*op;
  for(ip = in,op = out; ip != in+(inlen&~(32-1)); ip+=32, op += 24) { 
    LI32(ip,op,0); LI32(ip,op,1); LI32(ip,op,2); LI32(ip,op,3);
	__builtin_prefetch(ip+256, 0); 
  }
  for(; ip < in+inlen; ip+=4, op += 3) { unsigned u = ctou32(ip);  ctou32(op) = LU32(u); } 
  return inlen;
}
