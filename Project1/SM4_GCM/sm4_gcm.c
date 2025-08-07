#include "sm4_gcm.h"

// 大端存储64位整数
static void store64_be(uint8_t out[8], uint64_t val) {
  for (int i = 7; i >= 0; i--) {
    out[i] = val & 0xFF;
    val >>= 8;
  }
}

// GCTR增加计数器（最后4字节）
static void inc32(uint8_t counter[16]) {
  for (int i = 15; i >= 12; i--) {
    if (++counter[i])
      break;
  }
}

void gctr_encrypt(const uint8_t *in, uint8_t *out, size_t len,
                  uint8_t counter[16], const SM4_Key *key) {
  uint8_t stream[16], tmp[16];
  size_t i;

  while (len > 0) {
    sm4_encrypt(counter, key, stream);
    size_t block = len < 16 ? len : 16;
    for (i = 0; i < block; i++) {
      out[i] = in[i] ^ stream[i];
    }
    out += block;
    in += block;
    len -= block;
    inc32(counter);
  }
}

void gcm_sm4_init(GCM_SM4_CTX *ctx, const uint8_t *key, const uint8_t *iv,
                  size_t iv_len, const GHASH_METHOD *ghash_impl) {

  static const uint8_t ZERO[16] = {0};

  ctx->ghash = ghash_impl;

  sm4_keyInit(key, &ctx->sm4_key);
  sm4_encrypt(ZERO, &ctx->sm4_key, ctx->H);
  ctx->ghash->init(&ctx->ghash_ctx, ctx->H);

  if (iv_len == 12) {
    memcpy(ctx->counter0, iv, 12);
    ctx->counter0[12] = 0;
    ctx->counter0[13] = 0;
    ctx->counter0[14] = 0;
    ctx->counter0[15] = 1;
  } else {
    ctx->ghash->update(&ctx->ghash_ctx, iv, iv_len);
    size_t pad = (16 - (iv_len % 16)) % 16;
    uint8_t zero_pad[16] = {0};
    ctx->ghash->update(&ctx->ghash_ctx, zero_pad, pad);

    uint8_t len_block[16];
    store64_be(len_block, 0);
    store64_be(len_block + 8, iv_len * 8);
    ctx->ghash->update(&ctx->ghash_ctx, len_block, 16);
    ctx->ghash->final(&ctx->ghash_ctx, ctx->counter0);
    ctx->ghash->reset(&ctx->ghash_ctx);
  }

  memcpy(ctx->counter, ctx->counter0, 16);
  ctx->aad_len = 0;
  ctx->ct_len = 0;
}

void gcm_sm4_aad(GCM_SM4_CTX *ctx, const uint8_t *aad, size_t aad_len) {
  ctx->ghash->update(&ctx->ghash_ctx, aad, aad_len);
  size_t pad = (16 - (aad_len % 16)) % 16;
  uint8_t zero_pad[16] = {0};
  ctx->ghash->update(&ctx->ghash_ctx, zero_pad, pad);
  ctx->aad_len = aad_len;
}

void gcm_sm4_encrypt(GCM_SM4_CTX *ctx, const uint8_t *plaintext, size_t len,
                     uint8_t *ciphertext) {
  gctr_encrypt(plaintext, ciphertext, len, ctx->counter, &ctx->sm4_key);
  ctx->ghash->update(&ctx->ghash_ctx, ciphertext, len);
  size_t pad = (16 - (len % 16)) % 16;
  uint8_t zero_pad[16] = {0};
  ctx->ghash->update(&ctx->ghash_ctx, zero_pad, pad);
  ctx->ct_len = len;
}

void gcm_sm4_decrypt(GCM_SM4_CTX *ctx, const uint8_t *ciphertext, size_t len,
                     uint8_t *plaintext) {
  ctx->ghash->update(&ctx->ghash_ctx, ciphertext, len);
  size_t pad = (16 - (len % 16)) % 16;
  uint8_t zero_pad[16] = {0};
  ctx->ghash->update(&ctx->ghash_ctx, zero_pad, pad);
  ctx->ct_len = len;
  gctr_encrypt(ciphertext, plaintext, len, ctx->counter, &ctx->sm4_key);
}

void gcm_sm4_tag(GCM_SM4_CTX *ctx, uint8_t tag[16]) {
  uint8_t len_block[16];
  store64_be(len_block, ctx->aad_len * 8);
  store64_be(len_block + 8, ctx->ct_len * 8);
  ctx->ghash->update(&ctx->ghash_ctx, len_block, 16);

  uint8_t S[16];
  ctx->ghash->final(&ctx->ghash_ctx, S);

  sm4_encrypt(ctx->counter0, &ctx->sm4_key, tag);
  for (int i = 0; i < 16; i++) {
    tag[i] ^= S[i];
  }
}

int sm4_gcm_decrypt(const uint8_t *key, const uint8_t *iv, size_t iv_len,
                    const uint8_t *aad, size_t aad_len,
                    const uint8_t *ciphertext, size_t cipher_len,
                    const uint8_t *expected_tag, uint8_t *plaintext,
                    const GHASH_METHOD *ghash_impl) {
  GCM_SM4_CTX ctx;
  uint8_t computed_tag[16];

  // 初始化上下文
  gcm_sm4_init(&ctx, key, iv, iv_len, ghash_impl);

  // AAD 认证数据处理
  if (aad_len > 0 && aad != NULL) {
    gcm_sm4_aad(&ctx, aad, aad_len);
  }

  // 解密密文
  gcm_sm4_decrypt(&ctx, ciphertext, cipher_len, plaintext);

  // 生成并验证标签
  gcm_sm4_tag(&ctx, computed_tag);

  // 比较认证标签
  if (memcmp(computed_tag, expected_tag, 16) != 0) {
    // 如果标签不匹配，清除明文并返回失败
    memset(plaintext, 0, cipher_len);
    return -1;
  }

  return 0; // 解密和认证成功
}