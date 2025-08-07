#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sm4_gcm.h"

extern const GHASH_METHOD GHASH_COMMAN = {.init = ghash_init,
                                          .update = ghash_update,
                                          .final = ghash_final,
                                          .reset = ghash_reset};

extern const GHASH_METHOD GHASH_TABLE = {.init = ghash_table_init,
                                         .update = ghash_table_update,
                                         .final = ghash_table_final,
                                         .reset = ghash_table_reset};

// 打印十六进制辅助函数
void print_hex(const char *label, const uint8_t *data, size_t len) {
  printf("%s:", label);
  for (size_t i = 0; i < len; ++i) {
    if (i % 16 == 0)
      printf("\n  ");
    printf("%02X ", data[i]);
  }
  printf("\n");
}

void test(const GHASH_METHOD *ghash_impl) {
  // 测试向量（你可以换成真实测试向量）
  uint8_t key[16] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
                     0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};

  uint8_t iv[12] = {0xA1, 0xB2, 0xC3, 0xD4, 0xE5, 0xF6,
                    0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C};

  uint8_t aad[20] = {'A', 'u', 't', 'h', 'e', 'n', 't', 'i', 'c', 'a',
                     't', 'e', 'd', '-', 'H', 'e', 'a', 'd', 'e', 'r'};

  uint8_t plaintext[32] = {'S', 'M', '4', '-', 'G', 'C', 'M', ' ',
                           'E', 'n', 'c', 'r', 'y', 'p', 't', 'i',
                           'o', 'n', ' ', 'T', 'e', 's', 't', '!',
                           ' ', '1', '2', '3', '4', '5', '6', '7'};

  size_t pt_len = sizeof(plaintext);
  size_t aad_len = sizeof(aad);

  uint8_t ciphertext[32];
  uint8_t tag[16];
  uint8_t decrypted[32];

  // ---------- 加密过程 ----------
  GCM_SM4_CTX ctx;
  gcm_sm4_init(&ctx, key, iv, sizeof(iv), ghash_impl);
  gcm_sm4_aad(&ctx, aad, aad_len);
  gcm_sm4_encrypt(&ctx, plaintext, pt_len, ciphertext);
  gcm_sm4_tag(&ctx, tag);

  // 输出加密结果
  print_hex("Plaintext", plaintext, pt_len);
  print_hex("Ciphertext", ciphertext, pt_len);
  print_hex("Tag", tag, 16);

  // ---------- 解密过程 ----------
  int result = sm4_gcm_decrypt(key, iv, sizeof(iv), aad, aad_len, ciphertext,
                               pt_len, tag, decrypted, ghash_impl);

  if (result == 0) {
    printf("\n[✓] Tag verified. Decryption successful!\n");
    print_hex("Decrypted Plaintext", decrypted, pt_len);

    if (memcmp(plaintext, decrypted, pt_len) == 0) {
      printf("[✓] Decrypted plaintext matches original.\n");
    } else {
      printf("[✗] Decrypted plaintext does NOT match original!\n");
    }

  } else {
    printf("\n[✗] Authentication tag mismatch! Decryption failed.\n");
  }
}

int main() {

  printf("test gcm with comman ghash\n");
  test(&GHASH_COMMAN);

  printf("\n==========================\n\n");

  printf("test gcm with ghash table\n");
  test(&GHASH_TABLE);

  return 0;
}