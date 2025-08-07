#ifndef GCM_SM4_H
#define GCM_SM4_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../sm4.h"
#include "ghash.h"
#include "ghash_table.h"

typedef struct {
  void (*init)(GHASH_CTX *, const uint8_t H[16]);
  void (*update)(GHASH_CTX *, const uint8_t *data, size_t len);
  void (*final)(GHASH_CTX *, uint8_t out[16]);
  void (*reset)(GHASH_CTX *);
} GHASH_METHOD;

// 结构体定义
typedef struct {
  SM4_Key sm4_key;      // SM4 轮密钥
  uint8_t H[16];        // GHASH key: H = E_K(0^128)
  uint8_t counter[16];  // 当前 GCTR counter
  uint8_t counter0[16]; // 初始计数器 J0
  GHASH_CTX ghash_ctx;
  const GHASH_METHOD *ghash;
  uint64_t aad_len; // AAD字节长度
  uint64_t ct_len;  // 密文长度
} GCM_SM4_CTX;

// GCTR模式加密
void gctr_encrypt(const uint8_t *in, uint8_t *out, size_t len,
                  uint8_t counter[16], const SM4_Key *key);

// 初始化 GCM 上下文
void gcm_sm4_init(GCM_SM4_CTX *ctx, const uint8_t *key, const uint8_t *iv,
                  size_t iv_len, const GHASH_METHOD *ghash_impl);

// 处理 AAD
void gcm_sm4_aad(GCM_SM4_CTX *ctx, const uint8_t *aad, size_t aad_len);

// 加密数据
void gcm_sm4_encrypt(GCM_SM4_CTX *ctx, const uint8_t *plaintext, size_t len,
                     uint8_t *ciphertext);
// 解密数据
void gcm_sm4_decrypt(GCM_SM4_CTX *ctx, const uint8_t *ciphertext, size_t len,
                     uint8_t *plaintext);

// 生成 GMAC 标签
void gcm_sm4_tag(GCM_SM4_CTX *ctx, uint8_t tag[16]);

// GCM 解密实现
int sm4_gcm_decrypt(const uint8_t *key, const uint8_t *iv, size_t iv_len,
                    const uint8_t *aad, size_t aad_len,
                    const uint8_t *ciphertext, size_t cipher_len,
                    const uint8_t *expected_tag, uint8_t *plaintext,
                    const GHASH_METHOD *ghash_impl);

#endif // GCM_SM4_H
