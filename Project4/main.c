// main.c
#include "sm3.h"
#include <stdio.h>

void print_hash(const uint8_t digest[SM3_HASH_SIZE]) {
  for (int i = 0; i < SM3_HASH_SIZE; i++) {
    printf("%02x", digest[i]);
    if ((i + 1) % 4 == 0)
      printf(" ");
  }
  printf("\n");
}

void test_case1() {
  SM3_CTX ctx;
  uint8_t digest[SM3_HASH_SIZE];
  uint8_t msg[] = {'a', 'b', 'c'};

  sm3_init(&ctx);
  sm3_update(&ctx, msg, 3);
  sm3_final(&ctx, digest);

  printf("Test case 1 (abc): ");
  print_hash(digest);
}

void test_case2() {
  SM3_CTX ctx;
  uint8_t digest[SM3_HASH_SIZE];
  uint8_t msg[64];

  for (int i = 0; i < 64; i += 4) {
    msg[i] = 'a';
    msg[i + 1] = 'b';
    msg[i + 2] = 'c';
    msg[i + 3] = 'd';
  }

  sm3_init(&ctx);
  sm3_update(&ctx, msg, 64);
  sm3_final(&ctx, digest);

  printf("Test case 2 (64-byte input: abcd): ");
  print_hash(digest);
}

int main() {
  test_case1();
  test_case2();
  return 0;
}