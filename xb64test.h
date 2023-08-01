#include "base64/include/libbase64.h"
#include "fastbase64/include/chromiumbase64.h"
#include "fastbase64/include/scalarbase64.h"
#include "fastbase64/include/linuxbase64.h"
#include "fastbase64/include/fastavxbase64.h"
  #ifdef HAVE_AVX512BW
//#include "fastbase64/include/fastavxbase64.h"
  #endif // HAVE_AVX512BW
  #ifdef __ARM_NEON
int neon_base64_decode(char *out, const char *src, size_t srclen);
  #endif

unsigned smin = 16, smask = 0xff;
#define powof2(n) !((n)&((n)-1))

#define _TB64ENC { unsigned char *ip,*op; size_t i = 0,iplen,oplen;\
  unsigned char *ie = in+inlen;\
  for(ip = in, op = out; ip < ie; i++, ip += iplen, op += oplen) {\
	iplen = smin+(i&smask); if(iplen > ie - ip) iplen = ie - ip
#define TB64ENC_\
  }\
  return op - out;\
}


#define _TB64DEC { unsigned char *ip,*op; size_t i = 0,iplen,oplen,_oplen;\
  unsigned char *ie = in+inlen;\
  for(ip = in, op = out; ip < ie; i++, ip += iplen, op += oplen) {\
	oplen = smin+(i&smask); iplen = TB64ENCLEN(oplen); if(iplen > ie-ip) iplen = ie-ip
#define TB64DEC_\
  }\
}

size_t tb64ssenc(    unsigned char *in, size_t inlen, unsigned char *out) { _TB64ENC; oplen =  tb64senc(   ip,iplen,op); TB64ENC_; }
size_t tb64ssdec(    unsigned char *in, size_t inlen, unsigned char *out) { _TB64DEC;          tb64sdec(   ip,iplen,op); TB64DEC_; }

size_t tb64sxenc(    unsigned char *in, size_t inlen, unsigned char *out) { _TB64ENC; oplen =  tb64xenc(   ip,iplen,op); TB64ENC_; }
size_t tb64sxdec(    unsigned char *in, size_t inlen, unsigned char *out) { _TB64DEC;          tb64xdec(   ip,iplen,op); TB64DEC_; }

size_t tb64s128enc(  unsigned char *in, size_t inlen, unsigned char *out) { _TB64ENC; oplen =  tb64v128enc( ip,iplen,op); TB64ENC_; }
size_t tb64s128dec(  unsigned char *in, size_t inlen, unsigned char *out) { _TB64DEC;          tb64v128dec( ip,iplen,op); TB64DEC_; }

size_t tb64s128aenc(  unsigned char *in, size_t inlen, unsigned char *out) { _TB64ENC; oplen =  tb64v128aenc( ip,iplen,op); TB64ENC_; }
size_t tb64s128adec(  unsigned char *in, size_t inlen, unsigned char *out) { _TB64DEC;          tb64v128adec( ip,iplen,op); TB64DEC_; }

size_t tb64s256enc( unsigned char *in, size_t inlen, unsigned char *out) { _TB64ENC; oplen =  tb64v256enc(ip,iplen,op); TB64ENC_; }
size_t tb64s256dec( unsigned char *in, size_t inlen, unsigned char *out) { _TB64DEC;          tb64v256dec(ip,iplen,op); TB64DEC_; }

size_t _tb64s256enc(unsigned char *in, size_t inlen, unsigned char *out) { _TB64ENC; oplen = _tb64v256enc(ip,iplen,op); TB64ENC_; }
size_t _tb64s256dec(unsigned char *in, size_t inlen, unsigned char *out) { _TB64DEC;         _tb64v256dec(ip,iplen,op); TB64DEC_; }

  #ifndef _WIN32
size_t  b64ssseenc(  unsigned char *in, size_t inlen, unsigned char *out) { _TB64ENC; base64_encode((const char*)ip, iplen, (char*)op, &oplen, BASE64_FORCE_SSSE3); TB64ENC_; }
size_t  b64sssedec(  unsigned char *in, size_t inlen, unsigned char *out) { _TB64DEC; base64_decode((const char*)ip, iplen, (char*)op, &_oplen, BASE64_FORCE_SSSE3); TB64DEC_; }

size_t  b64savxenc(  unsigned char *in, size_t inlen, unsigned char *out) { _TB64ENC; base64_encode((const char*)ip, iplen, (char*)op, &oplen, BASE64_FORCE_AVX); TB64ENC_; }
size_t  b64savxdec(  unsigned char *in, size_t inlen, unsigned char *out) { _TB64DEC; base64_decode((const char*)ip, iplen, (char*)op, &_oplen, BASE64_FORCE_AVX); TB64DEC_; }

size_t  b64savx2enc( unsigned char *in, size_t inlen, unsigned char *out) { _TB64ENC; base64_encode((const char*)ip, iplen, (char*)op, &oplen, BASE64_FORCE_AVX2); TB64ENC_; }
size_t  b64savx2dec( unsigned char *in, size_t inlen, unsigned char *out) { _TB64DEC; base64_decode((const char*)ip, iplen, (char*)op, &_oplen, BASE64_FORCE_AVX2); TB64DEC_; }
  #endif
size_t fb64savx2enc( unsigned char *in, size_t inlen, unsigned char *out) { _TB64ENC; oplen = fast_avx2_base64_encode((char*)op, (const char*)ip, iplen); TB64ENC_; }
size_t fb64savx2dec( unsigned char *in, size_t inlen, unsigned char *out) { _TB64DEC;         fast_avx2_base64_decode((char*)op, (const char*)ip, iplen); TB64DEC_; }

size_t check_tb64v256dec(unsigned char *in, size_t inlen, unsigned char *out) { 
  _TB64DEC;
  for(size_t i = 0; i < iplen; i++) {
    unsigned char x = ip[i];  ip[i] = 0;
    if(_tb64v256dec(ip,iplen,op)) { printf("[%d:%d]", (int)iplen, (int)i);fflush(stdout);exit(0); }
    ip[i] = x;
    _tb64v256dec(ip,iplen,op);
    TB64DEC_;
  }
  printf("OK,");fflush(stdout);
}

