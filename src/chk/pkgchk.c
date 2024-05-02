#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <chk/pkgchk.h>
#include <debug/debug.h>
#include <utils/str.h>
#include <tree/merkletree.h>
#include <utils/queue.h>

#define SHA256_HEXLEN (64)

struct merkle_tree_node* construct_non_leaf_nodes(FILE* bpkg_file, uint32_t nhashes);
struct merkle_tree_node* construct_leaf_nodes(struct merkle_tree_node* root, FILE* bpkg_file, uint32_t nchunks);

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
            if (*endptr != '\0' && !isspace(*endptr)){
                d_print("bpkg_load", "The string in field size is not a int");
            }
            obj->size = right_int;
        } else if (strcmp(left, "nhashes") == 0){
            right_int = strtol(right, &endptr, 10);
            if (*endptr != '\0' && !isspace(*endptr)){
                d_print("bpkg_load", "The string in field nhashes is not a int");
            }
            obj->nhashes = right_int;
        } else if (strcmp(left, "hashes") == 0){
            struct merkle_tree_node* root = construct_non_leaf_nodes(bpkg_file, obj->nhashes);
            obj->merkle_tree->root = root;
        } else if (strcmp(left, "nchunks") == 0){
            right_int = strtol(right, &endptr, 10);
            if (*endptr != '\0' && !isspace(*endptr)){
                d_print("bpkg_load", "The string in field nchunks is not a int");
            }
            obj->nchunks = right_int;
        } else if (strcmp(left, "chunks") == 0){
            obj->merkle_tree->root = construct_leaf_nodes(obj->merkle_tree->root, bpkg_file, obj->nchunks);
        }
    }

    return obj;
}

struct merkle_tree_node* construct_non_leaf_nodes(FILE* bpkg_file, uint32_t nhashes){
    /**
     * Used to construct non-leaf nodes from hashes field
     * Called by bpkg_load
    */
    d_print("construct_non_leaf_nodes", "Start constructing non-leaf nodes");
    char file_line[100];
    struct merkle_tree_node* root = malloc(sizeof(struct merkle_tree_node));
    root->left = NULL;
    root->right = NULL;

    fgets(file_line, sizeof(file_line), bpkg_file);
    delete_whitespace_in_the_front(file_line);
    delete_newline_in_the_end(file_line);
    strcpy(root->expected_hash, file_line);

    struct Queue* queue = createQueue();
    enqueue(queue, (void*)root);
    
    for (int i = 0; i < nhashes-1; i++){ //The root has already been processed
        fgets(file_line, sizeof(file_line), bpkg_file);
        delete_whitespace_in_the_front(file_line);
        delete_newline_in_the_end(file_line);
        d_print("construct_non_leaf_nodes", "read from bpkg file: %s", file_line);

        // Get the parent
        struct merkle_tree_node* parent = (struct merkle_tree_node*)queue_get(queue);

        // Create current node
        struct merkle_tree_node* current = malloc(sizeof(struct merkle_tree_node));
        current->left = NULL;
        current->right = NULL;
        current->is_leaf = 0;
        strcpy(current->expected_hash, file_line);

        if (parent->left == NULL){
            parent->left = current;
        } else {
            parent->right = current;
            dequeue(queue); // The parent's left and right children are all set, so dequeue
        }
        enqueue(queue, (void*)current); // The current node is queueing
    }
    d_print("construct_non_leaf_node", "The non-leaf nodes are successfully constructed");

    free_queue(queue);

    return root;
}

struct merkle_tree_node* construct_leaf_nodes(struct merkle_tree_node* root, FILE* bpkg_file, uint32_t nchunks){
    d_print("construct_leaf_nodes", "Start constructing leaf nodes with nchunks %u", nchunks);
    char file_line[100];

    struct Queue* queue = createQueue();
    enqueue(queue, (void*)root); // Start from the root for linking leaves

    while (!is_queue_empty(queue)) {
        struct merkle_tree_node* parent = (struct merkle_tree_node*)dequeue(queue);
        if (parent->left != NULL){
            enqueue(queue, (void*)parent->left);
        }else if (parent->left == NULL){
            break;
        }
        if (parent->right != NULL) enqueue(queue, (void*)parent->right);
    }

    for (int i = 0; i < nchunks; i++) {
        if (fgets(file_line, sizeof(file_line), bpkg_file) == NULL) break;
        delete_whitespace_in_the_front(file_line);
        delete_newline_in_the_end(file_line);
        d_print("construct_leaf_nodes", "The file line read in current loop is %s", file_line);
        struct split_on_comma_return splited_file_line = split_on_comma(file_line);

        struct merkle_tree_node* leaf = malloc(sizeof(struct merkle_tree_node));
        leaf->left = NULL;
        leaf->right = NULL;
        strcpy(leaf->expected_hash, splited_file_line.hash);
        free(splited_file_line.hash);
        leaf->offset_in_file = splited_file_line.offset;
        leaf->size_in_file = splited_file_line.size;
        leaf->is_leaf = 1; // Mark this node as a leaf node

        if (!is_queue_empty(queue)) {
            struct merkle_tree_node* parent = (struct merkle_tree_node*)dequeue(queue);
            if (parent->left == NULL) {
                d_print("construst_leaf_nodes", "We have assgined the leaf to parent's left");
                parent->left = leaf;
            } else if (parent->right == NULL) {
                d_print("construst_leaf_nodes", "We have assgined the leaf to parent's right");
                parent->right = leaf;
            }
        }
    }

