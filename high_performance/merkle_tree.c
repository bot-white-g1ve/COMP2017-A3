#include <tree/merkletree.h>
#include <stdlib.h>
#include <debug/debug.h>

void free_merkle_tree(struct merkle_tree* tree){
    /**
     * Free the whole merkle tree
    */
    recursive_free_nodes(tree->root);
    free(tree->root);
    free(tree);
}

void recursive_free_nodes(struct merkle_tree_node* root){
    /**
     * Called by free_merkle_tree
    */
    if (root == NULL) {
        return;
    }

    // Recursively free left subtree
    recursive_free_nodes(root->left);

    // Recursively free right subtree
    recursive_free_nodes(root->right);

    if (root->is_leaf == 1){
        d_print("recursive_free_nodes", "the expected hash of node freeing now is %.64s", root->expected_hash);
        // Free current node
        free(root);
    }
}
