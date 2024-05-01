#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <chk/pkgchk.h>
#include <debug/debug.h>
#include <utils/str.h>
#include <tree/merkletree.h>
#include <utils/queue.h>

struct merkle_tree_node* construct_non_leaf_nodes(FILE* bpkg_file, uint32_t nhashes);

// PART 1


/**
 * Loads the package for when a valid path is given
 */
struct bpkg_obj* bpkg_load(const char* path) {
    struct bpkg_obj* obj = malloc(sizeof(struct bpkg_obj));

    FILE* bpkg_file = fopen(path, "r");
    if (bpkg_file == NULL) {
        d_print("bpkg_load", "Couldn't open the bpkg file");
        exit(EXIT_FAILURE);
    }
    
    char file_line[1200];
    char left[75];
    char right[1025];
    uint32_t right_int; //might not be used
    char* endptr; //might not be used
    struct merkle_tree* tree = malloc(sizeof(struct merkle_tree));
    obj->merkle_tree = tree;

    // read the file and load into obj  
    while (fgets(file_line, sizeof(file_line), bpkg_file) != NULL){
        split_on_first_colon(file_line, left, right);
        if (strcmp(left, "ident") == 0){
            delete_newline_in_the_end(right);
            strcpy(obj->ident, right);
        } else if (strcmp(left, "filename") == 0){
            delete_newline_in_the_end(right);
            strcpy(obj->filename, right);
        } else if (strcmp(left, "size") == 0){
            right_int = strtol(right, &endptr, 10);
            if (*endptr != '\0'){
                d_print("bpkg_load", "The string in field size is not a int");
            }
            obj->size = right_int;
        } else if (strcmp(left, "nhashes") == 0){
            right_int = strtol(right, &endptr, 10);
            if (*endptr != '\0'){
                d_print("bpkg_load", "The string in field nhashes is not a int");
            }
            obj->nhashes = right_int;
        } else if (strcmp(left, "hashes") == 0){
            struct merkle_tree_node* root = construct_non_leaf_nodes(bpkg_file, obj->nhashes);
            obj->merkle_tree->root = root;
        } else if (strcmp(left, "nchunks") == 0){
            right_int = strtol(right, &endptr, 10);
            if (*endptr != '\0'){
                d_print("bpkg_load", "The string in field nchunks is not a int");
            }
            obj->nchunks = right_int;
        } else if (strcmp(left, "chunks") == 0){
            ;
        }
    }

    return obj;
}

struct merkle_tree_node* construct_non_leaf_nodes(FILE* bpkg_file, uint32_t nhashes){
    /**
     * Used to construct non-leaf nodes from hashes field
     * Called by bpkg_load
    */
    char file_line[1200];
    struct merkle_tree_node* root = malloc(sizeof(struct merkle_tree_node));
    root->left = NULL;
    root->right = NULL;

    fgets(file_line, sizeof(file_line), bpkg_file);
    delete_whitespace_in_the_front(file_line);
    strcpy(root->expected_hash, file_line);

    struct Queue* queue = createQueue();
    enqueue(queue, root);

    for (int i = 0; i < nhashes; i++){
        fgets(file_line, sizeof(file_line), bpkg_file);
        delete_whitespace_in_the_front(file_line);

        // Get the parent
        struct merkle_tree_node* parent = queue_get(queue);

        // Create current node
        struct merkle_tree_node* current = malloc(sizeof(struct merkle_tree_node));
        current->left = NULL;
        current->right = NULL;
        strcpy(current->expected_hash, file_line);

        if (parent->left == NULL){
            parent->left = current;
        } else {
            parent->right = current;
            dequeue(queue); // The parent's left and right children are all set, so dequeue
        }
        enqueue(queue, current); // The current node is queueing
    }

    free_queue(queue);

    return root;
}

/**
 * Checks to see if the referenced filename in the bpkg file
 * exists or not.
 * @param bpkg, constructed bpkg object
 * @return query_result, a single string should be
 *      printable in hashes with len sized to 1.
 * 		If the file exists, hashes[0] should contain "File Exists"
 *		If the file does not exist, hashes[0] should contain "File Created"
 */
struct bpkg_query bpkg_file_check(struct bpkg_obj* bpkg){
    ;
}

/**
 * Retrieves a list of all hashes within the package/tree
 * @param bpkg, constructed bpkg object
 * @return query_result, This structure will contain a list of hashes
 * 		and the number of hashes that have been retrieved
 */
struct bpkg_query bpkg_get_all_hashes(struct bpkg_obj* bpkg) {
    struct bpkg_query qry = { 0 };
    
    return qry;
}

/**
 * Retrieves all completed chunks of a package object
 * @param bpkg, constructed bpkg object
 * @return query_result, This structure will contain a list of hashes
 * 		and the number of hashes that have been retrieved
 */
struct bpkg_query bpkg_get_completed_chunks(struct bpkg_obj* bpkg) { 
    struct bpkg_query qry = { 0 };
    return qry;
}


/**
 * Gets the mininum of hashes to represented the current completion state
 * Example: If chunks representing start to mid have been completed but
 * 	mid to end have not been, then we will have (N_CHUNKS/2) + 1 hashes
 * 	outputted
 *
 * @param bpkg, constructed bpkg object
 * @return query_result, This structure will contain a list of hashes
 * 		and the number of hashes that have been retrieved
 */
struct bpkg_query bpkg_get_min_completed_hashes(struct bpkg_obj* bpkg) {
    struct bpkg_query qry = { 0 };
    return qry;
}


/**
 * Retrieves all chunk hashes given a certain an ancestor hash (or itself)
 * Example: If the root hash was given, all chunk hashes will be outputted
 * 	If the root's left child hash was given, all chunks corresponding to
 * 	the first half of the file will be outputted
 * 	If the root's right child hash was given, all chunks corresponding to
 * 	the second half of the file will be outputted
 * @param bpkg, constructed bpkg object
 * @return query_result, This structure will contain a list of hashes
 * 		and the number of hashes that have been retrieved
 */
struct bpkg_query bpkg_get_all_chunk_hashes_from_hash(struct bpkg_obj* bpkg, 
    char* hash) {
    
    struct bpkg_query qry = { 0 };
    return qry;
}


/**
 * Deallocates the query result after it has been constructed from
 * the relevant queries above.
 */
void bpkg_query_destroy(struct bpkg_query* qry) {
    //TODO: Deallocate here!

}

/**
 * Deallocates memory at the end of the program,
 * make sure it has been completely deallocated
 */
void bpkg_obj_destroy(struct bpkg_obj* obj) {
    free_merkle_tree(obj->merkle_tree);
    free(obj);
}


