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
#ifndef _TURBOB64_H_
#define _TURBOB64_H_
#ifdef __cplusplus
extern "C" {
#endif

#define TB64_VERSION 100
//---------------------- base64 API functions ----------------------------------
// Base64 output length after encoding 
#define TB64ENCLEN(_n_) ((_n_ + 2)/3 * 4)
unsigned tb64memcpy(const unsigned char *in, unsigned inlen, unsigned char *out);

// return the base64 buffer length after encoding
unsigned tb64enclen(unsigned inlen);

// return the original (after decoding) length for a given base64 encoded buffer
unsigned tb64declen(const unsigned char *in, unsigned inlen);

// Encode binary input 'in' buffer into base64 string 'out' 
// with automatic cpu detection for avx2/sse4.1/scalar 
// in          : Input buffer to encode
// inlen       : Length in bytes of input buffer
// out         : Output buffer
// return value: Length of output buffer
// Remark      : byte 'zero' is not written to end of output stream
//               Caller must add 0 (out[outlen] = 0) for a null terminated string
unsigned tb64enc(const unsigned char *in, unsigned inlen, unsigned char *out);

// Decode base64 input 'in' buffer into binary buffer 'out' 
// in          : input buffer to decode
// inlen       : length in bytes of input buffer 
// out         : output buffer
// return value: >0 output buffer length
//                0 Error (invalid base64 input or input length = 0)
unsigned tb64dec(const unsigned char *in, unsigned inlen, unsigned char *out);

//---------------------- base64 Internal functions ------------------------------
// Space efficient scalar but (slower) version
unsigned tb64senc(   const unsigned char *in, unsigned inlen, unsigned char *out);
unsigned tb64sdec(   const unsigned char *in, unsigned inlen, unsigned char *out);

// Fast scalar base64
unsigned tb64xenc(   const unsigned char *in, unsigned inlen, unsigned char *out);
unsigned tb64xdec(   const unsigned char *in, unsigned inlen, unsigned char *out);

// ssse3 base64 
unsigned tb64sseenc( const unsigned char *in, unsigned inlen, unsigned char *out);
unsigned tb64ssedec( const unsigned char *in, unsigned inlen, unsigned char *out);

// avx base64 
unsigned tb64avxenc( const unsigned char *in, unsigned inlen, unsigned char *out);
unsigned tb64avxdec( const unsigned char *in, unsigned inlen, unsigned char *out);

// avx2 base 64
unsigned tb64avx2enc(const unsigned char *in, unsigned inlen, unsigned char *out);
unsigned tb64avx2dec(const unsigned char *in, unsigned inlen, unsigned char *out);

unsigned tb64avx512enc(const unsigned char *in, unsigned inlen, unsigned char *out);
unsigned tb64avx512dec(const unsigned char *in, unsigned inlen, unsigned char *out);

void tb64ini(int id);  // detect cpu && set the default run time functions for tb64enc/tb64dec
 
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