    free_queue(queue);
    d_print("construct_leaf_nodes", "Leaf nodes construction completed");
    return root; // Return the updated tree with leaves
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
    /**
     * Check if the file exists, if not, create and fill it with 0
     * Implemented with mmap
    */
    struct bpkg_query qry;
    int fd;
    qry.hashes = malloc(sizeof(char*));  // Allocate memory for a pointer to strings
    if (!qry.hashes) {
        d_print("bpkg_file_check", "Failed to allocate memory for qry.hashes (qry.hashse is a pointer to the pointer of the string)");
        exit(EXIT_FAILURE);
    }

    qry.hashes[0] = malloc(64 * sizeof(char));  // Assume 64 bytes are enough to store the message
    if (!qry.hashes[0]) {
        d_print("bpkg_file_check", "Failed to allocate memory for qry.hashes[0]");
        exit(EXIT_FAILURE);
    }

    fd = open(bpkg->filename, O_RDWR | O_CREAT | O_EXCL, 0666);
    if (fd == -1) {
        if (errno == EEXIST) {
            // File already exists
            strcpy(qry.hashes[0], "File Exists");
            d_print("bpkg_file_check", "The file exists");
            qry.len = 1;
        } else {
            // Error occurred while attempting to open file
            d_print("bpkg_file_check", "Failed to open the file");
            free(qry.hashes[0]);
            free(qry.hashes);
            exit(EXIT_FAILURE);
        }
    } else {
        // File did not exist and was created
        strcpy(qry.hashes[0], "File Created");
        d_print("bpkg_file_check", "File not exists, create");
        qry.len = 1;

        // Ensure file size is correct
        if (ftruncate(fd, bpkg->size) == -1) {
            d_print("bpkg_file_check", "Failed to truncate the file");
            close(fd);
            free(qry.hashes[0]);
            free(qry.hashes);
            exit(EXIT_FAILURE);
        }

        // Memory-map the file
        void *addr = mmap(NULL, bpkg->size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (addr == MAP_FAILED) {
            d_print("bpkg_file_check", "Failed to memory-map the file");
            close(fd);
            free(qry.hashes[0]);
            free(qry.hashes);
            exit(EXIT_FAILURE);
        }

        // Initialize file content to zero
        memset(addr, 0, bpkg->size);
        d_print("bpkg_file_check","fill 0 into the created file");

        // Cleanup mapping and file descriptor
        munmap(addr, bpkg->size);
    }

    close(fd);
    return qry;
}

/**
 * Retrieves a list of all hashes within the package/tree
 * @param bpkg, constructed bpkg object
 * @return query_result, This structure will contain a list of hashes
 * 		and the number of hashes that have been retrieved
 */
struct bpkg_query bpkg_get_all_hashes(struct bpkg_obj* bpkg) {
    struct bpkg_query qry = { 0 };

    if (bpkg == NULL || bpkg->merkle_tree == NULL || bpkg->merkle_tree->root == NULL) {
        d_print("bpkg_get_all_hashes", "Invalid Merkle tree data.\n");
        return qry;  // Return empty query result if input is invalid
    }

    // Initialize queue for BFS
    struct Queue* queue = createQueue();
    enqueue(queue, bpkg->merkle_tree->root);

    // Temporary storage for collecting hashes
    qry.hashes = malloc((bpkg->nchunks+bpkg->nhashes) * sizeof(char*));
    if (qry.hashes == NULL) {
        d_print("bpkg_get_all_hashes", "Failed to allocate memory for hash array.");
        exit(EXIT_FAILURE);
    }
    size_t count = 0;

    while (!is_queue_empty(queue)) {
        struct merkle_tree_node* current = dequeue(queue);

        // Allocate memory for each hash and copy it
        qry.hashes[count] = malloc(SHA256_HEXLEN + 10);  // SHA256_HEX_LEN is a defined constant for hash size
        if (qry.hashes[count] == NULL) {
            d_print("bpkg_get_all_hashes", "Failed to allocate memory for a hash.");
            exit(EXIT_FAILURE);
        }
        //d_print("bpkg_get_all_hashes", "In the current loop, the expected hash is %s", current->expected_hash);
        strcpy(qry.hashes[count], current->expected_hash);
        count++;

        // Enqueue child nodes
        if (current->left != NULL) {
            enqueue(queue, current->left);
        }
        if (current->right != NULL) {
            enqueue(queue, current->right);
        }
    }

    qry.len = count;  // Set the length of the hash array
    free_queue(queue);  // Clean up the queue
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
    //d_print("bpkg_query_destroy", "function entered");
    // Check if the query structure pointer is not NULL
    if (qry != NULL) {
        d_print("bpkg_query_destroy", "freeing bpkg_query");
        // Check if there is an array of hash pointers
        if (qry->hashes != NULL) {
            // Free each dynamically allocated hash string
            for (size_t i = 0; i < qry->len; i++) {
                if (qry->hashes[i] != NULL) {
                    free(qry->hashes[i]);
                }
            }
            // Free the array holding the hash pointers
            free(qry->hashes);
        }
        // Free the query structure itself if it was dynamically allocated
        // This step depends on how 'qry' was allocated; remove if not applicable
        //free(qry);
    }
}


/**
 * Deallocates memory at the end of the program,
 * make sure it has been completely deallocated
 */
void bpkg_obj_destroy(struct bpkg_obj* obj) {
    free_merkle_tree(obj->merkle_tree);
    free(obj);
}


