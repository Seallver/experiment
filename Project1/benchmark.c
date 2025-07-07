#include "sm4.h"
#include "sm4_aesni.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BLOCK_SIZE 16
#define NUM_BLOCKS 10000000 // 可根据需求调整

void print_speed(const char *label, size_t total_bytes, double seconds) {
  double speed = total_bytes / (1024.0 * 1024.0) / seconds;
  printf("%s：%.2f MB/s\n", label, speed);
}

void benchmark_sm4_encrypt(const uint8_t *key, const uint8_t *input) {
  printf("开始 benchmark：SM4 原始实现加密\n");

  uint8_t *output = malloc(NUM_BLOCKS * BLOCK_SIZE);
  SM4_Key sm4_key;
  sm4_keyInit(key, &sm4_key);

  clock_t start = clock();
  for (int i = 0; i < NUM_BLOCKS; i++) {
    sm4_encrypt(input, &sm4_key, output + i * BLOCK_SIZE);
  }
  clock_t end = clock();

  double time_used = (double)(end - start) / CLOCKS_PER_SEC;
  print_speed("SM4 Encrypt", NUM_BLOCKS * BLOCK_SIZE, time_used);
  free(output);
}

void benchmark_sm4_decrypt(const uint8_t *key, const uint8_t *input) {
  printf("开始 benchmark：SM4 原始实现解密\n");

  uint8_t *output = malloc(NUM_BLOCKS * BLOCK_SIZE);
  SM4_Key sm4_key;
  sm4_keyInit(key, &sm4_key);

  clock_t start = clock();
  for (int i = 0; i < NUM_BLOCKS; i++) {
    sm4_decrypt(input, &sm4_key, output + i * BLOCK_SIZE);
  }
  clock_t end = clock();

  double time_used = (double)(end - start) / CLOCKS_PER_SEC;
  print_speed("SM4 Decrypt", NUM_BLOCKS * BLOCK_SIZE, time_used);
  free(output);
}

void benchmark_sm4_encrypt_aesni(const uint8_t *key, const uint8_t *input) {
  printf("开始 benchmark：SM4 AES-NI 优化加密\n");

  uint8_t *output = malloc(NUM_BLOCKS * BLOCK_SIZE);
  SM4_Key sm4_key;
  sm4_keyInit(key, &sm4_key);

  clock_t start = clock();
  for (int i = 0; i < NUM_BLOCKS; i++) {
    sm4_encrypt(input, &sm4_key, output + i * BLOCK_SIZE);
  }
  clock_t end = clock();

  double time_used = (double)(end - start) / CLOCKS_PER_SEC;
  print_speed("SM4 AES-NI Encrypt", NUM_BLOCKS * BLOCK_SIZE, time_used);
  free(output);
}

void benchmark_sm4_decrypt_aesni(const uint8_t *key, const uint8_t *input) {
  printf("开始 benchmark：SM4 AES-NI 优化解密\n");

  uint8_t *output = malloc(NUM_BLOCKS * BLOCK_SIZE);
  SM4_Key sm4_key;
  sm4_keyInit(key, &sm4_key);

  clock_t start = clock();
  for (int i = 0; i < NUM_BLOCKS; i++) {
    sm4_decrypt(input, &sm4_key, output + i * BLOCK_SIZE);
  }
  clock_t end = clock();

  double time_used = (double)(end - start) / CLOCKS_PER_SEC;
  print_speed("SM4 AES-NI Decrypt", NUM_BLOCKS * BLOCK_SIZE, time_used);
  free(output);
}

int main() {
  uint8_t key[16] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
                     0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};

  uint8_t input[16] = {0x00};

  printf("\n数据块数量：%d，单块大小：%d 字节，总共 %.2f MB\n", NUM_BLOCKS,
         BLOCK_SIZE, NUM_BLOCKS * BLOCK_SIZE / (1024.0 * 1024.0));

  benchmark_sm4_encrypt(key, input);
  benchmark_sm4_decrypt(key, input);

  printf("\n");

  benchmark_sm4_encrypt_aesni(key, input);
  benchmark_sm4_decrypt_aesni(key, input);

  return 0;
}
