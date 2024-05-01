#include <tree/merkletree.h>
#include <stdlib.h>

void free_merkle_tree(struct merkle_tree* tree){
    /**
     * Free the whole merkle tree
    */
    recursive_free_nodes(tree->root);
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

    // Free current node
    free(root);
}
