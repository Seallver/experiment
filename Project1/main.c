#include "sm4.h"
#include "sm4_aesni.h"
#include "sm4_ttable.h"
#include <stdio.h>
#include <string.h>

typedef void (*EncryptFunc)(const uint8_t[16], const SM4_Key *, uint8_t[16]);
typedef void (*DecryptFunc)(const uint8_t[16], const SM4_Key *, uint8_t[16]);

void print_hex(const uint8_t *data, size_t len) {
  for (size_t i = 0; i < len; i++) {
    printf("%02x", data[i]);
  }
  printf("\n");
}

void run_test(const char *title, EncryptFunc encrypt, DecryptFunc decrypt,
              const uint8_t *key, const uint8_t *plaintext,
              const uint8_t *expected_ciphertext) {
  printf("\n%s\n", title);

  uint8_t ciphertext[16 * 8];
  uint8_t decrypted_text[16 * 8];

  SM4_Key sm4_key;
  sm4_keyInit(key, &sm4_key);

  encrypt(plaintext, &sm4_key, ciphertext);
  printf("加密结果：\t\t");
  print_hex(ciphertext, 16);

  printf("是否等于标准输出：\t%s\n",
         memcmp(ciphertext, expected_ciphertext, 16) == 0 ? "true" : "false");

  decrypt(ciphertext, &sm4_key, decrypted_text);
  printf("解密结果：\t\t");
  print_hex(decrypted_text, 16);

  printf("是否等于原文：\t\t%s\n",
         memcmp(decrypted_text, plaintext, 16) == 0 ? "true" : "false");
}

int main() {
  // 测试向量（来自 SM4 标准）
  uint8_t plaintext[16] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
                           0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};

  uint8_t key[16] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
                     0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};

  uint8_t expected_ciphertext[16] = {0x68, 0x1E, 0xDF, 0x34, 0xD2, 0x06,
                                     0x96, 0x5E, 0x86, 0xB3, 0xE9, 0x4F,
                                     0x53, 0x6E, 0x42, 0x46};

  run_test("SM4 原始实现标准示例测试", sm4_encrypt, sm4_decrypt, key, plaintext,
           expected_ciphertext);

  run_test("SM4 AES-NI x4 优化测试", sm4_encrypt_aesni, sm4_decrypt_aesni, key,
           plaintext, expected_ciphertext);

  run_test("SM4 T-table 优化测试", sm4_encrypt_ttable, sm4_decrypt_ttable, key,
           plaintext, expected_ciphertext);

  return 0;
}
