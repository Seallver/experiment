#ifndef GHASH_TABLE_H
#define GHASH_TABLE_H

#include "ghash.h"
#include <stddef.h>
#include <stdint.h>

// 初始化 GHASH 上下文
void ghash_table_init(GHASH_CTX *ctx, const uint8_t H[16]);

// 输入数据块（多个任意长度块）
void ghash_table_update(GHASH_CTX *ctx, const uint8_t *data, size_t len);

// 输出 GHASH 值
void ghash_table_final(GHASH_CTX *ctx, uint8_t out[16]);

// 重置 GHASH 上下文
void ghash_table_reset(GHASH_CTX *ctx);

// 一次性完成 GHASH
void ghash_table(const uint8_t H[16], const uint8_t *aad, size_t aad_len,
                 const uint8_t *ciphertext, size_t ct_len, uint8_t out[16]);

#endif // GHASH_TABLE_H
