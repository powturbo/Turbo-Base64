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
// Turbase64: TB64app.c - Benchmark app

#include <string.h> 
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef __APPLE__
#include <sys/malloc.h>
#else
#include <malloc.h>
#endif
  #ifdef _MSC_VER
#include "vs/getopt.h"
  #else
#include <getopt.h> 
#endif

#include "turbob64.h"
#include "time_.h"

int memcheck(unsigned char *in, unsigned n, unsigned char *cpy) { 
  int i;
  for(i = 0; i < n; i++)
    if(in[i] != cpy[i]) {
      printf("ERROR in[%d]=%x, dec[%d]=%x\n", i, in[i], i, cpy[i]);
	  return i+1; 
	}
  return 0;
}

void pr(unsigned l, unsigned n) { double r = (double)l*100.0/n; if(r>0.1) printf("%10u %6.2f%% ", l, r);else printf("%10u %7.3f%%", l, r); fflush(stdout); }

#define ID_MEMCPY 7
void bench(unsigned char *in, unsigned n, unsigned char *out, unsigned char *cpy, int id) { 
  unsigned l;
  
  memrcpy(cpy,in,n); 
  switch(id) {
    case 1:                    TMBENCH("",l=tb64senc(   in, n, out),n); pr(l,n); TMBENCH2("tb64s",      tb64sdec(out, l, cpy), n);     break;
    case 2:                    TMBENCH("",l=tb64xenc(   in, n, out),n); pr(l,n); TMBENCH2("tb64x",      tb64xdec( out, l, cpy), n);    break;
    case 3:                    TMBENCH("",l=tb64enc(    in, n, out),n); pr(l,n); TMBENCH2("tb64(auto)", tb64dec(out, l, cpy), n);      break;
    case 4:if(cpuini(0)>=33) { TMBENCH("",l=tb64sseenc( in, n, out),n); pr(l,n); TMBENCH2("tb64sse",    tb64ssedec(out, l, cpy), n); } break;
      #if defined(__i386__) || defined(__x86_64__)
    case 5:if(cpuini(0)>=50) { TMBENCH("",l=tb64avxenc( in, n, out),n); pr(l,n); TMBENCH2("tb64avx",    tb64avxdec( out, l, cpy), n); break; } else return;
    case 6:if(cpuini(0)>=52) { TMBENCH("",l=tb64avx2enc(in, n, out),n); pr(l,n); TMBENCH2("tb64avx2",   tb64avx2dec(out, l, cpy), n); break; } else return;
      #else
    case 5:
    case 6:return;  
      #endif
      break;
    case ID_MEMCPY:            TMBENCH( "", memcpy(out,in,n) ,n);       pr(n,n); TMBENCH2("memcpy",     memcpy( cpy,out,n) ,n);        break;
	default: return;
  }
  printf("\n");
  memcheck(in,n,cpy);
}

void usage(char *pgm) {
  fprintf(stderr, "\nTurboBase64 Copyright (c) 2016-2019 Powturbo %s\n", __DATE__);
  fprintf(stderr, "Usage: %s [options] [file]\n", pgm);
  fprintf(stderr, " -e#      # = function ids separated by ',' or ranges '#-#' (default='1-%d')\n", ID_MEMCPY);
  fprintf(stderr, " -B#s     # = max. benchmark filesize (default 1GB) ex. -B4G\n");
  fprintf(stderr, "          s = modifier s:B,K,M,G=(1, 1000, 1.000.000, 1.000.000.000) s:k,m,h=(1024,1Mb,1Gb). (default m) ex. 64k or 64K\n");
  fprintf(stderr, "Benchmark:\n");
  fprintf(stderr, " -i#/-j#  # = Minimum  de/compression iterations per run (default=auto)\n");
  fprintf(stderr, " -I#/-J#  # = Number of de/compression runs (default=3)\n");
  fprintf(stderr, " -e#      # = function id\n");
  fprintf(stderr, " -q#      # = cpuid (33:sse, 50:avx, 52:avx2 default:auto detect). Only for tb64enc/tb64dec)\n");
  fprintf(stderr, "Ex. turbob64 file\n");
  fprintf(stderr, "    turbob64 -e3 file\n");
  fprintf(stderr, "    turbob64 -q33 file\n");
  fprintf(stderr, "    turbob64 -q33 file -I15 -J15\n");
  exit(0);
} 

