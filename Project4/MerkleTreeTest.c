#include "MerkleTree.h"
#include "sm3.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define LEAF_COUNT 100000

typedef struct {
  uint8_t hash[SM3_HASH_SIZE];
  uint8_t *data;
  size_t len;
} LeafItem;

void generate_random_data(uint8_t **data_array, size_t *lengths) {
  for (size_t i = 0; i < LEAF_COUNT; ++i) {
    data_array[i] = (uint8_t *)malloc(32); // 每个叶子32字节
    for (int j = 0; j < 32; ++j) {
      data_array[i][j] = rand() % 256;
    }
    lengths[i] = 32; //填充对应的长度
  }
}

void free_data(uint8_t **data_array) {
  for (size_t i = 0; i < LEAF_COUNT; ++i) {
    free(data_array[i]);
  }
}

void free_merkle_tree(MerkleNode *node) {
  if (!node)
    return;
  free_merkle_tree(node->left);
  free_merkle_tree(node->right);
  free(node);
}

int compare_leaf_item(const void *a, const void *b) {
  return memcmp(((LeafItem *)a)->hash, ((LeafItem *)b)->hash, SM3_HASH_SIZE);
}

void calc_sm3_hash(const uint8_t *data, size_t len,
                   uint8_t hash_out[SM3_HASH_SIZE]) {
  SM3_CTX ctx;
  sm3_init(&ctx);
  sm3_update(&ctx, data, (uint32_t)len);
  sm3_final(&ctx, hash_out);
}

int main() {
  srand((unsigned)time(NULL));

  uint8_t **leaf_data = malloc(sizeof(uint8_t *) * LEAF_COUNT);
  uint8_t(*leaf_hashes)[SM3_HASH_SIZE] =
      malloc(sizeof(uint8_t[SM3_HASH_SIZE]) * LEAF_COUNT);
  size_t *lengths = malloc(sizeof(size_t) * LEAF_COUNT);

  LeafItem *items = malloc(sizeof(LeafItem) * LEAF_COUNT);

  // 生成随机数据并按哈希排序
  generate_random_data(leaf_data, lengths);
  for (size_t i = 0; i < LEAF_COUNT; ++i) {
    items[i].data = leaf_data[i];
    items[i].len = lengths[i];
    calc_sm3_hash(leaf_data[i], lengths[i], items[i].hash); // 计算 hash
  }
  qsort(items, LEAF_COUNT, sizeof(LeafItem), compare_leaf_item);

  for (size_t i = 0; i < LEAF_COUNT; ++i) {
    leaf_data[i] = items[i].data;
    lengths[i] = items[i].len;
    memcpy(leaf_hashes[i], items[i].hash, SM3_HASH_SIZE);
  }

  MerkleNode *root = NULL;
  printf("Building Merkle Tree with %d leaves...\n", LEAF_COUNT);
  BuildEngineeringMerkleTree(&root, (const uint8_t **)leaf_data, lengths,
                             LEAF_COUNT);

  printf("Merkle Root: ");
  for (int i = 0; i < 32; ++i)
    printf("%02x", root->hash[i]);
  printf("\n\n");

  size_t test_index = rand() % LEAF_COUNT;
  printf("Testing inclusion of leaf at index: %zu\n", test_index);

  MerkleProofItem proof[32];
  size_t proof_len = 0;

  if (!BuildInclusiveProof(root, test_index, LEAF_COUNT, proof, &proof_len)) {
    fprintf(stderr, "Failed to build proof\n");
    return 1;
  }

  uint8_t vrfy_root[SM3_HASH_SIZE];
  ComputVrfyRoot(leaf_data[test_index], lengths[test_index], test_index, proof,
                 proof_len, vrfy_root);
  if (memcmp(vrfy_root, root->hash, 32) == 0) {
    printf("Computed root: ");
    for (int i = 0; i < 32; ++i)
      printf("%02x", vrfy_root[i]);
    printf(" matches the Merkle tree root.\n");
    printf("Leaf at index %zu is VALID in the Merkle tree.\n", test_index);
  } else {
    printf("Leaf at index %zu is INVALID in the Merkle tree.\n", test_index);
  }

  printf("\n");

  // 构造一个目标哈希（随机产生，几率极小与叶子重复）
  uint8_t target_hash[SM3_HASH_SIZE];
  for (int i = 0; i < SM3_HASH_SIZE; i++) {
    target_hash[i] = rand() % 256;
  }
  printf("Testing exclusive of leaf at hash: ");
  for (int i = 0; i < SM3_HASH_SIZE; i++)
    printf("%02x", target_hash[i]);
  printf("\n");

  // 调用不在树中证明函数
  MerkleProofItem proof1[32], proof2[32];
  size_t proof_len1 = 0, proof_len2 = 0;
  size_t index1 = 0, index2 = 0;

  int ret =
      BuildExclusiveProof(root, leaf_hashes, target_hash, proof1, &proof_len1,
                          proof2, &proof_len2, &index1, &index2);
  if (ret == 0) {
    printf("Non-existence proof built successfully.\n");
    printf("+++++++++++++++++++++++++++++\n");
    printf("Proof for leaf index %zu (hash: ", index1);
    for (int j = 0; j < SM3_HASH_SIZE; j++) {
      printf("%02x", leaf_hashes[index1][j]);
    }
    printf("):\n");

    uint8_t vrfy_root[SM3_HASH_SIZE];
    ComputVrfyRoot(leaf_data[index1], lengths[index1], index1, proof1,
                   proof_len1, vrfy_root);
    if (memcmp(vrfy_root, root->hash, 32) == 0) {
      printf("Computed root: ");
      for (int i = 0; i < 32; ++i)
        printf("%02x", vrfy_root[i]);
      printf(" matches the Merkle tree root.\n");
      printf("Leaf at index %zu is VALID in the Merkle tree.\n", index1);
    } else {
      printf("Leaf at index %zu is INVALID in the Merkle tree.\n", index1);
    }

    printf("+++++++++++++++++++++++++++++\n");

    if (proof_len2 > 0) {
      printf("Proof for leaf index %zu (hash: ", index2);
      for (int j = 0; j < SM3_HASH_SIZE; j++) {
        printf("%02x", leaf_hashes[index2][j]);
      }
      printf("):\n");

      uint8_t vrfy_root[SM3_HASH_SIZE];
      ComputVrfyRoot(leaf_data[index2], lengths[index2], index2, proof2,
                     proof_len2, vrfy_root);
      if (memcmp(vrfy_root, root->hash, 32) == 0) {
        printf("Computed root: ");
        for (int i = 0; i < 32; ++i)
          printf("%02x", vrfy_root[i]);
        printf(" matches the Merkle tree root.\n");
        printf("Leaf at index %zu is VALID in the Merkle tree.\n", index2);
      } else {
        printf("Leaf at index %zu is INVALID in the Merkle tree.\n", index2);
      }
      printf("+++++++++++++++++++++++++++++\n");
      printf("与其哈希值相邻的两个节点在 Merkle Tree "
             "中位置是相邻的，因此能证明此节点不存在\n");
    }
  } else {
    printf("Failed to build non-existence proof.\n");
  }

  free(leaf_hashes);
  free_data(leaf_data);
  free(leaf_data);
  free(lengths);
  free_merkle_tree(root);

  return 0;
}
