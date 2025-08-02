// main.c

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "sm3.h"     // 原版实现
#include "sm3_opt.h" // 优化实现

void print_hash(const uint8_t digest[SM3_HASH_SIZE]) {
  for (int i = 0; i < SM3_HASH_SIZE; i++) {
    printf("%02x", digest[i]);
    if ((i + 1) % 4 == 0)
      printf(" ");
  }
  printf("\n");
}

int compare_hashes(const uint8_t *a, const uint8_t *b) {
  return memcmp(a, b, SM3_HASH_SIZE) == 0;
}

void test_case(const char *label, const uint8_t *msg, size_t msglen) {
  uint8_t digest_std[SM3_HASH_SIZE];
  uint8_t digest_opt[SM3_HASH_SIZE];

  // 标准实现
  SM3_CTX ctx;
  sm3_init(&ctx);
  sm3_update(&ctx, msg, msglen);
  sm3_final(&ctx, digest_std);

  // 优化实现
  sm3_opt((unsigned char *)msg, (unsigned int)msglen, digest_opt);

  // 输出结果
  printf("%s\n", label);
  printf("Standard: ");
  print_hash(digest_std);
  printf("Optimized: ");
  print_hash(digest_opt);

  // 比较输出
  if (compare_hashes(digest_std, digest_opt)) {
    printf("Output matches.\n\n");
  } else {
    printf("Output differs!\n\n");
  }
}

int main() {
  // 用例 1: "abc"
  uint8_t msg1[] = {'a', 'b', 'c'};
  test_case("Test Case 1: \"abc\"", msg1, sizeof(msg1));

  // 用例 2: 64 字节 'abcdabcd...'
  uint8_t msg2[64];
  for (int i = 0; i < 64; i += 4) {
    msg2[i] = 'a';
    msg2[i + 1] = 'b';
    msg2[i + 2] = 'c';
    msg2[i + 3] = 'd';
  }
  test_case("Test Case 2: 64-byte repeating \"abcd\"", msg2, sizeof(msg2));

  return 0;
}