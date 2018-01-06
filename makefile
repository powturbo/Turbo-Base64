# powturbo  (c) Copyright 2016-2018
CC ?= gcc
CXX ?= g++
#CC=clang
#CXX=clang++

ifeq ($(OS),Windows_NT)
  UNAME := Windows
CC=gcc
CXX=g++
else
  UNAME := $(shell uname -s)
ifeq ($(UNAME),$(filter $(UNAME),Linux Darwin FreeBSD GNU/kFreeBSD))
LDFLAGS+= -lrt
endif
endif

CFLAGS=-march=native

all: turbob64

turbob64: turbob64c.o turbob64d.o turbob64.o
	$(CC) turbob64c.o turbob64d.o turbob64.o $(LDFLAGS) -o turbob64
 
.c.o:
	$(CC) -O3 $(CFLAGS) $< -c -o $@

clean:
	rm  *.o

cleanw:
	del .\*.o
