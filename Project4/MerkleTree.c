#include "MerkleTree.h"

void LeafHash(uint8_t hash[32], const uint8_t *data, size_t len) {
  SM3_CTX ctx;
  sm3_init(&ctx);
  uint8_t prefix = 0x00;
  sm3_update(&ctx, &prefix, 1);
  sm3_update(&ctx, data, len);
  sm3_final(&ctx, hash);
}

void NodeHash(uint8_t hash[32], const uint8_t left[32],
              const uint8_t right[32]) {
  SM3_CTX ctx;
  sm3_init(&ctx);
  uint8_t prefix = 0x01;
  sm3_update(&ctx, &prefix, 1);
  sm3_update(&ctx, left, 32);
  sm3_update(&ctx, right, 32);
  sm3_final(&ctx, hash);
}

size_t LargestPowerOfTwoLessThan(size_t n) {
  size_t p = 1;
  while ((p << 1) < n)
    p <<= 1;
  return p;
}

void BuildEngineeringMerkleTree(MerkleNode **root, const uint8_t *data[],
                                const size_t lengths[], size_t count) {
  if (count == 0) {
    *root = NULL;
    return;
  }

  *root = (MerkleNode *)malloc(sizeof(MerkleNode));
  if (count == 1) {
    LeafHash((*root)->hash, data[0], lengths[0]);
    (*root)->left = NULL;
    (*root)->right = NULL;

    return;
  }

  size_t left_count = LargestPowerOfTwoLessThan(count);

  MerkleNode *left = NULL, *right = NULL;
  BuildEngineeringMerkleTree(&left, data, lengths, left_count);
  BuildEngineeringMerkleTree(&right, data + left_count, lengths + left_count,
                             count - left_count);

  (*root)->left = left;
  (*root)->right = right;
  NodeHash((*root)->hash, left->hash, right->hash);
}

bool BuildProofRFC6962(MerkleNode *node, size_t index, size_t total_leaves,
                       MerkleProofItem proof[], size_t *proof_index) {
  if (!node->left && !node->right) {
    return index == 0;
  }

  size_t left_count = 1;
  while ((left_count << 1) < total_leaves)
    left_count <<= 1;

  bool found;

  if (index < left_count) {
    // 在左子树
    found =
        BuildProofRFC6962(node->left, index, left_count, proof, proof_index);
    if (found && node->right) {
      memcpy(proof[*proof_index].hash, node->right->hash, 32);
      proof[*proof_index].is_left = false; // sibling on right
      (*proof_index)++;
    }
  } else {
    // 在右子树
    found = BuildProofRFC6962(node->right, index - left_count,
                              total_leaves - left_count, proof, proof_index);
    if (found) {
      memcpy(proof[*proof_index].hash, node->left->hash, 32);
      proof[*proof_index].is_left = true; // sibling on left
      (*proof_index)++;
    }
  }
  return found;
}

bool VerifyMerkleProofRFC6962(const uint8_t *leaf_data, size_t leaf_len,
                              size_t index, const MerkleProofItem proof[],
                              size_t proof_len, const uint8_t *expected_root) {
  uint8_t current[32];
  SM3_CTX ctx;

  // 计算叶子 hash: H(0x00 || data)
  sm3_init(&ctx);
  uint8_t prefix_leaf = 0x00;
  sm3_update(&ctx, &prefix_leaf, 1);
  sm3_update(&ctx, leaf_data, leaf_len);
  sm3_final(&ctx, current);

  for (size_t i = 0; i < proof_len; i++) {
    uint8_t buffer[65]; // 1 prefix + 64 data
    SM3_CTX ctx2;
    sm3_init(&ctx2);
    buffer[0] = 0x01; // internal node prefix

    if (proof[i].is_left) {
      memcpy(buffer + 1, proof[i].hash, 32);
      memcpy(buffer + 33, current, 32);
    } else {
      memcpy(buffer + 1, current, 32);
      memcpy(buffer + 33, proof[i].hash, 32);
    }

    sm3_update(&ctx2, buffer, 65);
    sm3_final(&ctx2, current);
  }

  return memcmp(current, expected_root, 32) == 0;
}
