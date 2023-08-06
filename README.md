## Turbo Base64:Fastest Base64 SSE/AVX2/AVX512/Neon/Altivec
[![Build ubuntu](https://github.com/powturbo/Turbo-Base64/actions/workflows/build.yaml/badge.svg)](https://github.com/powturbo/Turbo-Base64/actions/workflows/build.yaml)

##### **Fastest Base64 SIMD** Encoding library
 * 100% C (C++ headers), as simple as memcpy
 * No other base64 library encode or decode faster
 * :sparkles: **Scalar** can be faster than other SSE or ARM Neon based base64 libraries
 * **SSE** faster than other SSE/AVX/AVX2! base64 library
 * Fastest **AVX2** implementation 
 * TurboBase64 AVX2 decoding up to ~2x faster than other AVX2 libs.
 * TurboBase64 is 3,5-4 times faster than other libs for short strings
 * Fastest **ARM Neon** base64
 * :new:(2023.04) avx512 - 2x faster than avx2, faster than any other implementation
 * :+1: Dynamic CPU detection and **JIT scalar/sse/avx/avx2/avx512** switching
 * Base64 robust **error checking**, optimized for **long+short** strings
 * Portable library, 32/64 bits, **SSE/AVX/AVX2/AVX512**, **ARM Neon**, **Power9 Altivec**
 * OS:Linux amd64, arm64, Power9, MacOs+Apple M1, s390x. Windows: Mingw, visual c++
 * Big endian + Little endian
 * Ready and simple to use library, no armada of files, no hassles dependencies
 * **LICENSE GPL 3** . Commercial license available. Contact us at powturbo [_AT_] gmail [_DOT_] com
<p>
	
------------------------------------------------------------------------
	
Download Turbo-Base64 executable benchmark tb64app from 
[releases](https://github.com/powturbo/Turbo-Base64/releases/tag/2023.04),
extract the files and type "tb64app"</br>

## Benchmark incl. the best SIMD Base64 libs:
- Single thread
- Including base64 error checking
- Small file + realistic and practical (no PURE cache) benchmark 
- Unlike other benchmarks, the best of the best scalar+simd libraries are included
- all libraries with the latest version

#### Benchmark AMD CPU: AMD Ryzen 9 7950X @ 4,50 GHz, DDR5 6000 CL30 - gcc-12.2
|E Size|ratio%|E MB/s|D MB/s|1,000,000 bytes - 2023.07 |
|--------:|-----:|--------:|--------:|----------------|
|1333336|133.33%|**77619**|**76716**|**8:tb64v512vbmi**|
|1333336|133.33%|41325| 48783| 7:_tb64v256 avx2| 
|1333336|133.33%|45292| 46665| 5:tb64v256  avx2|
|1000000|100.00%|37047| 31694|10:memcpy        | 
|1333336|133.33%|25077| 28537| 4:tb64v128a avx | 
|1333336|133.33%|24375| 27880| 3:tb64v128      | 
|1333336|133.33%| 9513|  6908| 2:tb64x         | 
|1333336|133.33%| 9513|  5975| 9:_tb64x        | 
|1333336|133.33%| 4914|  5182| 1:tb64s         | 


|E Size|ratio%|E MB/s|D MB/s|10,000 bytes - 2023.07 |
|--------:|-----:|--------:|--------:|----------------|
|13336|133.36%|**89079**|**92006**|**8:tb64v512vbmi**|
|10000|100.00%|84418|  85703|10:memcpy            |
|13336|133.36%|34963|  46216| 7:_tb64v256 avx2    |
|13336|133.36%|40722|  44552| 5:tb64v256  avx2    |
|13336|133.36%|22601|  27298| 4:tb64v128a avx     |
|13336|133.36%|21113|  26930| 3:tb64v128          |
|13336|133.36%| 9648|   6809| 2:tb64x             |
|13336|133.36%| 9626|   5599| 9:_tb64x            |
|13336|133.36%| 4937|   5184| 1:tb64s             |

#### Benchmark Intel CPU: i7-9700k 3.6GHz gcc 11.2
|E Size|ratio%|E MB/s|D MB/s|Name|50,000 bytes - 2022.02 |
|--------:|-----:|--------:|--------:|----------------|----------------|
|66668|133.3|**32794**|**37837**|[**tb64v256**](https://github.com/powturbo/TurboBase64)|**Turbo Base64 avx2**|
|66668|133.3|27789|22264|[b64avx2](https://github.com/aklomp/base64)|aklomp Base64 avx2|
|66668|133.3|25305|21980|[fb64avx2](https://github.com/lemire/fastbase64)|lemire Fastbase64 avx2|
|||||||
|66668|133.3|**17348**|**20686**|[**tb64v128a**](https://github.com/powturbo/TurboBase64)|**Turbo Base64 avx**|
|66668|133.3|**16035**|**18865**|[**tb64v128**](https://github.com/powturbo/TurboBase64)|**Turbo Base64 sse**|
|66668|133.3|15820|13078|[b64avx](https://github.com/aklomp/base64)|aklomp Base64 avx|
|66668|133.3|15322|11302|[b64sse](https://github.com/aklomp/base64)|aklomp Base64 sse41|
|50000|100.0|47593|47623|memcpy||

|E Size|ratio%|E MB/s|D MB/s|Name| 1 MB - 2022.02|
|--------:|-----:|--------:|--------:|----------------|----------------|
|1333336|133.3|**29086**|**29748**|[**tb64v256**](https://github.com/powturbo/TurboBase64)|**Turbo Base64 avx2**|
|1333336|133.3|26153|22515|[b64avx2](https://github.com/aklomp/base64)|Base64 avx2|
|1333336|133.3|23686|21231|[fb64avx2](https://github.com/lemire/fastbase64)|Fastbase64 avx2|
|||||||
|1333336|133.3|**16897**|**20215**|[**tb64v128a**](https://github.com/powturbo/TurboBase64)|**Turbo Base64 avx**|
|1333336|133.3|**15932**|**18749**|[**tb64v128**](https://github.com/powturbo/TurboBase64)|**Turbo Base64 sse**|
|1333336|133.3|15537|12959|[b64avx](https://github.com/aklomp/base64)|Base64 avx|
|1333336|133.3|15135|11304|[b64sse](https://github.com/aklomp/base64)|Base64 sse41|
|||||||
|1333336|133.3|**6546**|**5473**|[**TB64x**](https://github.com/powturbo/TurboBase64)|**Turbo Base64 scalar**|
|1333336|133.3|6495|4454|[b64plain](https://github.com/aklomp/base64)|Base64 plain|
|1333336|133.3|1908|2752|[TB64s](https://github.com/powturbo/TurboBase64)|**Turbo Base64 scalar**|
|1333336|133.3|2541|4289|[chrome](https://github.com/lemire/fastbase64)|Google Chrome base64|
|1333336|133.3|2670|2299|[fb64plain](https://github.com/lemire/fastbase64)|FastBase64 plain|
|1333334|135.4|1754|219|[linux](https://github.com/lemire/fastbase64)|Linux base64|
|1000000|100.0|28688|28656|memcpy||

<a name="short"></a> TurboBase64 vs. Base64 for short strings (incl. checking)
|String length|E MB/s|D MB/s|Name|50,000 bytes - short strings 2022.02 |
|------------:|--------:|--------:|----------------|----------------|
| 4 - 16      |**2330**|**2161**|[**TB64avx2**](https://github.com/powturbo/TurboBase64)|**Turbo Base64 avx2**|
|             |891|734|[b64avx2](https://github.com/aklomp/base64)|Base64 avx2|
| 8 - 32      |**3963**|**3570**|[**TB64avx2**](https://github.com/powturbo/TurboBase64)|**Turbo Base64 avx2**|
|             |1348|943|[b64avx2](https://github.com/aklomp/base64)|Base64 avx2|
| 16 - 64     |**6881**|**5937**|[**TB64avx2**](https://github.com/powturbo/TurboBase64)|**Turbo Base64 avx2**|
|             |2509|1488|[b64avx2](https://github.com/aklomp/base64)|Base64 avx2|
| 32 - 128    |**10946**|**8880**|[**TB64avx2**](https://github.com/powturbo/TurboBase64)|**Turbo Base64 avx2**|
|             |4902|2777|[b64avx2](https://github.com/aklomp/base64)|Base64 avx2|

###### Benchmark ARM Neon: Apple M1 3,5GHz (clang 12.0)
|E MB/s|size|ratio| D MB/s| 50,000 bytes (2023.08) |
|--------|-----------:|------:|----------:|-----------------------------|
|24012.43|66668|133.34%|15352.09|tb64v128 (turbo-base64)|
|19087.55|66668|133.34%|12515.17|b64neon64 (aklomp/base64)|
|5611.48|66668|133.34%|5092.64|tb64s|
|9782.45|66668|133.34%|6798.98|tb64x|
|6181.37|66668|133.34%|3108.54|b64plain|
|45566.16|50000|100.00%|45484.13|memcpy|

###### Benchmark ARM Neon: ARMv8 A73-ODROID-N2 1.8GHz (clang 6.0)
|E Size|ratio%|E MB/s|D MB/s|Name|30MB binary 2019.12|
|--------:|-----:|--------:|--------:|----------------|----------------|
|40000000|133.3|**2026**|**1650**|[**TB64neon**](https://github.com/powturbo/TurboBase64)|**Turbo Base64 Neon**|
|40000000|133.3|1795|1285|[b64neon64](https://github.com/aklomp/base64)|Base64 Neon|
|40000000|133.3|**1270**|**1095**|[**TB64x**](https://github.com/powturbo/TurboBase64)|**Turbo Base64 scalar**|
|40000000|133.3|695|965|[TB64s](https://github.com/powturbo/TurboBase64)|**Turbo Base64 scalar**|
|40000000|133.3|512|782|[fb64neon](https://github.com/lemire/fastbase64)|Fastbase64 SIMD Neon|
|40000000|133.3|565|460|[Chrome](https://github.com/lemire/fastbase64)|Google Chrome base64|
|40000000|133.3|642|614|[b64plain](https://github.com/aklomp/base64)|Base64 plain|
|40000000|133.3|506|548|[fb64plain](https://github.com/lemire/fastbase64)|Fastbase64 plain|
|40500000|135.4|314|91|[Linux](https://github.com/lemire/fastbase64)|Linux base64|
|30000000|100.0|3820|3834|memcpy||

- (**bold** = pareto in category)  MB=1.000.000<br />
- (E/D) : Encode/Decode
- Timmings are respectively relative to the base64 output/input in encode/decode.


<p>

## Compile: (Download or clone Turbo Base64 SIMD)
        git clone https://github.com/powturbo/Turbo-Base64.git
        make

## Usage: (Benchmark App)

        ./tb64app file 
        or  
        ./tb64app

## Function usage:

>**static inline unsigned turbob64len(unsigned n)**<br />
	Base64 output length after encoding

>**unsigned tb64enc(const unsigned char *in, unsigned inlen, unsigned char *out)**<br />
	Encode binary input 'in' buffer into base64 string 'out'<br />
	with automatic cpu detection for simd and switch (sse/avx2/scalar<br />
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
- Linux arm: aarch64 ARMv8 Neon: gcc (>=6.3) 
- Linux arm: aarch64 ARMv8 Neon: clang (>=6.0) 
- MaxOS: XCode (>=9), apple M1
- PowerPC ppc64le: gcc (>=8.0) incl. SIMD Altivec

###### References:
- [fastbase v2022.02](https://github.com/lemire/fastbase64)
- [base64 v2022.02](https://github.com/aklomp/base64)
- [base64simd](https://github.com/WojciechMula/base64simd)

###### * **SIMD Base64 publications:**
  * :green_book:[Faster Base64 Encoding and Decoding Using AVX2 Instructions](https://arxiv.org/abs/1704.00605)
  * :green_book:[RFC 4648:The Base16, Base32, and Base64 Data Encodings](https://tools.ietf.org/html/rfc4648)

Last update: 06 AUG 2023
