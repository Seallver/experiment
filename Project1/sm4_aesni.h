#ifndef SM4_AESNI_H
#define SM4_AESNI_H

#include "sm4.h"

void sm4_encrypt_aesni(const uint8_t *plaintext, const SM4_Key *sm4_key,
                       uint8_t *ciphertext);

void sm4_decrypt_aesni(const uint8_t *ciphertext, const SM4_Key *sm4_key,
                       uint8_t *plaintext);

void SM4_AESNI_do(const uint8_t *in, uint8_t *out, const SM4_Key *sm4_key,
                  int enc);

#endif