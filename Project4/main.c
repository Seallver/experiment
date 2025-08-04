// main.c

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "LenExtAtt.h" // 长度扩展攻击实现
#include "sm3.h"       // 原版实现
#include "sm3_opt.h"   // 优化实现

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
  sm3_OPT((unsigned char *)msg, (unsigned int)msglen, digest_opt);

  // 输出结果
  printf("%s\n", label);
  printf("Standard: ");
  print_hash(digest_std);
  printf("Optimized: ");
  print_hash(digest_opt);

  // 比较输出
  if (compare_hashes(digest_std, digest_opt)) {
    printf("SUCCESS: Output matches.\n\n");
  } else {
    printf("FAILURE: Output differs!\n\n");
  }
}

void test_LengthExtensionAttack() {
  // 测试用例
  const char *original_msg = "This is a secret message";
  const char *extension = "This is a message appended by attacker";

  // 计算原始哈希
  uint8_t original_hash[SM3_HASH_SIZE];
  SM3_CTX ctx;
  sm3_init(&ctx);
  sm3_update(&ctx, (const uint8_t *)original_msg, strlen(original_msg));
  sm3_final(&ctx, original_hash);

  printf("Original message: '%s' (%zu bytes)\n", original_msg,
         strlen(original_msg));
  printf("Original hash: ");
  print_hash(original_hash);

  printf("Extension: '%s' (%zu bytes)\n", extension, strlen(extension));

  // 执行攻击
  uint8_t attack_hash[SM3_HASH_SIZE];
  sm3_length_extension_attack(original_hash, (const uint8_t *)extension,
                              strlen(extension), strlen(original_msg),
                              attack_hash);

  printf("Attack result:   ");
  print_hash(attack_hash);

  // 计算正确结果
  uint8_t correct_hash[SM3_HASH_SIZE];
  calculate_correct_hash((const uint8_t *)original_msg, strlen(original_msg),
                         (const uint8_t *)extension, strlen(extension),
                         correct_hash);

  printf("Correct result:  ");
  print_hash(correct_hash);

  // 验证
  if (memcmp(attack_hash, correct_hash, SM3_HASH_SIZE) == 0) {
    printf("SUCCESS: Attack worked!\n");
  } else {
    printf("FAILURE: Attack failed\n");

    // 调试信息
    printf("\nDebugging info:\n");
    printf("Original length: %zu bytes\n", strlen(original_msg));

    size_t padding = calculate_padding(strlen(original_msg));
    printf("Padding needed: %zu bytes (0x80 + %zu 0x00's)\n", padding + 1,
           padding);

    size_t total_padded = strlen(original_msg) + 1 + padding + 8;
    printf("Total padded length: %zu bytes\n", total_padded);
    printf("Total with extension: %zu bytes\n",
           total_padded + strlen(extension));
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

  test_LengthExtensionAttack();

  return 0;
}