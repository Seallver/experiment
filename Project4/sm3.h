// sm3.h
#ifndef SM3_H
#define SM3_H

#include <stdint.h>

#define SM3_BLOCK_SIZE 64
#define SM3_HASH_SIZE 32
#define SM3_HASH_WORDS 8

typedef struct {
  uint32_t hash[SM3_HASH_WORDS];
  uint64_t total_len;
  uint8_t buffer[SM3_BLOCK_SIZE];
  uint32_t buffer_len;
} SM3_CTX;

extern const uint32_t SM3_IV[SM3_HASH_WORDS];

void sm3_init(SM3_CTX *ctx);
void sm3_update(SM3_CTX *ctx, const uint8_t *data, uint32_t len);
void sm3_final(SM3_CTX *ctx, uint8_t digest[SM3_HASH_SIZE]);
void sm3_compress(uint32_t state[SM3_HASH_WORDS],
                  const uint8_t block[SM3_BLOCK_SIZE]);

#endif // SM3_H