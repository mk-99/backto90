// Convert utf-8 texts to fido texts and vice versa

#ifndef __FCONV_H__
#define __FCONV_H__

#include <stdlib.h>

enum {
    FCONV_TO_UTF8 = 0,
    FCONV_FROM_UTF8 = 1
};

// Converts in string to out string, zero-terminated, maxlen bytes
size_t fconv(char *in, char *out, int maxlen, int direction);

#endif
