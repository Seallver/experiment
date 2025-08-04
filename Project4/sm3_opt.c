#include "sm3_opt.h"
#include <immintrin.h>
#include <stdio.h>
#include <string.h>

// 全局变量改为 static，防止外部访问，且减少命名冲突风险
static unsigned char g_message_buffer[64] = {0};
static unsigned int g_hash[8] = {0};
static unsigned int g_T[64] = {0};

static unsigned int rotate_left(unsigned int a, unsigned int k) {
  k %= 32;
  return ((a << k) & 0xFFFFFFFF) | ((a & 0xFFFFFFFF) >> (32 - k));
}

static int init_T() {
  for (int i = 0; i < 16; i++) {
    g_T[i] = 0x79cc4519;
  }
  for (int i = 16; i < 64; i++) {
    g_T[i] = 0x7a879d8a;
  }
  return 1;
}

static unsigned int FF(unsigned int X, unsigned int Y, unsigned int Z,
                       unsigned int j) {
  if (j < 16) {
    return X ^ Y ^ Z;
  } else {
    return (X & Y) | (X & Z) | (Y & Z);
  }
}

static unsigned int GG(unsigned int X, unsigned int Y, unsigned int Z,
                       unsigned int j) {
  if (j < 16) {
    return X ^ Y ^ Z;
  } else {
    return (X & Y) | ((~X) & Z);
  }
}

#define P_0(X) ((X) ^ rotate_left((X), 9) ^ rotate_left((X), 17))
#define P_1(X) ((X) ^ rotate_left((X), 15) ^ rotate_left((X), 23))

static __m128i left(__m128i a, int k) {
  k %= 32;
  __m128i mask = _mm_set1_epi32(0xFFFFFFFF);
  __m128i left_shifted = _mm_and_si128(mask, _mm_slli_epi32(a, k));
  __m128i right_shifted = _mm_srli_epi32(_mm_and_si128(mask, a), 32 - k);
  return _mm_or_si128(left_shifted, right_shifted);
}

int CF(unsigned char *block) {
  unsigned int W[68];
  unsigned int W_1[64];
  unsigned int A, B, C, D, E, F, G, H;
  unsigned int SS1, SS2, TT1, TT2;

  __m128i temp[17];
  __m128i tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp10;
  __m128i mask = _mm_set1_epi32(0xFFFFFFFF);

  for (unsigned int j = 0; j < 16; j++) {
    W[j] = (block[j * 4 + 0] << 24) | (block[j * 4 + 1] << 16) |
           (block[j * 4 + 2] << 8) | block[j * 4 + 3];
  }

  // 消息扩展，SIMD优化
  for (unsigned int j = 4; j < 17; j++) {
    int idx = j * 4 + 3;
    tmp10 = _mm_setr_epi32(W[j * 4 - 16], W[j * 4 - 15], W[j * 4 - 14],
                           W[j * 4 - 13]);
    tmp4 = _mm_setr_epi32(W[j * 4 - 13], W[j * 4 - 12], W[j * 4 - 11],
                          W[j * 4 - 10]);
    tmp5 =
        _mm_setr_epi32(W[j * 4 - 9], W[j * 4 - 8], W[j * 4 - 7], W[j * 4 - 6]);
    tmp6 = _mm_setr_epi32(W[j * 4 - 3], W[j * 4 - 2], W[j * 4 - 1], 0);
    tmp7 =
        _mm_setr_epi32(W[j * 4 - 6], W[j * 4 - 5], W[j * 4 - 4], W[j * 4 - 3]);

    tmp1 = _mm_xor_si128(tmp10, tmp5);
    tmp2 = left(tmp6, 15);
    tmp1 = _mm_xor_si128(tmp1, tmp2);
    tmp3 = _mm_xor_si128(tmp1, _mm_xor_si128(left(tmp1, 15), left(tmp1, 23)));
    tmp8 = _mm_xor_si128(left(tmp4, 7), tmp7);
    temp[j] = _mm_xor_si128(tmp3, tmp8);

    _mm_maskstore_epi32(&W[j * 4], mask, temp[j]);

    W[idx] = P_1(W[idx - 16] ^ W[idx - 9] ^ rotate_left(W[idx - 3], 15)) ^
             rotate_left(W[idx - 13], 7) ^ W[idx - 6];
  }

  for (unsigned int j = 0; j < 64; j++) {
    W_1[j] = W[j] ^ W[j + 4];
  }

  A = g_hash[0];
  B = g_hash[1];
  C = g_hash[2];
  D = g_hash[3];
  E = g_hash[4];
  F = g_hash[5];
  G = g_hash[6];
  H = g_hash[7];

  for (unsigned int j = 0; j < 64; j++) {
    SS1 = rotate_left(
        (rotate_left(A, 12) + E + rotate_left(g_T[j], j)) & 0xFFFFFFFF, 7);
    SS2 = SS1 ^ rotate_left(A, 12);
    TT1 = (FF(A, B, C, j) + D + SS2 + W_1[j]) & 0xFFFFFFFF;
    TT2 = (GG(E, F, G, j) + H + SS1 + W[j]) & 0xFFFFFFFF;

    D = C;
    C = rotate_left(B, 9);
    B = A;
    A = TT1;
    H = G;
    G = rotate_left(F, 19);
    F = E;
    E = P_0(TT2);
  }

  g_hash[0] ^= A;
  g_hash[1] ^= B;
  g_hash[2] ^= C;
  g_hash[3] ^= D;
  g_hash[4] ^= E;
  g_hash[5] ^= F;
  g_hash[6] ^= G;
  g_hash[7] ^= H;

  return 1;
}

