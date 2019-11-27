# powturbo  (c) Copyright 2016-2019
# Linux: "export CC=clang" "export CXX=clang". windows mingw: "set CC=gcc" "set CXX=g++" or uncomment the CC,CXX lines
CC ?= gcc
CXX ?= g++

#CC=powerpc64le-linux-gnu-gcc

#------- OS/ARCH -------------------
ARCH=x86_64
ifneq (,$(filter Windows%,$(OS)))
  OS := Windows
  CC=gcc
  CXX=g++
else
  OS := $(shell uname -s)
  ARCH := $(shell uname -m)
$(info ARCH="$(ARCH)")

ifneq (,$(findstring aarch64,$(CC)))
  ARCH = aarch64
else ifneq (,$(findstring powerpc64le,$(CC)))
  ARCH = ppc64le
endif
endif

ifeq ($(ARCH),ppc64le)
  CFLAGS=-mcpu=power9 -mtune=power9
  MSSE=-D__SSE__ -D__SSE2__ -D__SSE3__ -D__SSSE3__ -DNO_WARN_X86_INTRINSICS

else ifeq ($(ARCH),aarch64)
  CFLAGS+=-march=armv8-a
ifneq (,$(findstring clang, $(CC)))
  CFLAGS+=-march=armv8-a -falign-loops -fomit-frame-pointer
else
  CFLAGS+=-march=armv8-a
endif
  MSSE=-march=armv8-a
else
  CFLAGS=-march=native
  MSSE=-mssse3
endif


ifeq ($(OS),$(filter $(OS),Linux GNU/kFreeBSD GNU OpenBSD FreeBSD DragonFly NetBSD MSYS_NT Haiku))
LDFLAGS+=-lrt
endif

all: tb64app

ifeq ($(FULLCHECK),1)
DEFS+=-DB64CHECK
endif

turbob64c.o: turbob64c.c
	$(CC) -O3 $(MARCH) $(DEFS) -fstrict-aliasing -falign-loops $< -c -o $@ 

turbob64d.o: turbob64d.c
	$(CC) -O3 $(MARCH) $(DEFS) -fstrict-aliasing -falign-loops $< -c -o $@ 

turbob64sse.o: turbob64sse.c
	$(CC) -O3 $(MSSE) $(DEFS) -fstrict-aliasing -falign-loops $< -c -o $@ 

turbob64avx.o: turbob64sse.c
	$(CC) -O3 $(DEFS) -march=corei7-avx -mtune=corei7-avx -mno-aes -fstrict-aliasing -falign-loops $< -c -o turbob64avx.o 

turbob64avx2.o: turbob64avx2.c
	$(CC) -O3 -march=haswell -fstrict-aliasing -falign-loops $< -c -o $@ 

LIB=turbob64c.o turbob64d.o 
ifneq ($(NSIMD),1)
ifeq ($(ARCH),$(filter $(ARCH),x86_64,aarch64,ppc64le))
LIB+=turbob64sse.o
endif
ifeq ($(ARCH),x86_64)
LIB+=turbob64avx.o turbob64avx2.o
endif
endif

tb64app: $(LIB) tb64app.o
	$(CC) $(LIB) tb64app.o $(LDFLAGS) -o tb64app
 
.c.o:
	$(CC) -O3 $(CFLAGS)  $(MARCH) $< -c -o $@

clean:
	@find . -type f -name "*\.o" -delete -or -name "*\~" -delete -or -name "core" -delete -or -name "turbob64"

cleanw:
	del /S *.o

