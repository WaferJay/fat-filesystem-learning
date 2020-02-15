#include "charset.h"

#include <stdio.h>
#include <string.h>
#include <iconv.h>

static iconv_t s_conv = NULL;

static void __attribute__ ((constructor)) open_iconv()
{
    char *charset;
    if (WCHAR_SIZE == 2) {
        charset = "utf-16";
    } else if (WCHAR_SIZE == 4) {
        charset = "utf-32";
    } else {
        fprintf(stderr, "error: WCHAR_SIZE == %d\n", WCHAR_SIZE);
        return;
    }

    s_conv = iconv_open(charset, "utf-16le");
}

static void __attribute__ ((destructor)) close_iconv()
{
    iconv_close(s_conv);
}

int utf16le_decode(char *src, size_t srclen, wchar_t *dest, size_t destlen)
{
    if (s_conv <= 0) {
        return -1;
    }

    char *destbs = (char *) dest;
    size_t destsize = destlen * WCHAR_SIZE;
    int err = iconv(s_conv, &src, &srclen, &destbs, &destsize);
    if (err) {
        return -1;
    }

    if (destsize) {
        memset(destbs, 0, destsize < WCHAR_SIZE ? destsize : WCHAR_SIZE);
    }

    return destlen - destsize / WCHAR_SIZE;
}
