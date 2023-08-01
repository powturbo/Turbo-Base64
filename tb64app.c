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
// Turbo-Base64: TB64app.c - Benchmark app

#include <string.h> 
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
  #if !defined(_WIN32)  
#include <sys/mman.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>
  #else
#include <io.h> 
#include <fcntl.h>
  #endif

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

#include "conf.h"
#include "turbob64.h"
#include "time_.h"
    #ifdef _WIN32 
int getpagesize_() {
  static int pagesize = 0;
  if (pagesize == 0) {
    SYSTEM_INFO system_info;
    GetSystemInfo(&system_info);
    pagesize = max(system_info.dwPageSize,
                        system_info.dwAllocationGranularity);
  }
  return pagesize;
} 
  #endif

#include "turbob64_.h"

  #ifdef XBASE64
#define FAC 2
#include "xb64test.h"
  #else
#define FAC 1
  #endif
  
  #ifdef CRZY
#include "crzy64/crzy64.h"
  #endif
//------------------------------- malloc ------------------------------------------------
#define USE_MMAP
  #if __WORDSIZE == 64
#define MAP_BITS 30
  #else
#define MAP_BITS 28
  #endif

void *_valloc(size_t size, unsigned a) {
  if(!size) return NULL;
    #ifdef _WIN32
  return VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    #elif defined(USE_MMAP) && !defined(__APPLE__)
  void *ptr = mmap(NULL/*0(size_t)a<<MAP_BITS*/, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  if(ptr == MAP_FAILED) return NULL;                                                        
  return ptr;
    #else
  return malloc(size); 
    #endif
}

void _vfree(void *p, size_t size) {
  if(!p) return;
    #ifdef _WIN32
  VirtualFree(p, 0, MEM_RELEASE);
    #elif defined(USE_MMAP) && !defined(__APPLE__)
  munmap(p, size);
    #else
  free(p);
    #endif
} 

int memcheck(unsigned char *in, unsigned n, unsigned char *cpy) { 
  int i;
  for(i = 0; i < n; i++)
    if(in[i] != cpy[i]) {
      printf("ERROR in[%d]=%x, dec[%d]=%x\n", i, in[i], i, cpy[i]);
      return i+1; 
    }
  return 0;
}
 
#define ID_MEMCPY 10
unsigned bench(unsigned char *in, unsigned n, unsigned char *out, unsigned char *cpy, int id) { 
  unsigned l = 0,m=tb64enclen(n);
    #ifndef _MSC_VER
  memrcpy(cpy,in,n); 
    #endif
  switch(id) {
    case 1:                      TMBENCH("",l=tb64senc(    in, n, out),m); pr(l,n); TMBENCH2(" 1:tb64s          ", tb64sdec(    out, l, cpy), l);   break;
    case 2:                      TMBENCH("",l=tb64xenc(    in, n, out),m); pr(l,n); TMBENCH2(" 2:tb64x          ", tb64xdec(    out, l, cpy), l);   break;
      #if defined(__i386__) || defined(__x86_64__) || defined(__ARM_NEON) || defined(__powerpc64__)
    case 3:if(cpuini(0)>=0x33) { TMBENCH("",l=tb64v128enc( in, n, out),m); pr(l,n); TMBENCH2(" 3:tb64v128       ", tb64v128dec( out, l, cpy), l); } break;
    case 6:                      TMBENCH("",l=tb64enc(     in, n, out),m); pr(l,n); TMBENCH2(" 6:tb64auto       ", tb64dec(     out, l, cpy), l);   break;
      #endif
      #if defined(__i386__) || defined(__x86_64__)
    case 4:if(cpuini(0)>=0x50) { TMBENCH("",l=tb64v128aenc(in, n, out),m); pr(l,n); TMBENCH2(" 4:tb64v128a avx  ", tb64v128adec(out, l, cpy), l); } break;
    case 5:if(cpuini(0)>=0x60) { TMBENCH("",l=tb64v256enc( in, n, out),m); pr(l,n); TMBENCH2(" 5:tb64v256  avx2 ", tb64v256dec( out, l, cpy), l); } break;
    case 7:if(cpuini(0)>=0x60) { TMBENCH("",l=_tb64v256enc(in, n, out),m); pr(l,n); TMBENCH2(" 7:_tb64v256 avx2 ", _tb64v256dec(out, l, cpy), l); } break;
        #ifndef NAVX512                                                            // +VBMI
    case 8:{ unsigned c = cpuini(0); 
      if(c>=(0x800|0x200)) { TMBENCH("",  l=tb64v512enc(in, n, out),m); pr(l,n); TMBENCH2(" 8:tb64v512vbmi   ", tb64v512dec( out,l,cpy),l); } 
      //else if(c>=0x800)    { TMBENCH("",  l=tb64v256enc(in, n, out),m); pr(l,n); TMBENCH2(" 8:tb64v512       ", tb64v512dec0(out, l, cpy),l); } 
    } break;
        #endif 
	  #endif
    case 9:                      TMBENCH("",l=tb64xenc(    in, n, out),m); pr(l,n); TMBENCH2(" 9:_tb64x         ", _tb64xd(     out, l, cpy), l);   break;
    case ID_MEMCPY:              TMBENCH( "", memcpy(out,in,m) ,m);        pr(n,n); TMBENCH2("10:memcpy         ", memcpy(cpy,out,n), n);  l = n;   break;
      #ifdef XBASE64
    #include "xtb64test_.c"
      #endif
    default: return 0;
  }
  if(l) { printf(" %10d\n", n); memcheck(in,n,cpy); }
  return l;
}

void usage(char *pgm) {
  fprintf(stderr, "\nTurboBase64 Copyright (c) 2016-2023 Powturbo %s\n", __DATE__);
  fprintf(stderr, "Usage: %s [options] [file]\n", pgm);
  fprintf(stderr, " -e#      # = function ids separated by ',' or ranges '#-#' (default='1-%d')\n", ID_MEMCPY);
  fprintf(stderr, " -B#s     # = max. benchmark filesize (default 120Mb)\n");
  fprintf(stderr, "          s = modifier s:B,K,M,G=(1, 1000, 1.000.000, 1.000.000.000) s:k,m,h=(1024,1Mb,1Gb). (default m) ex. 64k or 64K\n");
  fprintf(stderr, "Benchmark:\n");
  fprintf(stderr, " -I#/-J#  # = Number of de/compression runs (default=3)\n");
  fprintf(stderr, " -e#      # = function id\n");
  fprintf(stderr, " -q#      # = cpuid (33=ssse, 50:avx, 60=avx2 (default:auto detect)\n");
  fprintf(stderr, " -k#      random test with predefined sizes (0=1K-20MB function id\n");
  fprintf(stderr, "          #: 0=1K-20MB, 1=short input\n");
  fprintf(stderr, " -T       Test all input sizes 0-10000\n");
  fprintf(stderr, "Ex. turbob64 file\n");
  fprintf(stderr, "    turbob64 -e3 file\n");
  fprintf(stderr, "    turbob64 -e1-6 file\n");
  fprintf(stderr, "    turbob64 -q33 file -I15 -J15\n");
  fprintf(stderr, "    turbob64 -e1-6 -k1\n");
  exit(0);
} 


void fuzzcheck(unsigned char *_in, unsigned insize, unsigned char *_out, unsigned outsize, unsigned char *_cpy, unsigned fuzz) {
  unsigned char *in = _in, *out = _out, *cpy = _cpy;                            printf(" Fuzz OK. Waiting seg. fault\n");fflush(stdout);
  unsigned      n;
  for(n = 0; n <= 10099; n++) {  												
    unsigned m = tb64enclen(n); 
    if(fuzz & 2) { cpy = (_cpy+insize) - n, out = (_out+outsize) - m;           printf("O%x ", out[m]);fflush(stdout); 
                                                                                printf("C%x ", cpy[n]);fflush(stdout); 
    }
    if(fuzz & 1) { in  = (_in +insize) - n;                                     printf("I%x ", in[n]); fflush(stdout); }
  }                      														printf("Fuzztest not reliable. Reapeat until seg. fault\n");fflush(stdout);
}

void fuzztest(unsigned id, unsigned char *_in, unsigned insize, unsigned char *_out, unsigned outsize, unsigned char *_cpy, unsigned fuzz) {
  unsigned char *in = _in, *out = _out, *cpy = _cpy;
  unsigned      s,n,i,l = 0;
                                                                                printf("[id=%u", id);fflush(stdout);
  for(n = 0; n <= 10099; n++) {  																					
    unsigned m = tb64enclen(n); 
    if(fuzz & 1) in  = (_in +insize) - n;                                       // move the i/o buffers to the end, this will normally (but not always) cause a seg fault
    if(fuzz & 2) cpy = (_cpy+insize) - n, out = (_out+outsize) - m;             // by reading/writing beyond the buffer end
    			
    srand(time(0));                                                             
	for(i = 0; i < n; i++)                                                      // Generate a random string 
	  in[i] = rand()&0xff;  
    memrcpy(cpy, in, n);   														// copy input reversed 
    
    switch(id) {
      case  1:                       l = tb64senc(    in, n, out); if(l != m) die("Error n=%u\n", n); tb64sdec(    out, l, cpy);   break;
      case  2:                       l = tb64xenc(    in, n, out); if(l != m) die("Error n=%u\n", n); tb64xdec(    out, l, cpy);   break;
      case  3: if(cpuini(0)>=0x33) { l = tb64v128enc( in, n, out); if(l != m) die("Error n=%u\n", n); tb64v128dec( out, l, cpy); } break;
        #if defined(__i386__) || defined(__x86_64__)
      case  4: if(cpuini(0)>=0x50) { l = tb64v128aenc(in, n, out); if(l != m) die("Error n=%u\n", n); tb64v128adec(out, l, cpy); } break;
      case  5: if(cpuini(0)>=0x60) { l = tb64v256enc( in, n, out); if(l != m) die("Error n=%u\n", n); tb64v256dec( out, l, cpy); } break;
      case  8: if(cpuini(0)>=(0x800|0x200)) { l = tb64v512enc( in, n, out); if(l != m) die("Error n=%u\n", n); tb64v512dec( out, l, cpy); } break;
      case  11: if(cpuini(0)>=0x60) { l = _tb64v256enc(in, n, out); if(l != m) die("Error n=%u\n", n);_tb64v256dec(out, l, cpy); } break; //unsafe when OVHD=0
        #endif
        #ifdef XBASE64F // fastbase is unsafe, can reads/writes beyound i/o buffers
      #include "xtb64fuzz_.c"  
	#endif
      default:                                                                  printf("]");fflush(stdout);
	return;
    }                                                                           
    if(l && memcheck(in,n,cpy)) exit(-1); 	                                    									
  }                                                                             printf(" OK]");fflush(stdout);
}

int verbose=3;

int main(int argc, char* argv[]) {                              
  unsigned      cmp = 1, bsize = (120*Mb), esize = 4, fno, id=0, fuzz = 0, bid = 0, tst = 0,
                n = bsize, outsize = tb64enclen(n), insize = outsize;
  char          *scmd = NULL, _scmd[33];
  unsigned char *in, *_in=NULL, *cpy, *_cpy=NULL, *out, *_out=NULL;

  int           c, digit_optind = 0, this_option_optind = optind ? optind : 1, option_index = 0;
  static struct option long_options[] = { {"blocsize",  0, 0, 'b'}, {0, 0, 0}  };
  for(;;) {
    if((c = getopt_long(argc, argv, "B:e:f:I:J:k:m:M:q:Tv:", long_options, &option_index)) == -1) break;
    switch(c) {
      case  0 : printf("Option %s", long_options[option_index].name); if(optarg) printf (" with arg %s", optarg);  printf ("\n"); break;                                
      case 'B': bsize = argtoi(optarg,1);                             break;
      case 'k': bid = atoi(optarg);                                   break;
      case 'f': fuzz = atoi(optarg);                                  break;
      case 'e': scmd = optarg;                                        break;
      case 'I': if((tm_Rep  = atoi(optarg))<=0) tm_rep = tm_Rep  = 1; break;
      case 'J': if((tm_Rep2 = atoi(optarg))<=0) tm_rep = tm_Rep2 = 1; break;
        #ifdef XBASE64
      case 'm': if(!(smin = atoi(optarg))) smin = 1;                  break;
      case 'M': smask = atoi(optarg); if(smask&(smask-1)) die("Range must be power of 2"); smask--; break;
        #endif	  
      case 'q':      if(!strcasecmp(optarg,"sse"))    cpuini(0x33);  
                else if(!strcasecmp(optarg,"avx"))    cpuini(0x50); 
                else if(!strcasecmp(optarg,"avx2"))   cpuini(0x60); 
                else if(!strcasecmp(optarg,"avx512")) cpuini(0x78);   break;
      case 'T': tst++;                                                break;
	  case 'v': verbose = atoi(optarg); break;
      default: 
        usage(argv[0]);
        exit(0); 
    }
  }
  
  tm_init(tm_Rep, verbose);  
  sprintf(_scmd, "1-%d", ID_MEMCPY);
  tb64ini(0,0); 																printf("detected simd (id=%x->'%s')\n\n", cpuini(0), cpustr(cpuini(0))); 
  unsigned char *tmp1,*tmp2;
  if(!(_in  = (unsigned char*)_valloc(insize, 0))) die("malloc error in size=%u\n",  insize);  in  = _in;
  if(!(tmp1 = (unsigned char*)_valloc(insize, 0))) die("malloc error cpy size=%u\n", insize);  
  if(!(_cpy = (unsigned char*)_valloc(insize, 0))) die("malloc error cpy size=%u\n", insize);  cpy = _cpy;
  if(!(tmp2 = (unsigned char*)_valloc(outsize,0))) die("malloc error cpy size=%u\n", insize);  
  if(!(_out = (unsigned char*)_valloc(outsize,0))) die("malloc error out size=%u\n", outsize); out = _out;
  _vfree(tmp1, insize); 
  _vfree(tmp2, outsize);
                                                                                
  if(tst) {	//------------------------ test + fuzzer (option -T) ----------------------------------------------------------------------------
    char *p = scmd?scmd:_scmd;
    do { 
      unsigned id = strtoul(p, &p, 10), idx = id, i;    
      while(isspace(*p)) p++; 
	  if(*p == '-' && (idx = strtoul(p+1, &p, 10)) < id)
	    idx = id;  
      for(i = id; i <= idx; i++) 
	    fuzztest(i,_in,insize,_out,outsize,_cpy, fuzz);
	  fuzzcheck(_in,insize,_out,outsize,_cpy, fuzz);
	
    } while(*p++);															    printf("fuzz OK\n");
  } else if(argc - optind < 1) { //------------------ bechmark with predefined sizes (option -k#) -------------------------------------------
    unsigned _size0[] = { 1*KB, 10*KB, 50*KB, 100*KB, 200*KB, 500*KB, 1*MB, 10*MB, 20*MB, 0 }, 
	         _size1[] = { 1, 3, 7, 15, 31, 67, 127, 255, 511, 1*KB, 0 },
             _size2[] = { 12, 15,16,17, 31,32,33, 47,48,50, 63,64,65, 95,96,97, 120, 180, 250, 500, 1*KB, 10*KB, 50*KB, 100*KB, 500*KB, 1*MB, 0 }, 
             _size3[] = { 100, 200, 1*KB, 100*KB, 1*MB, 0 }, 
	         _size4[] = { 15, 63, 127, 159, 191, 255, 511, 1023, 0 },
	         _size5[] = { 1,2, 3,4,5,6, 63,64,65,66, 0 },
		    *_sizea[] = { _size0, _size1, _size2, _size3, _size4, _size5 },
			 *sizes   = _sizea[bid>5?5:bid];
	if(bid > 5) { _size5[0] = bid; _size5[1] = 0; }
                                                                                printf("  E MB/s    size     ratio%%   D MB/s   function (random)\n");   
    for(int s = 0; sizes[s]; s++) {
      n = sizes[s];  if(n > bsize) continue;
      
      if(fuzz & 1) in  = (_in +insize) - n; 
      if(fuzz & 2) out = (_out+outsize) - tb64enclen(n), cpy = (_cpy+insize)-n;
	  
      srand(0); for(int i = 0; i < n; i++) in[i] = rand()&0xff;
      char *p = scmd?scmd:_scmd;
      do { 
        unsigned id = strtoul(p, &p, 10),idx = id, i;    
        while(isspace(*p)) p++; 
	    if(*p == '-' &&  (idx = strtoul(p+1, &p, 10)) < id)
		  idx = id;  
        for(i = id; i <= idx; i++) 
          bench(in, n, out,cpy,i);
      } while(*p++);
    } 
  } else { //------------------------------------ file benchmark -----------------------------------------------------------------------------
    for(fno = optind; fno < argc; fno++) {
      unsigned flen,m,n=bsize,i;    
      char *inname = argv[fno];
	  if(strcmp(inname, "NULL")) {
        FILE *fi = fopen(inname, "rb");                                           if(!fi ) { perror(inname); continue; }  
        fseek(fi, 0, SEEK_END); 
        flen = ftell(fi); 													 	
        fseek(fi, 0, SEEK_SET);
    
        if(flen > bsize) flen = bsize;                                           
        n = flen;
        n = fread(in, 1, n, fi);                                                  printf("File='%s' Length=%u\n", inname, n);            
        fclose(fi);
	  }
	  m = tb64enclen(n);
      if(!n) exit(0);
																				printf("  E MB/s    size     ratio    D MB/s   function\n");  
      char *p = scmd?scmd:_scmd;
      do { 
        unsigned id = strtoul(p, &p, 10),idx = id, i;    
        while(isspace(*p)) p++; 
		if(*p == '-' && (idx = strtoul(p+1, &p, 10)) < id) 
		  idx = id; 
        for(i = id; i <= idx; i++)
          bench(in,n,out,cpy,i);    
      } while(*p++);
    }
  }
  _vfree(_in,  insize);
  _vfree(_out, outsize);
  _vfree(_cpy, insize);
}
