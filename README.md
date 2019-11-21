TurboBase64: Turbo Base64 Encoding [![Build Status](https://travis-ci.org/powturbo/TurboBase64.svg?branch=master)](https://travis-ci.org/powturbo/TurboBase64)
===================================

###### **Turbo Base64** Encoding library
 * 100% C (C++ compatible headers), without inline assembly
 * No other scalar base64 library encode or decode faster
 * Encode or decode more than **3 times** faster than other libraries
 * can be faster than other SSE or ARM Neon based base64 libraries
 * More than 4 GB/s, saturates the fastest SSD drives
 * Portable library, both 32 and 64 bits supported
 * Ready and simple to use library, no hassless dependencies
<p>

------------------------------------------------------------------------

## Benchmark:
- with [TurboBench](https://github.com/powturbo/TurboBench)
- Single thread
- Realistic and practical benchmark with large binary game assets corpus pd3d.tar (32 MB)

###### Benchmark Intel CPU: Skylake i7-6700 3.4GHz gcc 9.2
|C Size|ratio%|C MB/s|D MB/s|Name|Description 2019.11|
|--------:|-----:|--------:|--------:|----------------|----------------|
|42603868|133.3|**7809**|**7815**|[base64_sse](https://github.com/aklomp/base64)|Base64 SIMD AVX2|
|42603868|133.3|6420|5560|[base64_sse](https://github.com/aklomp/base64)|Base64 SIMD sse41|
|42603868|133.3|3914|3391|[**TurboB64**](https://github.com/powturbo/TurboBase64)|**TurboBase64 scalar**|
|42603868|133.3|1904|2179|[TurboB64s](https://github.com/powturbo/TurboBase64)|**TurboBase64 scalar**|
|42603868|133.3|1262|1375|[fb64chromium](https://github.com/lemire/fastbase64)|Google Chromium base64|
|42603868|133.3|1674|1250|[fbase64_plain](https://github.com/aklomp/base64)|Base64 plain|
|42603868|133.3|1675|1167|[fb64scalar](https://github.com/lemire/fastbase64)|Scalar FastBase64|
|42603869|162.8|1122|816|[fb64quicktime](https://github.com/lemire/fastbase64)|Apple Quicktime base64|
|43269553|135.4| 903|171|[fb64linux](https://github.com/lemire/fastbase64)|Linux base64|
|31952900|100.0|**13398**|**14448**|**memcpy**|

###### Benchmark ARM: ARMv8 A73-ODROID-N2 1.8GHz
|C Size|ratio%|C MB/s|D MB/s|Name|Description 2019.11|
|--------:|-----:|--------:|--------:|----------------|----------------|
|42603868|133.3|**1279**|**831**|[**TurboB64**](https://github.com/powturbo/TurboBase64)|**TurboBase64 scalar**|
|42603868|133.3|702|639|[TurboB64s](https://github.com/powturbo/TurboBase64)|**TurboBase64 scalar**|
|42603868|133.3|566|615|[fastbase neon](https://github.com/lemire/fastbase64)|FastBase64 SIMD Neon|
|42603868|133.3|565|460|[fb64chromium](https://github.com/lemire/fastbase64)|Google Chromium base64|
|42603868|133.3|506|412|[fb64scalar](https://github.com/lemire/fastbase64)|FastBase64 Scalar|
|43269553|135.4|314|91|[fb64linux](https://github.com/lemire/fastbase64)|Linux base64|
|31952900|100.0|**4050**|**4105**|**memcpy**|

(**bold** = pareto)  MB=1.000.000


<p>

## Compile:
        git clone git://github.com/powturbo/TurboBase64.git
        make

## Usage:

        ./turbob64 file

### Environment:

###### OS/Compiler (32 + 64 bits):
- Windows: MinGW-w64 makefile
- Linux amd/intel: GNU GCC (>=4.6)
- Linux amd/intel: Clang (>=3.2) 
- Linux arm: aarch64 ARMv8:  gcc (>=6.3)
- MaxOS: XCode (>=9)
- PowerPC ppc64le: gcc (>=8.0)

###### References:
- [fastbase](https://github.com/lemire/fastbase64)
- [base64simd](https://github.com/WojciechMula/base64simd)
- [base64](https://github.com/aklomp/base64)

Last update: 17 Nov 2019

