#include "MerkleTree.h"
#include "sm3.h"

#include <stdlib.h>
#include <time.h>

#define LEAF_COUNT 100000

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

int main() {
  srand((unsigned)time(NULL));

  uint8_t **leaf_data = malloc(sizeof(uint8_t *) * LEAF_COUNT);
  size_t *lengths = malloc(sizeof(size_t) * LEAF_COUNT);

  generate_random_data(leaf_data, lengths);

  MerkleNode *root = NULL;

  BuildEngineeringMerkleTree(&root, (const uint8_t **)leaf_data, lengths,
                             LEAF_COUNT);

  printf("Merkle Root: ");
  for (int i = 0; i < 32; ++i)
    printf("%02x", root->hash[i]);
  printf("\n");

  size_t test_index = rand() % LEAF_COUNT;
  printf("Testing inclusion of leaf at index: %zu\n", test_index);

  MerkleProofItem proof[32];
  size_t proof_len = 0;

  if (!BuildProofRFC6962(root, test_index, LEAF_COUNT, proof, &proof_len)) {
    fprintf(stderr, "Failed to build proof\n");
    return 1;
  }

  if (VerifyMerkleProofRFC6962(leaf_data[test_index], lengths[test_index],
                               test_index, proof, proof_len, root->hash)) {
    printf("Leaf at index %zu is VALID in the Merkle tree.\n", test_index);
  } else {
    printf("Leaf at index %zu is INVALID in the Merkle tree.\n", test_index);
  }

  free_data(leaf_data);
  free(leaf_data);
  free(lengths);
  free_merkle_tree(root);
  return 0;
}