int main(int argc, char* argv[]) {  							
  unsigned cmp=1, b = 1 << 30, esize=4, lz=0, fno,id=0;
  char     *scmd = NULL;
  tm_Rep  = 15;  
  tm_Rep2 = 15;

  int      c, digit_optind = 0, this_option_optind = optind ? optind : 1, option_index = 0;
  static struct option long_options[] = { {"blocsize", 	0, 0, 'b'}, {0, 0, 0}  };
  for(;;) {
    if((c = getopt_long(argc, argv, "B:ce:i:I:j:J:q:", long_options, &option_index)) == -1) break;
    switch(c) {
      case  0 : printf("Option %s", long_options[option_index].name); if(optarg) printf (" with arg %s", optarg);  printf ("\n"); break;								
      case 'B': b = argtoi(optarg,1); 	break;
      case 'c': cmp++; 				  	break;
	  case 'e': scmd   = optarg; break;
      case 'i': if((tm_rep  = atoi(optarg))<=0) tm_rep =tm_Rep=1; break;
      case 'I': if((tm_Rep  = atoi(optarg))<=0) tm_rep =tm_Rep=1; break;
      case 'j': if((tm_rep2 = atoi(optarg))<=0) tm_rep2=tm_Rep2=1; break;
      case 'J': if((tm_Rep2 = atoi(optarg))<=0) tm_rep2=tm_Rep2=1; break;
	  case 'q': cpuini(atoi(optarg));  break;
      default: 
        usage(argv[0]);
        exit(0); 
    }
  }
  if(argc - optind < 1) { fprintf(stderr, "File not specified\n"); exit(-1); }
  {
    unsigned char *in,*out,*cpy;
    uint64_t totlen=0,tot[3]={0};
    for(fno = optind; fno < argc; fno++) {
      uint64_t flen;
      int n,i;	  
      char *inname = argv[fno];  									
      FILE *fi = fopen(inname, "rb"); 							if(!fi ) { perror(inname); continue; } 	
      fseek(fi, 0, SEEK_END); 
      flen = ftell(fi); 
	  fseek(fi, 0, SEEK_SET);
	
      if(flen > b) flen = b;
      n = flen; 
      if(!(in  =        (unsigned char*)malloc(n+64)))                 { fprintf(stderr, "malloc error\n"); exit(-1); } cpy = in;
      if(!(out =        (unsigned char*)malloc(turbob64len(flen)+64))) { fprintf(stderr, "malloc error\n"); exit(-1); } 
      if(cmp && !(cpy = (unsigned char*)malloc(n+64)))                 { fprintf(stderr, "malloc error\n"); exit(-1); }
      n = fread(in, 1, n, fi);											 printf("File='%s' Length=%u\n", inname, n);			
      fclose(fi);
      if(n <= 0) exit(0);
      tm_init(tm_Rep, tm_Rep2);  
      tb64ini(0); 
      printf("detected simd=%s\n\n", cpustr(cpuini(0))); 

      printf("  E MB/s    size     ratio    D MB/s   function\n");  
	  char *p = scmd?scmd:"1-10"; 
	  do { 
        unsigned id = strtoul(p, &p, 10),idx = id, i;    
	    while(isspace(*p)) p++; if(*p == '-') { if((idx = strtoul(p+1, &p, 10)) < id) idx = id; if(idx > ID_MEMCPY) idx = ID_MEMCPY; } 
	    for(i = id; i <= idx; i++)
          bench(in,n,out,cpy,i);    
	  } while(*p++);
    }
  }
}

