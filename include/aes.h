#pragma once

#include <string.h>
#include "mbedtls/aes.h"
#include "math.h"
#define AES_BLOCK_SIZE 16 // bytes


int aes_decrypt(const unsigned char *input, const size_t input_len, unsigned char *dec_out, const unsigned char *key)
{
    int ret = 0;
    unsigned char in_block[AES_BLOCK_SIZE];
    unsigned char out_block[AES_BLOCK_SIZE];
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_enc(&aes, (const unsigned char *)key, strlen((char *)key) * 8);
    // decrypt each single block
    for (int i = 0; i < input_len / AES_BLOCK_SIZE; i++)
    {
        strncpy((char *)in_block, (char *)(input + i * AES_BLOCK_SIZE), AES_BLOCK_SIZE);
        ret += mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_DECRYPT, in_block, out_block);
        strncpy((char *)(dec_out + i * AES_BLOCK_SIZE), (char *)out_block, AES_BLOCK_SIZE);
    }
    // un-pad message
    mbedtls_aes_free(&aes);
    return ret;
}


