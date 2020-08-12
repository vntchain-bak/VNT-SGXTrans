#include "sgx_trts.h"
#include "../Enclave.h"
#include "Enclave_t.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <openssl/aes.h>

int aes_encrypt(char *in, char *key, char *out)
{
    if (!in || !key || !out)
    {
        return 0;
    }

    AES_KEY aes;
    if (AES_set_encrypt_key((unsigned char *)key, 256, &aes) < 0)
    {
        return 0;
    }

    int len = 32768, en_len = 0;

    while (en_len < len)
    {
        AES_encrypt((unsigned char *)in, (unsigned char *)out, &aes);
        in += AES_BLOCK_SIZE;
        out += AES_BLOCK_SIZE;
        en_len += AES_BLOCK_SIZE;
    }

    return 1;
}

int aes_decrypt(char *in, char *key, char *out)
{
    if (!in || !key || !out)
    {
        return 0;
    }

    AES_KEY aes;
    if (AES_set_decrypt_key((unsigned char *)key, 256, &aes) < 0)
    {
        return 0;
    }

    int len = 32768, en_len = 0;
    while (en_len < len)
    {
        AES_decrypt((unsigned char *)in, (unsigned char *)out, &aes);
        in += AES_BLOCK_SIZE;
        out += AES_BLOCK_SIZE;
        en_len += AES_BLOCK_SIZE;
    }

    return 1;
}
