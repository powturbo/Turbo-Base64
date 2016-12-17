TurboBase64: Turbo Base64 Encoding [![Build Status](https://travis-ci.org/powturbo/TurboBase64.svg?branch=master)](https://travis-ci.org/powturbo/TurboBase64)
===================================

###### **Turbo Base64** Encoding library
- 100% C (C++ compatible headers), without inline assembly
<p>
- No other scalar base64 library encode or decode faster
- Encode or decode more than 2,5x faster as other libraries
- Up to 3 GB/s, saturates the fastest SSD drives
<p>


------------------------------------------------------------------------

## Benchmark:
CPU: Skylake at 3.7GHz, gcc 6.2, ubuntu 16.10
- with [TurboBench](https://github.com/powturbo/TurboBench)
- Single thread
- Realistic and practical benchmark with large binary game assets corpus [pd3d](http://www.cbloom.com/pd3d.7z)

|C Size|ratio%|C MB/s|D MB/s|Name|Descrition
|--------:|-----:|--------:|--------:|----------------|----------------|------------------------|
|42603868|133.3|**3025.39**|**2531.27**|[**TurboB64**](https://github.com/powturbo/TurboBase64)|**TurboBase64**|
|42603868|133.3|1715.36|1956.14|[TurboB64s](https://github.com/powturbo/TurboBase64)|**TurboBase64**|
|42603868|133.3|1262.20|1375.18|[fb64chromium](https://github.com/lemire/fastbase64)|**Google Chromium base64**|
|42603868|133.3|1674.74|1167.17|[fb64scalar](https://github.com/lemire/fastbase64)|**Scalar FastBase64**|
|43269553|135.4| 902.80|170.56|[fb64linux](https://github.com/lemire/fastbase64)|**Linux base64**|
|52024524|162.8|1121.98|816.12|[fb64quicktime](https://github.com/lemire/fastbase64)|**Apple Quicktime base64**|
|31952900|100.0|**13397.48**|**14447.93**|**memcpy**|

(**bold** = pareto)  MB=1.000.000

<p>

## Compile:

        make

## Usage:

        ./turbob64 file

### Environment:

###### OS/Compiler (32 + 64 bits):
- Linux: GNU GCC (>=4.6)
- clang (>=3.2) 
- Windows: MinGW
- Windows: Visual Studio 2015

Last update: 17 DEC 2016
