#include "ghash.h"
#include <string.h>

// GF(2^128) 乘法函数
static void gf_mul(const uint8_t X[16], const uint8_t Y[16], uint8_t Z[16]) {
  uint8_t V[16], Zt[16] = {0};
  memcpy(V, Y, 16);

  for (int i = 0; i < 16; ++i) {
    for (int j = 7; j >= 0; --j) {
      if ((X[i] >> j) & 1) {
        for (int k = 0; k < 16; ++k) {
          Zt[k] ^= V[k];
        }
      }

      // V = V >> 1 with conditional reduction
      uint8_t lsb = V[15] & 1;
      for (int k = 15; k > 0; --k) {
        V[k] = (V[k] >> 1) | ((V[k - 1] & 1) << 7);
      }
      V[0] >>= 1;

      if (lsb) {
        V[0] ^= 0xE1;
      }
    }
  }

  memcpy(Z, Zt, 16);
}

// 初始化 GHASH
void ghash_init(GHASH_CTX *ctx, const uint8_t H[16]) {
  memcpy(ctx->H, H, 16);
  memset(ctx->Y, 0, 16);
}

// 重置 GHASH 累加器
void ghash_reset(GHASH_CTX *ctx) { memset(ctx->Y, 0, 16); }

// 输入数据（任意长度）
void ghash_update(GHASH_CTX *ctx, const uint8_t *data, size_t len) {
  uint8_t block[16];
  while (len > 0) {
    size_t block_len = len < 16 ? len : 16;
    memset(block, 0, 16);
    memcpy(block, data, block_len);

    // Y_i ← (Y_{i-1} ⊕ X_i) · H
    for (int i = 0; i < 16; ++i) {
      ctx->Y[i] ^= block[i];
    }

    gf_mul(ctx->Y, ctx->H, ctx->Y);

    data += block_len;
    len -= block_len;
  }
}

// 计算最终 GHASH 值
void ghash_final(GHASH_CTX *ctx, uint8_t out[16]) { memcpy(out, ctx->Y, 16); }

// 一次性计算 GHASH
void ghash(const uint8_t H[16], const uint8_t *aad, size_t aad_len,
           const uint8_t *ciphertext, size_t ct_len, uint8_t out[16]) {
  GHASH_CTX ctx;
  ghash_init(&ctx, H);
  ghash_update(&ctx, aad, aad_len);

  // Padding AAD
  if (aad_len % 16 != 0) {
    uint8_t zero_pad[16] = {0};
    ghash_update(&ctx, zero_pad, 16 - (aad_len % 16));
  }

  ghash_update(&ctx, ciphertext, ct_len);

  // Padding CT
  if (ct_len % 16 != 0) {
    uint8_t zero_pad[16] = {0};
    ghash_update(&ctx, zero_pad, 16 - (ct_len % 16));
  }

  // Length block: [len(aad) || len(ct)] in bits
  uint8_t len_block[16];
  uint64_t aad_bits = (uint64_t)aad_len * 8;
  uint64_t ct_bits = (uint64_t)ct_len * 8;
  for (int i = 0; i < 8; i++) {
    len_block[i] = (aad_bits >> (56 - 8 * i)) & 0xFF;
    len_block[i + 8] = (ct_bits >> (56 - 8 * i)) & 0xFF;
  }
  ghash_update(&ctx, len_block, 16);

  ghash_final(&ctx, out);
}
