XDEFS+=-DXBASE64 
BASE64=1
#FASTBASE64=1
#CRZY=1

ifeq ($(BASE64),1)
XDEFS+=-D_BASE64 
#-DBASE64_STATIC_DEFINE
ifeq ($(ARCH),aarch64)
CFLAGS+=-DHAVE_NEON64 -Ibase64/lib
XLIB+=base64/lib/tables/tables.o base64/lib/arch/neon64/codec.o base64/lib/arch/generic/codec.o base64/lib/lib.o base64/lib/codec_choose.o \
  base64/lib/arch/avx512/codec.o base64/lib/arch/avx2/codec.o base64/lib/arch/neon32/codec.o base64/lib/arch/ssse3/codec.o base64/lib/arch/sse41/codec.o \
  base64/lib/arch/sse42/codec.o base64/lib/arch/avx/codec.o \

else
XLIB+=base64/lib/libbase64.o
endif
endif

ifeq ($(FASTBASE64),1)
XDEFS+=-D_FASTBASE64

fastbase64/src/scalarbase64.o:   CFLAGS=-Ifastbase64/include 
fastbase64/src/chromiumbase64.o: CFLAGS=-Ifastbase64/include
XLIB+=fastbase64/src/scalarbase64.o fastbase64/src/chromiumbase64.o

ifeq ($(ARCH),aarch64)
fastbase64/src/neonbase64.o:     CFLAGS=-Ifastbase64/include -marmv8-a
XLIB+=fastbase64/src/neonbase64.o
else
fastbase64/src/fastavxbase64.o:  CFLAGS=-Ifastbase64/include -mavx2 
XLIB+=fastbase64/src/fastavxbase64.o
endif
endif

ifeq ($(CRZY),1)
XDEFS+=-D_CRZY
endif
