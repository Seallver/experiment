#ifndef GHASH_H
#define GHASH_H

#include <stddef.h>
#include <stdint.h>

//为查找表做准备
typedef uint64_t HTable[256][2];

// GHASH 上下文
typedef struct {
  uint8_t H[16]; // 密钥 H（E_K(0^128)）
  uint8_t Y[16]; // 累加器
  HTable table;  // 查表
} GHASH_CTX;

// 初始化 GHASH 上下文
void ghash_init(GHASH_CTX *ctx, const uint8_t H[16]);

// 输入数据块（多个任意长度块）
void ghash_update(GHASH_CTX *ctx, const uint8_t *data, size_t len);

// 输出 GHASH 值
void ghash_final(GHASH_CTX *ctx, uint8_t out[16]);

// 重置 GHASH 上下文（Y ← 0）
void ghash_reset(GHASH_CTX *ctx);

// 一次性完成 GHASH（常用于认证标签校验）
void ghash(const uint8_t H[16], const uint8_t *aad, size_t aad_len,
           const uint8_t *ciphertext, size_t ct_len, uint8_t out[16]);

#endif // GHASH_H
