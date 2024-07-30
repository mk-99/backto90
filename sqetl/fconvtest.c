#include <stdio.h>
#include <string.h>

#include "fconv.h"

int main(int argc, char *argv[])
{
    char *s_uni = "Тестовая строка";
    char s_out[5000];
    char s_check[5000];


    fconv(s_uni, s_out, strlen(s_uni), FCONV_FROM_UTF8);
    fconv(s_out, s_check, strlen(s_out), FCONV_TO_UTF8);

    printf("SRC: %s\n", s_uni);
    printf("CP866: %s\n", s_out);
    printf("UtF8: %s\n", s_check);

    return 0;
}