#ifndef MERKLE_TREE_H
#define MERKLE_TREE_H

#include "sm3.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Merkle树节点结构
typedef struct MerkleNode {
  uint8_t hash[SM3_HASH_SIZE];
  struct MerkleNode *left;
  struct MerkleNode *right;
} MerkleNode;

void LeafHash(uint8_t hash[32], const uint8_t *data, size_t len);

void NodeHash(uint8_t hash[32], const uint8_t left[32],
              const uint8_t right[32]);

void BuildEngineeringMerkleTree(MerkleNode **root, const uint8_t *data[],
                                const size_t lengths[], size_t count);

typedef struct {
  uint8_t hash[32];
  bool is_left;
} MerkleProofItem;

bool BuildInclusiveProof(MerkleNode *node, size_t index, size_t total_leaves,
                         MerkleProofItem proof[], size_t *proof_index);

void ComputVrfyRoot(const uint8_t *leaf_data, size_t leaf_len, size_t index,
                    const MerkleProofItem proof[], size_t proof_len,
                    uint8_t *vrfy_root);

#define LEAF_COUNT 100000

// 构建叶子不在树中的证明
int BuildExclusiveProof(MerkleNode *root,
                        const uint8_t leaf_hashes[LEAF_COUNT][SM3_HASH_SIZE],
                        const uint8_t target_hash[SM3_HASH_SIZE],
                        MerkleProofItem proof1[32], size_t *proof_len1,
                        MerkleProofItem proof2[32], size_t *proof_len2,
                        size_t *index1, size_t *index2);

#endif