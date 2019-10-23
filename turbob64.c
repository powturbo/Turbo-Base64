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
#include <string.h> 
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
  #ifdef _MSC_VER
#include "vs/getopt.h"
  #else
#include <getopt.h> 
#endif

#include "turbob64.h"
   
//-------------------------------------- Time ---------------------------------------------------------------------  
typedef unsigned long long tm_t;
#define TM_T 1000000.0
#define TM_MAX (1ull<<63)
  #ifdef _WIN32
#include <windows.h>
static LARGE_INTEGER tps;
static tm_t tmtime(void) { LARGE_INTEGER tm; QueryPerformanceCounter(&tm); return (tm_t)(tm.QuadPart*1000000/tps.QuadPart); }
static tm_t tminit() { QueryPerformanceFrequency(&tps); tm_t t0=tmtime(),ts; while((ts = tmtime())==t0); return ts; } 
  #else
#include <time.h>
static   tm_t tmtime(void)    { struct timespec tm; clock_gettime(CLOCK_MONOTONIC, &tm); return (tm_t)tm.tv_sec*1000000ull + tm.tv_nsec/1000; }
static   tm_t tminit()        { tm_t t0=tmtime(),ts; while((ts = tmtime())==t0) {}; return ts; }
  #endif
//---------------------------------------- bench ---------------------------------------------------------------------
#define TMPRINT(__x) { printf("%7.2f MB/s\t%s", (double)(tm>=0.000001?(((double)n*rm/MBS)/(((double)tm/1)/TM_T)):0.0), __x); fflush(stdout); }
#define TMDEF unsigned r,t,rm; tm_t tx = 2000000,t0,tc,tm
#define TMBEG for(tm = TM_MAX,t = 0; t < trips; t++) {  for(t0 = tminit(), r=0; r < reps;) { 
#define TMEND                                            r++; if((tc = tmtime() - t0) > tx) break; } if(tc < tm) tm = tc,rm=r; }
#define MBS 1000000.0 //MiBS 1048576.0

unsigned argtoi(char *s) {
  char *p; unsigned n = strtol(s, &p, 10),f=1; 
  switch(*p) {
     case 'k': f = 1000;       break;
     case 'm': f = 1000000;    break;
     case 'g': f = 1000000000; break;
     case 'K': f = 1<<10; 	   break;
     case 'M': f = 1<<20; 	   break;
     case 'G': f = 1<<30; 	   break;
  }
  return n*f;
}
//-------------------------------------------------------------------------------------------------------------------------------------
void check(const unsigned char *in, unsigned char *cpy, unsigned n) { int i;
  for(i = 0; i < n; i++) 
    if(in[i] != cpy[i]) { printf("ERROR at %d ", i); break; }
  memset(cpy,0xff,n); 
}

int main(int argc, char *argv[]) {
  unsigned reps = 1<<30, trips = 3,cmp=0, b = 1 << 30;
  int c, digit_optind = 0, this_option_optind = optind ? optind : 1, option_index = 0;
  static struct option long_options[] = { {"blocsize", 	0, 0, 'b'}, {0,0, 0, 0}  };
  for(;;) {
    if((c = getopt_long(argc, argv, "cb:r:R:", long_options, &option_index)) == -1) break;
    switch(c) {
      case  0 : printf("Option %s", long_options[option_index].name); if(optarg) printf (" with arg %s", optarg);  printf ("\n"); break;								
      case 'r': reps  = atoi(optarg); break;
      case 'R': trips = atoi(optarg); break;
      case 'b': b = argtoi(optarg);   break;
      case 'c': cmp++; 				  break;
    }
  }
  if(argc - optind < 1) { fprintf(stderr, "File not specified\n"); exit(-1); }

  unsigned char *in,*out,*cpy;
  char *inname = argv[optind];  
  FILE *fi = fopen(inname, "rb"); if(!fi ) perror(inname), exit(1);  							
  fseek(fi, 0, SEEK_END); long long flen = ftell(fi); fseek(fi, 0, SEEK_SET);
  if(flen > b) flen = b;
  int n = flen; 
  if(!(in  =        (unsigned char*)malloc(n+1024)))        { fprintf(stderr, "malloc error\n"); exit(-1); } cpy = in;
  if(!(out =        (unsigned char*)malloc(flen*4/3+1024))) { fprintf(stderr, "malloc error\n"); exit(-1); } 
  if(cmp && !(cpy = (unsigned char*)malloc(n+1024)))        { fprintf(stderr, "malloc error\n"); exit(-1); }
  n = fread(in, 1, n, fi);
  fclose(fi);
  if(n <= 0) exit(0); 																printf("'%s' %u\n", inname, n);
    
  unsigned l;
  TMDEF; memcpy(out, in,  n); memcpy(out,cpy,n);
  TMBEG l = turbob64enc( in, n, out); 		  TMEND; printf("%10u ", l); TMPRINT(""); TMBEG turbob64dec( out, l, cpy);	TMEND; if(cmp) check(in,cpy,n); TMPRINT("TurboB64\n");
  TMBEG l = turbob64encs(in, n, out); 		  TMEND; printf("%10u ", l); TMPRINT(""); TMBEG turbob64decs(out, l, cpy);	TMEND; if(cmp) check(in,cpy,n); TMPRINT("TurboB64\n");
  TMBEG memcpy(out, in,  n);   				  TMEND; printf("%10u ", n); TMPRINT(""); TMBEG memcpy(cpy,out, n); 		TMEND; if(cmp) check(in,cpy,n); TMPRINT("memcpy\n"); 
}
