#ifndef LEN_EXT_ATT_H
#define LEN_EXT_ATT_H

#include "sm3.h"
#include <stdio.h>
#include <string.h>

// 将哈希值转换为状态数组
void hash_to_state(const uint8_t hash[SM3_HASH_SIZE],
                   uint32_t state[SM3_HASH_WORDS]);

// 计算需要多少填充字节
size_t calculate_padding(size_t len);

// 执行长度扩展攻击
void sm3_length_extension_attack(const uint8_t original_hash[SM3_HASH_SIZE],
                                 const uint8_t *extension, size_t extension_len,
                                 size_t original_len,
                                 uint8_t result_hash[SM3_HASH_SIZE]);

// 计算正确的哈希用于验证
void calculate_correct_hash(const uint8_t *msg, size_t msg_len,
                            const uint8_t *extension, size_t ext_len,
                            uint8_t digest[SM3_HASH_SIZE]);

//实施长度扩展攻击的测试函数
void test_LengthExtensionAttack();

#endif
