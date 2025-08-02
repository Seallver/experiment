#include "sm4.h"
#include "sm4_aesni.h"
#include "sm4_opt.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BLOCK_SIZE 16       // SM4 每个块是 128 位（16 字节）
#define NUM_BLOCKS 10000000 // 可根据需要调整数据量

typedef void (*EncryptFunc)(const uint8_t[16], const SM4_Key *, uint8_t[16]);
typedef void (*DecryptFunc)(const uint8_t[16], const SM4_Key *, uint8_t[16]);

void print_speed(const char *label, size_t total_bytes, double seconds) {
  double speed = total_bytes / (1024.0 * 1024.0) / seconds;
  printf("%s：%.2f MB/s\n", label, speed);
}

void benchmark(const char *label_enc, const char *label_dec,
               EncryptFunc encrypt, DecryptFunc decrypt, const uint8_t *key,
               const uint8_t *input) {
  uint8_t *output = malloc(NUM_BLOCKS * BLOCK_SIZE);
  if (!output) {
    fprintf(stderr, "内存分配失败\n");
    exit(1);
  }

  SM4_Key sm4_key;
  sm4_keyInit(key, &sm4_key);

  // 加密 benchmark
  printf("开始 benchmark：%s\n", label_enc);
  clock_t start_enc = clock();
  for (int i = 0; i < NUM_BLOCKS; i++) {
    encrypt(input, &sm4_key, output + i * BLOCK_SIZE);
  }
  clock_t end_enc = clock();
  double time_enc = (double)(end_enc - start_enc) / CLOCKS_PER_SEC;
  print_speed(label_enc, NUM_BLOCKS * BLOCK_SIZE, time_enc);

  // 解密 benchmark
  printf("开始 benchmark：%s\n", label_dec);
  clock_t start_dec = clock();
  for (int i = 0; i < NUM_BLOCKS; i++) {
    decrypt(output + i * BLOCK_SIZE, &sm4_key, output + i * BLOCK_SIZE);
  }
  clock_t end_dec = clock();
  double time_dec = (double)(end_dec - start_dec) / CLOCKS_PER_SEC;
  print_speed(label_dec, NUM_BLOCKS * BLOCK_SIZE, time_dec);

  free(output);
}

int main() {
  uint8_t key[16] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
                     0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};

  uint8_t input[16] = {0x00}; // 测试输入，可以设为全零或随机

  printf("\n数据块数量：%d，单块大小：%d 字节，总共 %.2f MB\n\n", NUM_BLOCKS,
         BLOCK_SIZE, NUM_BLOCKS * BLOCK_SIZE / (1024.0 * 1024.0));

  benchmark("SM4 原始加密", "SM4 原始解密", sm4_encrypt, sm4_decrypt, key,
            input);

  printf("\n");

  benchmark("SM4 AES-NI 加密", "SM4 AES-NI 解密", sm4_encrypt_aesni,
            sm4_decrypt_aesni, key, input);

  printf("\n");

  benchmark("SM4 T-table 加密", "SM4 T-table 解密", sm4_encrypt_ttable,
            sm4_decrypt_ttable, key, input);

  return 0;
}
