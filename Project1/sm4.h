#ifndef SM4_H
#define SM4_H

#include "stdint.h"

//轮密钥
typedef struct sm4_key {
  uint32_t rk[32];
} SM4_Key;

//初始化密钥
void sm4_keyInit(const uint8_t *key, SM4_Key *sm4_key);

//加密函数
void sm4_encrypt(const uint8_t *input, const SM4_Key *key, uint8_t *output);

//解密函数
void sm4_decrypt(const uint8_t *input, const SM4_Key *key, uint8_t *output);

#endif