# powturbo  (c) Copyright 2016-2019
# Linux: "export CC=clang" "export CXX=clang". windows mingw: "set CC=gcc" "set CXX=g++" or uncomment the CC,CXX lines
CC ?= gcc
CXX ?= g++

DDEBUG=-DNDEBUG -s
#DDEBUG=-g

ifneq (,$(filter Windows%,$(OS)))
  OS := Windows
else
  OS := $(shell uname -s)
  ARCH := $(shell uname -m)
ifneq (,$(findstring powerpc64le,$(CC)))
  ARCH = ppc64le
endif
ifneq (,$(findstring aarch64,$(CC)))
  ARCH = aarch64
endif
endif

ifeq ($(ARCH),ppc64le)
CFLAGS=-mcpu=power9 -mtune=power9
else
CFLAGS=-march=native
endif

ifeq ($(OS),$(filter $(OS),Linux GNU/kFreeBSD GNU OpenBSD FreeBSD DragonFly NetBSD MSYS_NT Haiku))
LDFLAGS+=-lrt
endif

all: turbob64

turbob64: turbob64c.o turbob64d.o turbob64.o
	$(CC) turbob64c.o turbob64d.o turbob64.o $(LDFLAGS) -o turbob64
 
.c.o:
	$(CC) -O3 $(CFLAGS) $< -c -o $@

clean:
	@find . -type f -name "*\.o" -delete -or -name "*\~" -delete -or -name "core" -delete -or -name "turbob64"

cleanw:
	del /S *.o
