XDEFS+=-DXBASE64 
BASE64=1
#FASTBASE64=1
#CRZY=1

ifeq ($(BASE64),1)
XDEFS+=-D_BASE64 -DBASE64_STATIC_DEFINE
XLIB+=base64/lib/libbase64.o
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
