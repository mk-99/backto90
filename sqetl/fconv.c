// Convert utf-8 texts to fido texts and vice versa

#include <iconv.h>

#include "fconv.h"

// maxlen in bytes, returns number of bytes stored to out
size_t fconv(char *in, char *out, int maxlen, int direction)
{
    iconv_t converter_descriptor;
    char *charset_from, *charset_to;
    char *in_ptr, *out_ptr;
    size_t src_left, dst_left, n_conv;

    in_ptr = in;
    out_ptr = out;

    switch (direction) {
        case FCONV_FROM_UTF8:
            charset_from = "UTF8";
            charset_to = "CP866";
            src_left = maxlen;
            dst_left = maxlen;
            break;
        case FCONV_TO_UTF8:
            charset_from = "CP866";
            charset_to = "UTF8";
            src_left = maxlen;
            dst_left = maxlen * 2;
            break;
        default:
            return 0;
    }

    converter_descriptor = iconv_open(charset_to, charset_from);

    if (converter_descriptor == (iconv_t)(-1)) {
        perror("iconv_open");
        return 0;
    }


    n_conv = iconv(converter_descriptor, &in_ptr, &src_left, &out_ptr, &dst_left);

    iconv_close(converter_descriptor);

    // if (n_conv == (size_t)(-1))
    //     return 0;

    if (dst_left >= sizeof(char)) {
        *out_ptr = '\0';
    }

    return out_ptr - out;
}
