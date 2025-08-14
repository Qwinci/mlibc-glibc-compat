#include "utils.hpp"
#include <stddef.h>
#include <string.h>

using iconv_t = void *;

EXPORT iconv_t iconv_open(const char *tocode, const char *fromcode) {
       return reinterpret_cast<iconv_t>(1);
}

EXPORT size_t iconv(iconv_t cd, char **__restrict inbuf, size_t *__restrict inbytesleft, char **__restrict outbuf, size_t *__restrict outbytesleft) {
       (void)inbytesleft;
       (void)outbytesleft;

       if(cd == (iconv_t)1) { // UTF-8 to UTF-8
               memcpy(inbuf, outbuf, *inbytesleft);
               *outbytesleft = *inbytesleft;
               return *outbytesleft;
       }
       __ensure(!"iconv() not implemented");
       __builtin_unreachable();
}

EXPORT int iconv_close(iconv_t) {
       return 0;
}

