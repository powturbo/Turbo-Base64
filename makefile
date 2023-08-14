# powturbo  (c) Copyright 2016-2023
# Linux: "export CC=clang" "export CXX=clang". windows mingw: "set CC=gcc" "set CXX=g++" or uncomment the CC,CXX lines
CC ?= gcc
CXX ?= g++
#CC=aarch64-linux-gnu-gcc
#CC=powerpc64le-linux-gnu-gcc

# uncomment to disable checking for more faster decoding
#NCHECK=1
#uncomment for full base64 checking (default=partial checking, detect allmost all errors)
#FULLCHECK=1
#uncomment for use memcpy instead of unaligned loads
#UAMEMCPY=1

#NAVX512=1
#NAVX2=1
#NSSE=1
#NAVX=1

#RDTSC=1
#XBASE64=1

#CFLAGS+=-DDEBUG -g
CFLAGS+=-DNDEBUG

#------- OS/ARCH -------------------
ifneq (,$(filter Windows%,$(OS)))
  OS := Windows
  ARCH=x86_64
else
  OS := $(shell uname -s)
  ARCH := $(shell uname -m)

ifneq (,$(findstring aarch64,$(CC)))
  ARCH = aarch64
else ifneq (,$(findstring arm64,$(ARCH)))
  ARCH = aarch64
else ifneq (,$(findstring powerpc64le,$(CC)))
  ARCH = ppc64le
endif
endif

ifeq ($(ARCH),ppc64le)
  CFLAGS=-mcpu=power9 -mtune=power9
  MSSE=-D__SSSE3__
else ifeq ($(ARCH),aarch64)
  CFLAGS+=-march=armv8-a 
ifneq (,$(findstring clang, $(CC)))
  CFLAGS+=-fomit-frame-pointer
endif
  MSSE=-march=armv8-a
else ifeq ($(ARCH),$(filter $(ARCH),x86_64 ppc64le))
  MSSE=-mssse3
endif
ifeq (,$(findstring clang, $(CC)))
  CFLAGS+=-falign-loops
endif
#---------------------------------------------------
ifeq ($(OS),$(filter $(OS),Linux GNU/kFreeBSD GNU OpenBSD FreeBSD DragonFly NetBSD MSYS_NT Haiku))
LDFLAGS+=-lrt
endif

ifeq ($(STATIC),1)
LDFLAGS+=-static
endif

FPIC=-fPIC

ifeq ($(NCHECK),1)
DEFS+=-DNB64CHECK
else
ifeq ($(FULLCHECK),1)
DEFS+=-DB64CHECK
endif
endif

ifeq ($(RDTSC),1)
DEFS+=-D_RDTSC
endif

ifeq ($(UAMEMCPY),1)
DEFS+=-DUA_MEMCPY
endif

ifeq ($(XBASE64),1)
include xtb64.mak
endif

all: tb64app libtb64.so libtb64.a

tb64app.o:       CFLAGS+=$(XDEFS) $(MARCH) 
turbob64c.o:     CFLAGS+=$(DEFS) $(FPIC) -fstrict-aliasing $(MARCH)
turbob64d.o:     CFLAGS+=$(DEFS) $(FPIC) -fstrict-aliasing $(MARCH)
turbob64v128.o:  CFLAGS+=$(DEFS) $(FPIC) -fstrict-aliasing $(MSSE)
turbob64v256.o:  CFLAGS+=$(DEFS) $(FPIC) -fstrict-aliasing -march=haswell
turbob64v512.o:  CFLAGS+=$(DEFS) $(FPIC) -fstrict-aliasing -march=skylake-avx512 -mavx512vbmi
turbob64v128a.o: turbob64v128.c
	$(CC) -O3 $(CFLAGS) $(DEFS) $(FPIC) -fstrict-aliasing -march=corei7-avx -mtune=corei7-avx -mno-aes $< -c -o turbob64v128a.o 

#_tb64.o: _tb64.c
#	$(CC) -O3 $(FPIC) -I/usr/include/python2.7 $< -c -o $@ 

LIB=turbob64c.o turbob64d.o turbob64v128.o
ifeq ($(ARCH),x86_64)
LIB+=turbob64v128a.o turbob64v256.o
ifneq ($(NAVX512),1)
LIB+=turbob64v512.o
else
DEFS+=-DNAVX512
endif
endif

#_tb64.so: _tb64.o
#	$(CC) -shared $^ -o $@

libtb64.a: $(LIB)
	ar cr $@ $+

libtb64.so: $(LIB)
	$(CC) -shared $^ -o $@
	
install:
	cp libtb64.so ~/.local/lib/
	cp libtb64.a ~/.local/lib/
	# FIXME: how does one build _tb64.so?
	# ./python/tb64/build.py
	# cp _tb64.so ~/.local/lib/

tb64app: $(LIB) $(XLIB) tb64app.o 
	$(CC) -O3 $(LIB) $(XLIB) $(XDEFS) tb64app.o $(LDFLAGS) -o tb64app

tb64bench: $(LIB) tb64bench.o 
	$(CC) -O3 $(LIB) tb64bench.o $(LDFLAGS) -o tb64bench

tb64test: $(LIB) tb64test.o 
	$(CC) -O3 $(LIB) tb64test.o $(LDFLAGS) -o tb64test
	
.c.o:
	$(CC) -O3 $(CFLAGS) $< -c -o $@

.cc.o:
	$(CXX) -O3 $(CXXFLAGS) $< -c -o $@

ifeq ($(OS),Windows)
clean:
	del /S *.o
	del *.a
	del *.so
else
clean:
	@find . -type f -name "*\.o" -delete -or -name "*\~" -delete -or -name "core" -delete -or -name "tb64app" -delete -or -name "_tb64.so" \
	-delete -or -name "_tb64.c" -delete -or -name "xlibtb64.so" -delete -or -name "libtb64.a" -delete -or -name "libtb64.so"
endif
