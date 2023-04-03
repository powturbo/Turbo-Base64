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
// Turbo-Base64: Scalar decode 
//------------- TurboBase64 : Base64 decoding -------------------
#include "turbob64_.h"
#include "turbob64.h"

//--------------------- Decoding with small lut (only 64 bytes used)------------------------------------

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

#define DS32(_u_) BSWAP32(lut[(unsigned char)(_u_     )] << 26 |\
                          lut[(unsigned char)(_u_>>  8)] << 20 |\
                          lut[(unsigned char)(_u_>> 16)] << 14 |\
                          lut[                _u_>> 24 ] <<  8)

#define DS32C(_u_)       (lut[(unsigned char)(_u_     )] |\
                          lut[(unsigned char)(_u_>>  8)] |\
                          lut[(unsigned char)(_u_>> 16)] |\
                          lut[                _u_>> 24 ])
						  
#define DS_(_i_,_check0_,_check1_) {\
  _check0_; uint32_t v = ctou32(ip+4+_i_*8  ); stou32(op+_i_*6,   DS32(u));\
  _check1_;          u = ctou32(ip+4+_i_*8+4); stou32(op+_i_*6+3, DS32(v));\
}
#define DSC(_i_) DS_(_i_,CHECK0(cu |= u),CHECK1(cu |= v))
#define DS(_i_)  DS_(_i_,;,;)
				   
  #ifdef NCHECK     // no checking
#define DSC(a) DS(a)
  #else
    #ifdef B64CHECK // Full check
#undef DS
#define DS(a) DSC(a)
    #endif
  #endif
 
size_t tb64sdec(const unsigned char *__restrict in, size_t inlen, unsigned char *__restrict out) {
  const unsigned char *ip = in;
        unsigned char *op = out;
        unsigned      cu  = 0;
  if(!inlen || (inlen&3)) return 0;												  								
  
  if(inlen > 8) { 															
    uint32_t u = ctou32(ip);
    for(; ip < in+inlen-64; ip += 64, op += 64*3/4) { DSC(0); DS(1); DS( 2); DS( 3); DS( 4); DS( 5); DS( 6); DS( 7); PREFETCH(ip, 384, 0); }
    for(; ip < in+inlen- 8; ip +=  8, op +=  8*3/4)   DS(0); 
  }
  if(ip < (in+inlen)-4) { unsigned u = ctou32(ip); ip += 4; cu |= DS32C(u); u = DS32(u); stou32(op, u); op += 3; }

       if(ip[3] != '=') { unsigned u = ctou32(ip); cu |= DS32C(u); u = DS32(u);                   op[0] = u; op[1] = u>>8; op[2] = u>>16; op+=3;                                               } // 4->3 bytes
  else if(ip[2] != '=') { unsigned u = BSWAP32(lut[ip[0]]<<26 | lut[ip[1]]<<20 | lut[ip[2]]<<14); op[0] = u; op[1] = u>>8;                op+=2;    cu |= lut[ip[0]] | lut[ip[1]] | lut[ip[2]];} // 3->2 bytes
  else if(ip[1] != '=') {                                                                        *op++  = BSWAP32(lut[ip[0]]<<26 | lut[ip[1]]<<20); cu |= lut[ip[0]] | lut[ip[1]];             } // 2->1 byte
  else                  {                                                                        *op++  = BSWAP32(lut[ip[0]]);                      cu |= lut[ip[0]];                          };// 1->1 byte
  return (cu == 0xff)?0:(op - out);
}

//------ Fast decoding with pre shifted lookup table: 4k=4*4*256, (but only 4*4*64 = 1024 bytes used) for fast decoding --------------------------------------------------------------------

  #if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) 
#define _ -1 // invalid entry
const unsigned tb64lutd0[] = {
         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _, 
         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,
         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,0xf8000000,         _,         _,         _,0xfc000000,
0xd0000000,0xd4000000,0xd8000000,0xdc000000,0xe0000000,0xe4000000,0xe8000000,0xec000000,0xf0000000,0xf4000000,         _,         _,         _,         _,         _,         _,
         _,0x00000000,0x04000000,0x08000000,0x0c000000,0x10000000,0x14000000,0x18000000,0x1c000000,0x20000000,0x24000000,0x28000000,0x2c000000,0x30000000,0x34000000,0x38000000,
0x3c000000,0x40000000,0x44000000,0x48000000,0x4c000000,0x50000000,0x54000000,0x58000000,0x5c000000,0x60000000,0x64000000,         _,         _,         _,         _,         _,
         _,0x68000000,0x6c000000,0x70000000,0x74000000,0x78000000,0x7c000000,0x80000000,0x84000000,0x88000000,0x8c000000,0x90000000,0x94000000,0x98000000,0x9c000000,0xa0000000,
0xa4000000,0xa8000000,0xac000000,0xb0000000,0xb4000000,0xb8000000,0xbc000000,0xc0000000,0xc4000000,0xc8000000,0xcc000000,         _,         _,         _,         _,         _,
         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,
         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,
         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,
         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,
         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,
         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _, 
         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,
         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _,         _
};

