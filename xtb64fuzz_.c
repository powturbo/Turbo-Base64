	#ifdef _BASE64 // ok
      case 21: if(cpuini(0)>=33) { size_t outlen; base64_encode((const char*)in, n, (char*)out, &outlen, BASE64_FORCE_SSSE3); base64_decode((const char*)out, outlen, (char*)cpy, &outlen, BASE64_FORCE_SSSE3); } break;
      case 22: if(cpuini(0)>=41) { size_t outlen; base64_encode((const char*)in, n, (char*)out, &outlen, BASE64_FORCE_SSE41); base64_decode((const char*)out, outlen, (char*)cpy, &outlen, BASE64_FORCE_SSE41); } break; 
      case 23: if(cpuini(0)>=50) { size_t outlen; base64_encode((const char*)in, n, (char*)out, &outlen, BASE64_FORCE_AVX);   base64_decode((const char*)out, outlen, (char*)cpy, &outlen, BASE64_FORCE_AVX); }  break;
      case 24: if(cpuini(0)>=0x60) { size_t outlen; base64_encode((const char*)in, n, (char*)out, &outlen, BASE64_FORCE_AVX2); base64_decode((const char*)out, l, (char*)cpy, &outlen, BASE64_FORCE_AVX2);} break;
	#endif
	#ifdef _FASTBASE
      case 33: if(cpuini(0) >= 60) { l=fast_avx2_base64_encode((char*)out, (const char*)in, n); fast_avx2_base64_decode((char*)cpy,(const char*)out,l); } break;
        #endif
        #ifdef _CRZY
      case 40: l = crzy64_encode(out, in, n); crzy64_decode(cpy, out, l); break; // ok
        #endif
