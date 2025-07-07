#include "sm4.h"
#include "sm4_aesni.h"
#include "stdio.h"
#include <string.h>

void print_hex(const uint8_t *data, size_t len) {
  for (size_t i = 0; i < len; i++) {
    printf("%02x", data[i]);
  }
  printf("\n");
}

int main() {
  // 标准示例测试
  printf("\nSM4原始实现标准示例测试\n");

  uint8_t plaintext[16] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
                           0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};

  uint8_t key[16] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
                     0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};

  uint8_t expected_ciphertext[16] = {0x68, 0x1E, 0xDF, 0x34, 0xD2, 0x06,
                                     0x96, 0x5E, 0x86, 0xB3, 0xE9, 0x4F,
                                     0x53, 0x6E, 0x42, 0x46};

  uint8_t ciphertext[16];

  SM4_Key sm4_key;
  sm4_keyInit(key, &sm4_key);

  sm4_encrypt(plaintext, &sm4_key, ciphertext);

  printf("加密结果：\t\t");
  print_hex(ciphertext, 16);

  printf("是否等于标准输出：\t%s\n",
         memcmp(ciphertext, expected_ciphertext, 16) == 0 ? "true" : "false");

  uint8_t decrypted_text[16];

  sm4_decrypt(ciphertext, &sm4_key, decrypted_text);

  printf("解密结果：\t\t");
  print_hex(decrypted_text, 16);

  printf("是否等于原文：\t\t%s\n",
         memcmp(decrypted_text, plaintext, 16) == 0 ? "true" : "false");
  printf("\n");

  // AES-NI x4 测试
  printf("\nSM4 AES-NI x4 优化标准示例测试\n");

  uint8_t ciphertext_aesni[16];

  SM4_Key sm4_key_aesni;
  sm4_keyInit(key, &sm4_key_aesni);

  sm4_encrypt(plaintext, &sm4_key_aesni, ciphertext_aesni);

  printf("加密结果：\t\t");
  print_hex(ciphertext, 16);

  printf("是否等于标准输出：\t%s\n",
         memcmp(ciphertext_aesni, expected_ciphertext, 16) == 0 ? "true"
                                                                : "false");

  uint8_t decrypted_text_aesni[16];

  sm4_decrypt(ciphertext_aesni, &sm4_key_aesni, decrypted_text_aesni);

  printf("解密结果：\t\t");
  print_hex(decrypted_text_aesni, 16);

  printf("是否等于原文：\t\t%s\n",
         memcmp(decrypted_text_aesni, plaintext, 16) == 0 ? "true" : "false");
  printf("\n");

  return 0;
}
