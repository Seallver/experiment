#ifndef SM4_OPT_H
#define SM4_OPT_H

#include "sm4.h"
#include <stdint.h>

void _SM4_do(const uint8_t *in, uint8_t *out, const SM4_Key *sm4_key,
             uint8_t enc);

void sm4_encrypt_ttable(const uint8_t *in, const SM4_Key *key, uint8_t *out);
void sm4_decrypt_ttable(const uint8_t *in, const SM4_Key *key, uint8_t *out);

#endif