const unsigned tb64lutd1[] = {
        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,
        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,
        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,0x3e00000,        _,        _,        _,0x3f00000,
0x3400000,0x3500000,0x3600000,0x3700000,0x3800000,0x3900000,0x3a00000,0x3b00000,0x3c00000,0x3d00000,        _,        _,        _,        _,        _,        _,
        _,0x0000000,0x0100000,0x0200000,0x0300000,0x0400000,0x0500000,0x0600000,0x0700000,0x0800000,0x0900000,0x0a00000,0x0b00000,0x0c00000,0x0d00000,0x0e00000,
0x0f00000,0x1000000,0x1100000,0x1200000,0x1300000,0x1400000,0x1500000,0x1600000,0x1700000,0x1800000,0x1900000,        _,        _,        _,        _,        _,
        _,0x1a00000,0x1b00000,0x1c00000,0x1d00000,0x1e00000,0x1f00000,0x2000000,0x2100000,0x2200000,0x2300000,0x2400000,0x2500000,0x2600000,0x2700000,0x2800000,
0x2900000,0x2a00000,0x2b00000,0x2c00000,0x2d00000,0x2e00000,0x2f00000,0x3000000,0x3100000,0x3200000,0x3300000,        _,        _,        _,        _,        _,
        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,
        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,
        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,
        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,
        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,
        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,
        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,
        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,        _,         _
};

const unsigned tb64lutd2[] = {
      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,
      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,
      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,0xf8000,      _,      _,      _,0xfc000,
0xd0000,0xd4000,0xd8000,0xdc000,0xe0000,0xe4000,0xe8000,0xec000,0xf0000,0xf4000,      _,      _,      _,      _,      _,      _,
      _,0x00000,0x04000,0x08000,0x0c000,0x10000,0x14000,0x18000,0x1c000,0x20000,0x24000,0x28000,0x2c000,0x30000,0x34000,0x38000,
0x3c000,0x40000,0x44000,0x48000,0x4c000,0x50000,0x54000,0x58000,0x5c000,0x60000,0x64000,      _,      _,      _,      _,      _,
      _,0x68000,0x6c000,0x70000,0x74000,0x78000,0x7c000,0x80000,0x84000,0x88000,0x8c000,0x90000,0x94000,0x98000,0x9c000,0xa0000,
0xa4000,0xa8000,0xac000,0xb0000,0xb4000,0xb8000,0xbc000,0xc0000,0xc4000,0xc8000,0xcc000,      _,      _,      _,      _,      _,
      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,
      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,
      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,
      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,
      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,
      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _, 
      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,
      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _,      _
};

const unsigned tb64lutd3[] = {
     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,
     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,
     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,0x3e00,     _,     _,     _,0x3f00,
0x3400,0x3500,0x3600,0x3700,0x3800,0x3900,0x3a00,0x3b00,0x3c00,0x3d00,     _,     _,     _,     _,     _,     _,
     _,0x0000,0x0100,0x0200,0x0300,0x0400,0x0500,0x0600,0x0700,0x0800,0x0900,0x0a00,0x0b00,0x0c00,0x0d00,0x0e00,
0x0f00,0x1000,0x1100,0x1200,0x1300,0x1400,0x1500,0x1600,0x1700,0x1800,0x1900,     _,     _,     _,     _,     _,
     _,0x1a00,0x1b00,0x1c00,0x1d00,0x1e00,0x1f00,0x2000,0x2100,0x2200,0x2300,0x2400,0x2500,0x2600,0x2700,0x2800,
0x2900,0x2a00,0x2b00,0x2c00,0x2d00,0x2e00,0x2f00,0x3000,0x3100,0x3200,0x3300,     _,     _,     _,     _,     _,
     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,
     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,
     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,
     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,
     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,
     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,
     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,
     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _,     _
};
#undef _

  #else
#define _ -1 // invalid entry
const unsigned tb64lutd0[] = { 
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

const unsigned tb64lutd1[] = {
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

const unsigned tb64lutd2[] = {
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

const unsigned tb64lutd3[] = {
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
  #endif

  #ifdef NCHECK
#define DXC(a) DX(a)
  #else
    #ifdef B64CHECK
#undef DX
#define DX(a) DXC(a)
    #endif
#endif

size_t tb64declen(const unsigned char *__restrict in, size_t inlen) {
  if(!inlen || (inlen&3)) return 0;

  size_t outlen = (inlen/4)*3;
  const unsigned char *ip = in+inlen;
       if(ip[-1] != '=');  
  else if(ip[-2] != '=') outlen -= 1; 
  else if(ip[-3] != '=') outlen -= 2;
  else                   outlen -= 3;
  return outlen;
}

#define DX_(_i_,_check0_,_check1_) {\
  unsigned long long v; ltou64(&v, ip+8+_i_*16  ); unsigned _x = u; _check0_; stou32(op+_i_*12,   DU32(_x)); _x = u>>32; stou32(op+_i_*12+3, DU32(_x));\
              ltou64(&u, ip+8+_i_*16+8);          _x = v; _check1_; stou32(op+_i_*12+6, DU32(_x)); _x = v>>32; stou32(op+_i_*12+9, DU32(_x));\
}

#define DXC(_i_) DX_(_i_, CHECK0(cu |= _x), CHECK1(cu |= _x))
#define DX(_i_)  DX_(_i_,;,;)

size_t tb64xdec(const unsigned char *__restrict in, size_t inlen, unsigned char *__restrict out) {
  const unsigned char *ip = in;
        unsigned char *op = out;  
        unsigned      cu  = 0;
  if(!inlen || (inlen&3)) return 0;
  
  if(inlen > 16+4) { 															
    unsigned long long u = ctou64(ip); 
    for(; ip < in+inlen-4-128; ip += 128, op += 128*3/4) { DXC(0); DX(1); DX( 2); DX( 3); DX( 4); DX( 5); DX( 6); DX( 7); PREFETCH(ip, 384, 0); }
    for(; ip < in+inlen-4-16;  ip +=  16, op +=  16*3/4)   DX( 0);
  }
  for(; ip < (in+inlen)-4; ip += 4, op += 3) { unsigned u = ctou32(ip); u = DU32(u); stou32(op, u); cu |= u; }
  DXTAILC(ip,out,op,cu |= u)  
  return (cu == -1)?0:(op-out);
}
