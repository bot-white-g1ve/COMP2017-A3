#ifndef MERKLE_TREE_H
#define MERKLE_TREE_H

#include <stddef.h>
#include <stdint.h>

#define SHA256_HEXLEN (64)

struct merkle_tree_node {
    void* key;
    void* value;
    struct merkle_tree_node* left;
    struct merkle_tree_node* right;
    int is_leaf;
    uint32_t offset_in_file;
    uint32_t size_in_file;
    char expected_hash[SHA256_HEXLEN];
    char computed_hash[SHA256_HEXLEN];
};


struct merkle_tree {
    struct merkle_tree_node* root;
    size_t n_nodes;
};

extern void free_merkle_tree(struct merkle_tree* tree);
extern void recursive_free_nodes(struct merkle_tree_node* root);

#endif
