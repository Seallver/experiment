// sm3.c
#include "sm3.h"
#include <string.h>

const uint32_t SM3_IV[SM3_HASH_WORDS] = {0x7380166f, 0x4914b2b9, 0x172442d7,
                                         0xda8a0600, 0xa96f30bc, 0x163138aa,
                                         0xe38dee4d, 0xb0fb0e4e};

static uint32_t Tj(uint8_t j) { return (j < 16) ? 0x79cc4519 : 0x7a879d8a; }

static uint32_t FF(uint32_t X, uint32_t Y, uint32_t Z, uint8_t j) {
  return (j < 16) ? (X ^ Y ^ Z) : ((X & Y) | (X & Z) | (Y & Z));
}

static uint32_t GG(uint32_t X, uint32_t Y, uint32_t Z, uint8_t j) {
  return (j < 16) ? (X ^ Y ^ Z) : ((X & Y) | ((~X) & Z));
}

static uint32_t RL(uint32_t a, uint8_t k) {
  k %= 32;
  return ((a << k) & 0xFFFFFFFF) | ((a & 0xFFFFFFFF) >> (32 - k));
}

static uint32_t P0(uint32_t X) { return X ^ RL(X, 9) ^ RL(X, 17); }

static uint32_t P1(uint32_t X) { return X ^ RL(X, 15) ^ RL(X, 23); }

void sm3_init(SM3_CTX *ctx) {
  memcpy(ctx->hash, SM3_IV, sizeof(SM3_IV));
  ctx->total_len = 0;
  ctx->buffer_len = 0;
}

void sm3_compress(uint32_t state[SM3_HASH_WORDS],
                  const uint8_t block[SM3_BLOCK_SIZE]) {
  uint32_t Wj0[68];
  uint32_t Wj1[64];
  uint32_t A = state[0], B = state[1], C = state[2], D = state[3];
  uint32_t E = state[4], F = state[5], G = state[6], H = state[7];
  uint32_t SS1, SS2, TT1, TT2;

  // Message expansion
  for (int i = 0; i < 16; i++) {
    Wj0[i] = ((uint32_t)block[i * 4] << 24) |
             ((uint32_t)block[i * 4 + 1] << 16) |
             ((uint32_t)block[i * 4 + 2] << 8) | block[i * 4 + 3];
  }

  for (int i = 16; i < 68; i++) {
    Wj0[i] = P1(Wj0[i - 16] ^ Wj0[i - 9] ^ RL(Wj0[i - 3], 15)) ^
             RL(Wj0[i - 13], 7) ^ Wj0[i - 6];
  }

  for (int i = 0; i < 64; i++) {
    Wj1[i] = Wj0[i] ^ Wj0[i + 4];
  }

  // Compression function
  for (int j = 0; j < 64; j++) {
    SS1 = RL((RL(A, 12) + E + RL(Tj(j), j)) & 0xFFFFFFFF, 7);
    SS2 = SS1 ^ RL(A, 12);
    TT1 = (FF(A, B, C, j) + D + SS2 + Wj1[j]) & 0xFFFFFFFF;
    TT2 = (GG(E, F, G, j) + H + SS1 + Wj0[j]) & 0xFFFFFFFF;

    D = C;
    C = RL(B, 9);
    B = A;
    A = TT1;
    H = G;
    G = RL(F, 19);
    F = E;
    E = P0(TT2);
  }

  // Update state
  state[0] ^= A;
  state[1] ^= B;
  state[2] ^= C;
  state[3] ^= D;
  state[4] ^= E;
  state[5] ^= F;
  state[6] ^= G;
  state[7] ^= H;
}

void sm3_update(SM3_CTX *ctx, const uint8_t *data, uint32_t len) {
  uint32_t remaining = SM3_BLOCK_SIZE - ctx->buffer_len;

  ctx->total_len += len;

  if (len >= remaining) {
    memcpy(ctx->buffer + ctx->buffer_len, data, remaining);
    sm3_compress(ctx->hash, ctx->buffer);
    ctx->buffer_len = 0;
    data += remaining;
    len -= remaining;

    while (len >= SM3_BLOCK_SIZE) {
      sm3_compress(ctx->hash, data);
      data += SM3_BLOCK_SIZE;
      len -= SM3_BLOCK_SIZE;
    }
  }

  if (len > 0) {
    memcpy(ctx->buffer + ctx->buffer_len, data, len);
    ctx->buffer_len += len;
  }
}

void sm3_final(SM3_CTX *ctx, uint8_t digest[SM3_HASH_SIZE]) {
  uint64_t bit_len = ctx->total_len * 8;
  uint8_t pad[SM3_BLOCK_SIZE] = {0};

  // Add padding
  pad[0] = 0x80;
  uint32_t pad_len =
      (ctx->buffer_len < 56) ? (56 - ctx->buffer_len) : (120 - ctx->buffer_len);

  sm3_update(ctx, pad, pad_len);

  // Add length
  uint8_t len_bytes[8];
  for (int i = 0; i < 8; i++) {
    len_bytes[i] = (bit_len >> (56 - i * 8)) & 0xFF;
  }
  sm3_update(ctx, len_bytes, 8);

  // Output hash
  for (int i = 0; i < SM3_HASH_WORDS; i++) {
    digest[i * 4] = (ctx->hash[i] >> 24) & 0xFF;
    digest[i * 4 + 1] = (ctx->hash[i] >> 16) & 0xFF;
    digest[i * 4 + 2] = (ctx->hash[i] >> 8) & 0xFF;
    digest[i * 4 + 3] = ctx->hash[i] & 0xFF;
  }
}