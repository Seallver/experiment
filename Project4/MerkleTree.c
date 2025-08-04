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

bool BuildInclusiveProof(MerkleNode *node, size_t index, size_t total_leaves,
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
        BuildInclusiveProof(node->left, index, left_count, proof, proof_index);
    if (found && node->right) {
      memcpy(proof[*proof_index].hash, node->right->hash, 32);
      proof[*proof_index].is_left = false; // sibling on right
      (*proof_index)++;
    }
  } else {
    // 在右子树
    found = BuildInclusiveProof(node->right, index - left_count,
                                total_leaves - left_count, proof, proof_index);
    if (found) {
      memcpy(proof[*proof_index].hash, node->left->hash, 32);
      proof[*proof_index].is_left = true; // sibling on left
      (*proof_index)++;
    }
  }
  return found;
}

void ComputVrfyRoot(const uint8_t *leaf_data, size_t leaf_len, size_t index,
                    const MerkleProofItem proof[], size_t proof_len,
                    uint8_t *vrfy_root) {
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

  memcpy(vrfy_root, current, 32);
}

// 比较两个hash的字节序
static int cmp_hash(const uint8_t *h1, const uint8_t *h2) {
  return memcmp(h1, h2, SM3_HASH_SIZE);
}

// 二分查找插入位置
static size_t
find_insert_pos(const uint8_t target[SM3_HASH_SIZE],
                const uint8_t leaf_hashes[LEAF_COUNT][SM3_HASH_SIZE]) {
  size_t left = 0, right = LEAF_COUNT;
  while (left < right) {
    size_t mid = (left + right) / 2;
    if (cmp_hash(leaf_hashes[mid], target) <= 0) {
      left = mid + 1;
    } else {
      right = mid;
    }
  }
  return left;
}

int BuildExclusiveProof(MerkleNode *root,
                        const uint8_t leaf_hashes[LEAF_COUNT][SM3_HASH_SIZE],
                        const uint8_t target_hash[SM3_HASH_SIZE],
                        MerkleProofItem proof1[32], size_t *proof_len1,
                        MerkleProofItem proof2[32], size_t *proof_len2,
                        size_t *index1, size_t *index2) {
  if (!root || !leaf_hashes || !target_hash || !proof_len1 || !index1)
    return -1;

  *proof_len1 = 0;
  *proof_len2 = 0;
  *index1 = 0;
  *index2 = 0;

  size_t pos = find_insert_pos(target_hash, leaf_hashes);

  if (pos == 0) {
    // 目标比最小叶子还小
    if (!BuildInclusiveProof(root, 0, LEAF_COUNT, proof1, proof_len1))
      return -1;
    *index1 = 0;
  } else if (pos == LEAF_COUNT) {
    // 目标比最大叶子还大
    if (!BuildInclusiveProof(root, LEAF_COUNT - 1, LEAF_COUNT, proof1,
                             proof_len1))
      return -1;
    *index1 = LEAF_COUNT - 1;
  } else {
    // 目标在中间，分别构造两个proof
    if (!BuildInclusiveProof(root, pos - 1, LEAF_COUNT, proof1, proof_len1))
      return -1;
    if (!BuildInclusiveProof(root, pos, LEAF_COUNT, proof2, proof_len2))
      return -1;
    *index1 = pos - 1;
    *index2 = pos;
  }

  return 0;
}