void SM3_Init() {
  init_T();
  g_hash[0] = 0x7380166f;
  g_hash[1] = 0x4914b2b9;
  g_hash[2] = 0x172442d7;
  g_hash[3] = 0xda8a0600;
  g_hash[4] = 0xa96f30bc;
  g_hash[5] = 0x163138aa;
  g_hash[6] = 0xe38dee4d;
  g_hash[7] = 0xb0fb0e4e;
}

void Block(unsigned char *msg, unsigned int msglen) {
  unsigned int i = 0;
  unsigned int left = 0;
  unsigned long long total = 0;

  for (i = 0; i < msglen / 64; i++) {
    memcpy(g_message_buffer, msg + i * 64, 64);
    CF(g_message_buffer);
  }

  total = (unsigned long long)msglen * 8;
  left = msglen % 64;

  memset(&g_message_buffer[left], 0, 64 - left);
  memcpy(g_message_buffer, msg + i * 64, left);
  g_message_buffer[left] = 0x80;

  if (left <= 55) {
    for (i = 0; i < 8; i++) {
      g_message_buffer[56 + i] = (total >> ((7 - i) * 8)) & 0xFF;
    }
    CF(g_message_buffer);
  } else {
    CF(g_message_buffer);
    memset(g_message_buffer, 0, 64);
    for (i = 0; i < 8; i++) {
      g_message_buffer[56 + i] = (total >> ((7 - i) * 8)) & 0xFF;
    }
    CF(g_message_buffer);
  }
}

void sm3_OPT(unsigned char *msg, unsigned int msglen, unsigned char *out_hash) {
  SM3_Init();
  Block(msg, msglen);

  // 将内部hash数组转为字节形式输出
  for (int i = 0; i < 8; i++) {
    out_hash[i * 4] = (g_hash[i] >> 24) & 0xFF;
    out_hash[i * 4 + 1] = (g_hash[i] >> 16) & 0xFF;
    out_hash[i * 4 + 2] = (g_hash[i] >> 8) & 0xFF;
    out_hash[i * 4 + 3] = g_hash[i] & 0xFF;
  }
}
