#ifndef __FAT_CHARSET_H_INCLUDED__
#define __FAT_CHARSET_H_INCLUDED__

#include <wchar.h>

#define WCHAR_SIZE sizeof(wchar_t)

int utf16le_decode(char *src, size_t srclen, wchar_t *dest, size_t destlen);

#endif
