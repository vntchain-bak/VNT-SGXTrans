
#include "../App.h"
#include "Enclave_u.h"
#include <stdio.h>

#define TOLOWER(x) ((x) | 0x20)
#define isxdigit(c) (('0' <= (c) && (c) <= '9') || ('a' <= (c) && (c) <= 'f') || ('A' <= (c) && (c) <= 'F'))
#define isdigit(c) ('0' <= (c) && (c) <= '9')

//bin2str(signret,out,strlen(signret),16); 转换为16进制
//last char of in must be 0x00
void bin2str(unsigned char *in, char *out, int size, int base)
{
    unsigned char *pt1 = in;
    char *pt2 = out;
    do
    {
        pt2 += sprintf(pt2, "%02X", *pt1++);
        size--;
    } while (*pt1 && size);
}

void str2bin(char *in, unsigned char *out, int size, int base)
{
    unsigned char *pt1 = (unsigned char *)in;
    unsigned char *pt2 = out;
    while (isxdigit(*pt1) && size--)
    {
        *pt2++ = base * (isdigit(*pt1) ? *pt1++ - '0' : TOLOWER(*pt1++) - 'a' + 10) + (isdigit(*pt1) ? *pt1++ - '0' : TOLOWER(*pt1++) - 'a' + 10);
    }
}