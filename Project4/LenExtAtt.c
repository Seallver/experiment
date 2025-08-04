#include "LenExtAtt.h"

void hash_to_state(const uint8_t hash[SM3_HASH_SIZE],
                   uint32_t state[SM3_HASH_WORDS]) {
  for (int i = 0; i < SM3_HASH_WORDS; i++) {
    state[i] = ((uint32_t)hash[i * 4] << 24) |
               ((uint32_t)hash[i * 4 + 1] << 16) |
               ((uint32_t)hash[i * 4 + 2] << 8) | ((uint32_t)hash[i * 4 + 3]);
  }
}

size_t calculate_padding(size_t len) {
  // SM3填充规则: 先加0x80，然后加0x00直到长度 56 mod 64
  size_t remainder = (len + 1) % SM3_BLOCK_SIZE; // +1 for 0x80
  if (remainder <= 56) {
    return 56 - remainder;
  } else {
    return (SM3_BLOCK_SIZE - remainder) + 56;
  }
}

void sm3_length_extension_attack(const uint8_t original_hash[SM3_HASH_SIZE],
                                 const uint8_t *extension, size_t extension_len,
                                 size_t original_len,
                                 uint8_t result_hash[SM3_HASH_SIZE]) {
  SM3_CTX ctx;
  memset(&ctx, 0, sizeof(SM3_CTX));

  // 恢复 hash 状态
  hash_to_state(original_hash, ctx.hash);
  // 计算原始消息填充后的总长度
  size_t padding_len = calculate_padding(original_len);
  size_t total_padded_len = original_len + 1 + padding_len + 8;

  // 设置 total_len 为“伪造完整消息”的总长度
  ctx.total_len = total_padded_len;
  ctx.buffer_len = 0;

  // 只添加 extension，padding 交给 sm3_final 自动完成
  sm3_update(&ctx, extension, extension_len);

  // 生成哈希
  sm3_final(&ctx, result_hash);
}

void calculate_correct_hash(const uint8_t *msg, size_t msg_len,
                            const uint8_t *extension, size_t ext_len,
                            uint8_t digest[SM3_HASH_SIZE]) {
  SM3_CTX ctx;
  sm3_init(&ctx);

  // 1. 处理原始消息
  sm3_update(&ctx, msg, msg_len);

  // 2. 添加填充
  uint8_t pad[SM3_BLOCK_SIZE] = {0};
  pad[0] = 0x80;
  size_t padding_len = calculate_padding(msg_len);

  sm3_update(&ctx, pad, 1); // 添加0x80
  if (padding_len > 0) {
    sm3_update(&ctx, pad + 1, padding_len); // 添加0x00填充
  }

  // 3. 添加长度(以位为单位)
  uint8_t len_bytes[8];
  uint64_t bit_len = msg_len * 8;
  for (int i = 0; i < 8; i++) {
    len_bytes[i] = (bit_len >> (56 - i * 8)) & 0xFF;
  }
  sm3_update(&ctx, len_bytes, 8);

  // 4. 处理扩展数据
  sm3_update(&ctx, extension, ext_len);

  // 5. 获取最终哈希
  sm3_final(&ctx, digest);
}