#ifndef SM4_AESNI_H
#define SM4_AESNI_H

#include "sm4.h"

void SM4_AESNI_Encrypt(uint8_t *plaintext, SM4_Key *sm4_key,
                       uint8_t *ciphertext);

void SM4_AESNI_Decrypt(uint8_t *ciphertext, SM4_Key *sm4_key,
                       uint8_t *plaintext);

#endif