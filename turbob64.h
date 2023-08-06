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
// Turbo-Base64 - C/C++ include header
#ifndef _TURBOB64_H_
#define _TURBOB64_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TB64_VERSION 100
//---------------------- Turbo-Base64 API functions ----------------------------------
// return the base64 buffer length after encoding
size_t tb64enclen(size_t inlen);

// return the original (after decoding) length for a given base64 encoded buffer
size_t tb64declen(const unsigned char *in, size_t inlen);

// Encode binary input 'in' buffer into base64 string 'out' 
// with automatic cpu detection for avx2/sse4.1/scalar 
// in          : Input buffer to encode
// inlen       : Length in bytes of input buffer
// out         : Output buffer
// return value: Length of output buffer
// Remark      : byte 'zero' is not written to end of output stream
//               Caller must add 0 (out[outlen] = 0) for a null terminated string
size_t tb64enc(const unsigned char *in, size_t inlen, unsigned char *out);

// Decode base64 input 'in' buffer into binary buffer 'out' 
// in          : input buffer to decode
// inlen       : length in bytes of input buffer 
// out         : output buffer
// return value: >0 output buffer length
//                0 Error (invalid base64 input or input length = 0)
size_t tb64dec(const unsigned char *in, size_t inlen, unsigned char *out);

//------ Direct call to tb64enc + tb64dec ---------------------------------------
// Direct call to tb64enc + tb64dec saving a function call + a check instruction
// call tb64ini, then call _tb64e(in, inlen, out) or _tb64d(in, inlen, out)
typedef size_t (*TB64FUNC)(const unsigned char *__restrict in, size_t n, unsigned char *__restrict out);

extern TB64FUNC _tb64e;
extern TB64FUNC _tb64d;

//---------------------- base64 Internal functions ------------------------------
// Base64 output length after encoding 
#define TB64ENCLEN(_n_) ((_n_ + 2)/3 * 4)

// Memory efficient (small lookup tables) scalar but (slower) version
size_t tb64senc(     const unsigned char *__restrict in, size_t inlen, unsigned char *__restrict out);
size_t tb64sdec(     const unsigned char *__restrict in, size_t inlen, unsigned char *__restrict out);

// Fast scalar
size_t tb64xenc(     const unsigned char *__restrict in, size_t inlen, unsigned char *__restrict out);
size_t tb64xdec(     const unsigned char *__restrict in, size_t inlen, unsigned char *__restrict out);

// ssse3  
size_t tb64v128enc(  const unsigned char *__restrict in, size_t inlen, unsigned char *__restrict out);
size_t tb64v128dec(  const unsigned char *__restrict in, size_t inlen, unsigned char *__restrict out);

// avx 
size_t tb64v128aenc( const unsigned char *__restrict in, size_t inlen, unsigned char *__restrict out);
size_t tb64v128adec( const unsigned char *__restrict in, size_t inlen, unsigned char *__restrict out);

// avx2
size_t tb64v256enc(  const unsigned char *__restrict in, size_t inlen, unsigned char *__restrict out);
size_t tb64v256dec(  const unsigned char *__restrict in, size_t inlen, unsigned char *__restrict out);

// avx512_vbmi
size_t tb64v512enc(  const unsigned char *__restrict in, size_t inlen, unsigned char *__restrict out);
size_t tb64v512dec(  const unsigned char *__restrict in, size_t inlen, unsigned char *__restrict out);

// detect cpu && set the default run time functions for tb64enc/tb64dec
// isshort = 0 : default
// isshort > 0 : set optimized short strings version (actually only avx2)
void tb64ini(unsigned id, unsigned isshort);  

//------- optimized functions for short strings only --------------------------
// - decoding without checking  
// - can read beyond the input buffer end, 
//   therefore input buffer size must be 32 bytes larger than input length
size_t _tb64v256enc(const unsigned char *in, size_t inlen, unsigned char *out);
size_t _tb64v256dec(const unsigned char *in, size_t inlen, unsigned char *out);

//------- CPU instruction set ----------------------
// cpuisa  = 0: return current simd set, 
// cpuisa != 0: set simd set 0:scalar, 0x33:sse2, 0x60:avx2
unsigned cpuini(unsigned cpuisa); 

// convert simd set to string "sse3", "ssse3", "sse4.1", "avx", "avx2", "neon",... 
// Ex.: printf("current cpu set=%s\n", cpustr(cpuini(0)) ); 
char *cpustr(unsigned cpuisa); 

#ifdef __cplusplus
}
#endif
#endif
