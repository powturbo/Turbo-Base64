/**
Copyright (c) 2016-2017, Powturbo
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

//	   conf.h - config & common 
#ifndef CONF_H
#define CONF_H
//------------------------- Compiler ------------------------------------------
  #if defined(__GNUC__)
#define bswap32(x) __builtin_bswap32(x)
#define bswap64(x) __builtin_bswap64(x)

  #elif _MSC_VER //----------------------------------------------------
#define __builtin_prefetch(x) //_mm_prefetch(x, _MM_HINT_NTA)

#define bswap32(x) _byteswap_ulong(x)
#define bswap64(x) _byteswap_uint64(x)

#define sleep(x) Sleep(x/1000)
  #endif 

//--------------- Unaligned memory access -------------------------------------
  #if defined(__i386__) || defined(__x86_64__) || \
    defined(_M_IX86) || defined(_M_AMD64) || /*MSC_VER*/\
    defined(__powerpc__) ||\
    defined(__ARM_FEATURE_UNALIGNED) || defined(__aarch64__) || defined(__arm__) ||\
    defined(__ARM_ARCH_4__) || defined(__ARM_ARCH_4T__) || \
    defined(__ARM_ARCH_5__) || defined(__ARM_ARCH_5T__) || defined(__ARM_ARCH_5TE__) || defined(__ARM_ARCH_5TEJ__) || \
    defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6K__)  || defined(__ARM_ARCH_6T2__) || defined(__ARM_ARCH_6Z__)   || defined(__ARM_ARCH_6ZK__)
#define ctou32(_cp_) (*(unsigned       *)(_cp_))

    #if defined(__i386__) || defined(__x86_64__) || defined(__powerpc__)
#define ctou64(_cp_)       (*(unsigned long long *)(_cp_))
    #elif defined(__ARM_FEATURE_UNALIGNED)
struct _PACKED longu     { unsigned long long l; };
#define ctou64(_cp_) ((struct longu     *)(_cp_))->l
    #endif

  #elif defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7S__)
struct _PACKED unsignedu { unsigned           u; };
struct _PACKED longu     { unsigned long long l; };

#define ctou32(_cp_) ((struct unsignedu *)(_cp_))->u
#define ctou64(_cp_) ((struct longu     *)(_cp_))->l
  #else
#error "unknown cpu"	  
  #endif

  #ifndef ctou32
static inline unsigned           ctou32(const void *cp) { unsigned           x; memcpy(&x, cp, sizeof(x)); return x; }
  #endif

  #ifndef ctou64
static inline unsigned long long ctou64(const void *cp) { unsigned long long x; memcpy(&x, cp, sizeof(x)); return x; }
  #endif
#endif

