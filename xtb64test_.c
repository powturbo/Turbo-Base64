    case 11:                      TMBENCH("",l=tb64ssenc(   in, n, out),n);  pr(l,n); TMBENCH2("20:tb64ss",    tb64ssdec(   out, l, cpy), l);   break;
    case 12:                      TMBENCH("",l=tb64sxenc(   in, n, out),n);  pr(l,n); TMBENCH2("21:tb64sx",    tb64sxdec(   out, l, cpy), l);   break;
    case 13:if(cpuini(0)>=0x33) { TMBENCH("",l=tb64s128enc( in, n, out),n);  pr(l,n); TMBENCH2("22:tb64s128",  tb64s128dec( out, l, cpy), l); } break;
      #if defined(__i386__) || defined(__x86_64__)
    case 14:if(cpuini(0)>=0x50) { TMBENCH("",l=tb64s128aenc( in, n, out),n); pr(l,n); TMBENCH2("23:tb64s128a", tb64s128adec( out, l, cpy), l); } break;
    case 15:if(cpuini(0)>=0x60) { TMBENCH("",l=tb64s256enc(in, n, out),n);   pr(l,n); TMBENCH2("24:tb64s256",  tb64s256dec(out, l, cpy), l); } break;
    case 16:if(cpuini(0)>=0x60) { TMBENCH("",l=_tb64s256enc(in, n, out),n);  pr(l,n); TMBENCH2("25:_tb64s256",_tb64s256dec(out, l, cpy), l); } break;
    case 17:if(cpuini(0)>=0x60) { TMBENCH("",l=_tb64v256enc(in, n, out),n);pr(l,n); TMBENCH2("check_tb64v256",check_tb64v256dec(out, l, cpy), l); } break;
    
        #ifndef _WIN32
    case 20:if(cpuini(0)>=0x33) { TMBENCH("",l=b64ssseenc( in, n, out),n);  pr(l,n); TMBENCH2("26:b64ssse",   b64sssedec( out, l, cpy), l); } break;
    case 21:if(cpuini(0)>=0x50) { TMBENCH("",l=b64savxenc( in, n, out),n);  pr(l,n); TMBENCH2("27:b64savx",   b64savxdec( out, l, cpy), l); } break;
    case 22:if(cpuini(0)>=0x60) { TMBENCH("",l=b64savx2enc(in, n, out),n);  pr(l,n); TMBENCH2("28:b64savx2",  b64savx2dec(out, l, cpy), l); } break;
	#endif
      #endif
    case 23:if(cpuini(0)>=0x60) { TMBENCH("",l=fb64savx2enc(in, n, out),n); pr(l,n); TMBENCH2("29:fb64savx2", fb64savx2dec(out, l, cpy), l); } break;
      #ifndef _WIN32
    case 30: { size_t outlen=0; TMBENCH("",base64_encode((char*)in, n, (char*)out, &outlen, BASE64_FORCE_PLAIN),n); l=outlen; pr(l,n); TMBENCH2("b64plain", base64_decode((char*)out, l, (char*)cpy, &outlen, BASE64_FORCE_PLAIN), l);  break; }
      #endif
      #if defined(__i386__) || defined(__x86_64__)
	#ifndef _WIN32
    case 31: if(cpuini(0)>=33) { size_t outlen; TMBENCH("",base64_encode((const char*)in, n, (char*)out, &outlen, BASE64_FORCE_SSSE3),n); l=outlen; pr(l,n); TMBENCH2("b64ssse3", base64_decode((const char*)out, l, (char*)cpy, &outlen, BASE64_FORCE_SSSE3), l);  break; }
    case 32: if(cpuini(0)>=41) { size_t outlen; TMBENCH("",base64_encode((const char*)in, n, (char*)out, &outlen, BASE64_FORCE_SSE41),n); l=outlen; pr(l,n); TMBENCH2("b64sse41", base64_decode((const char*)out, l, (char*)cpy, &outlen, BASE64_FORCE_SSE41), l);  break; }
    case 33: if(cpuini(0)>=50) { size_t outlen; TMBENCH("",base64_encode((const char*)in, n, (char*)out, &outlen, BASE64_FORCE_AVX),  n); l=outlen; pr(l,n); TMBENCH2("b64avx",   base64_decode((const char*)out, l, (char*)cpy, &outlen, BASE64_FORCE_AVX),   l);  break; }
    case 34: if(cpuini(0)>=60) { size_t outlen; TMBENCH("",base64_encode((const char*)in, n, (char*)out, &outlen, BASE64_FORCE_AVX2),n);  l=outlen; pr(l,n); TMBENCH2("b64avx2",  base64_decode((const char*)out, l, (char*)cpy, &outlen, BASE64_FORCE_AVX2),  l);  break; }

    case 35: {                TMBENCH("",l=linux_base64_encode((char*)out, (const char*)in, (const char*)(in+n)),n);  pr(l,n); TMBENCH2("linux",     linux_base64_decode((char*)cpy,(const char*)out,(const char*)in+l),  l);  break; }
	#endif
    case 36: {                TMBENCH("",l=chromium_base64_encode((char*)out, (const char*)in, n),n);                 pr(l,n); TMBENCH2("chrome",    chromium_base64_decode((char*)cpy,(const char*)out,l),  l);  break; }
    case 37: { size_t outlen; TMBENCH("",scalar_base64_encode((const char*)in, n, (char*)out,&outlen),n); l = outlen; pr(l,n); TMBENCH2("fb64plain", scalar_base64_decode((char*)out,l,(char*)cpy,&outlen),  l);  break; }
    case 38: if(cpuini(0) >= 60) { TMBENCH("",l=fast_avx2_base64_encode((char*)out, (const char*)in, n),n); pr(l,n); TMBENCH2("fb64avx2",  fast_avx2_base64_decode((char*)cpy,(const char*)out,l),  l);  break; }
    case 39: {                TMBENCH("",l=crzy64_encode(out, in, n),n);  pr(l,n); TMBENCH2("crzy",     crzy64_decode(cpy, out, l),  l);  break; }
      #elif defined(__ARM_NEON)
   //case  8: { size_t outlen=0; TMBENCH("",base64_encode((const char*)in, n, (char*)out, &outlen, BASE64_FORCE_NEON32),n); l=outlen; pr(l,n); TMBENCH2("b64neon64", base64_decode((const char*)out, l, (char*)cpy, &outlen, BASE64_FORCE_NEON32), l);  break; }
    case 40: { size_t outlen=0; TMBENCH("",base64_encode((const char*)in, n, (char*)out, &outlen, BASE64_FORCE_NEON64),n); l=outlen; pr(l,n); TMBENCH2("b64neon64", base64_decode((const char*)out, l, (char*)cpy, &outlen, BASE64_FORCE_NEON64), l);  break; }
    case 41: {                  TMBENCH("",l=chromium_base64_encode((char*)out, (const char*)in, n),n);   pr(l,n); TMBENCH2("fb64neon", neon_base64_decode((char*)cpy, (const char*)out,l), l);  break; }
      #endif


