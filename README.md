TurboBase64: Fastest Base64 SIMD/Neon Encoding[![Build Status](https://travis-ci.org/powturbo/TurboBase64.svg?branch=master)](https://travis-ci.org/powturbo/TurboBase64)
===================================

###### **Fastest Base64 SIMD** Encoding library
 * 100% C (C++ headers), as simple as memcpy. OS:Linux amd64, arm64, Power9, MacOs
 * No other base64 library encode or decode faster
 * :sparkles: Scalar can be faster than other SSE or ARM Neon based base64 libraries
 * :new: TurboBase64 SSE faster than other SSE/AVX/AVX2! base64 library
 * :new: Fastest **AVX2** implementation
 * :new: Fastest **ARM Neon** base64
 * :+1: Dynamic CPU detection and **JIT scalar/sse/avx/avx2** switching
 * Base64 robust error checking
 * Portable library, 32/64 bits, SSE/AVX/AVX2, ARM Neon, Power9
 * Ready and simple to use library, no hassles dependencies
<p>

------------------------------------------------------------------------

## Benchmark incl. the fastest SIMD Base64 libs:
- with [TurboBench](https://github.com/powturbo/TurboBench)
- Single thread
- Including base64 error checking
- Realistic and practical (no PURE cache) benchmark with large binary game assets corpus pd3d.tar (32 MB)

###### Benchmark Intel CPU: Skylake i7-6700 3.4GHz gcc 9.2
|C Size|ratio%|C MB/s|D MB/s|Name|Description 2019.12|
|--------:|-----:|--------:|--------:|----------------|----------------|
|42603868|133.3|**8435**|**9011**|[**TB64avx2**](https://github.com/powturbo/TurboBase64)|**Turbo Base64 avx2**|
|42603868|133.3|**8057**|**8564**|[**TB64avx**](https://github.com/powturbo/TurboBase64)|**Turbo Base64 avx**|
|42603868|133.3|**7943**|**8017**|[**TB64sse**](https://github.com/powturbo/TurboBase64)|**Turbo Base64 sse**|
|42603868|133.3|7795|7860|[fb64avx2](https://github.com/lemire/fastbase64)|Fastbase64 avx2|
|42603868|133.3|7809|7815|[b64avx2](https://github.com/aklomp/base64)|Base64 avx2|
|42603868|133.3|7161|6510|[b64avx](https://github.com/aklomp/base64)|Base64 avx|
|42603868|133.3|6420|5560|[b64sse](https://github.com/aklomp/base64)|Base64 sse41|
|42603868|133.3|**3925**|**3143**|[**TB64x**](https://github.com/powturbo/TurboBase64)|**Turbo Base64 scalar**|
|42603868|133.3|1872|2490|[b64plain](https://github.com/aklomp/base64)|Base64 plain|
|42603868|133.3|1907|2179|[TB64s](https://github.com/powturbo/TurboBase64)|**Turbo Base64 scalar**|
|42603868|133.3|1262|1375|[chromium](https://github.com/lemire/fastbase64)|Google Chromium base64|
|42603868|133.3|1675|1167|[fb64plain](https://github.com/lemire/fastbase64)|FastBase64 plain|
|42603869|162.8|1122|816|[quicktime](https://github.com/lemire/fastbase64)|Apple Quicktime base64|
|43269553|135.4| 903|171|[linux](https://github.com/lemire/fastbase64)|Linux base64|
|31952900|100.0|**14198**|**14448**|**memcpy**|

###### Benchmark ARM: ARMv8 A73-ODROID-N2 1.8GHz (clang 6.0)
|C Size|ratio%|C MB/s|D MB/s|Name|Description 2019.12|
|--------:|-----:|--------:|--------:|----------------|----------------|
|42603868|133.3|**2026**|**1212**|[**TB64neon**](https://github.com/powturbo/TurboBase64)|**TurboBase64 Neon**|
|42603868|133.3|1795|989|[b64neon](https://github.com/aklomp/base64)|Base64 Neon|
|42603868|133.3|**1279**|**801**|[**TB64x**](https://github.com/powturbo/TurboBase64)|**TurboBase64 scalar**|
|42603868|133.3|702|639|[TB64s](https://github.com/powturbo/TurboBase64)|**TurboBase64 scalar**|
|42603868|133.3|512|615|[fb64neon](https://github.com/lemire/fastbase64)|Fastbase64 SIMD Neon|
|42603868|133.3|565|460|[Chromium](https://github.com/lemire/fastbase64)|Google Chromium base64|
|42603868|133.3|641|459|[b64plain](https://github.com/aklomp/base64)|Base64 plain|
|42603868|133.3|506|412|[fb64plain](https://github.com/lemire/fastbase64)|Fastbase64 plain|
|43269553|135.4|314|91|[Linux](https://github.com/lemire/fastbase64)|Linux base64|
|31952900|100.0|**4050**|**4105**|**memcpy**|

(**bold** = pareto in category)  MB=1.000.000


<p>

## Compile: (Download or clone TurboBase64)
        git clone git://github.com/powturbo/TurboBase64.git
        make

## Usage: (Benchmark App)

        ./tb64app file

## Function usage:

>**static inline unsigned turbob64len(unsigned n)**<br />
	Base64 output length after encoding

>**unsigned tb64enc(const unsigned char *in, unsigned inlen, unsigned char *out)**<br />
	Encode binary input 'in' buffer into base64 string 'out'<br />
	with automatic cpu detection for avx2/sse4.1/scalar<br />
	**in**          : Input buffer to encode<br />
	**inlen**       : Length in bytes of input buffer<br />
	**out**         : Output buffer<br />
	**return value**: Length of output buffer<br />
	**Remark**      : byte 'zero' is not written to end of output stream<br />
    	         	  Caller must add 0 (out[outlen] = 0) for a null terminated string<br />


>**unsigned tb64dec(const unsigned char *in, unsigned inlen, unsigned char *out)**<br />
	Decode base64 input 'in' buffer into binary buffer 'out' <br />
	**in**          : input buffer to decode<br />
	**inlen**       : length in bytes of input buffer <br />
	**out**         : output buffer<br />
	**return value**: >0 output buffer length<br />
                       0 Error (invalid base64 input or input length = 0)<br />

### Environment:

###### OS/Compiler (32 + 64 bits):
- Windows: Visual C++ (2017)
- Windows: MinGW-w64 makefile
- Linux amd/intel: GNU GCC (>=4.6)
- Linux amd/intel: Clang (>=3.2) 
- Linux arm: aarch64 ARMv8: gcc (>=6.3) 
- Linux arm: aarch64 ARMv8: clang (>=6.0) 
- MaxOS: XCode (>=9)
- PowerPC ppc64le: gcc (>=8.0)

###### References:
- [fastbase](https://github.com/lemire/fastbase64)
- [base64simd](https://github.com/WojciechMula/base64simd)
- [base64](https://github.com/aklomp/base64)

Last update: 26 Nov 2019

