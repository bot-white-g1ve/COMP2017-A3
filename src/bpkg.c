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
#include <crypt/sha256.h>
#include <stdbool.h>

#define SHA256_HEXLEN (64)

struct merkle_tree_node* construct_non_leaf_nodes(FILE* bpkg_file, uint32_t nhashes);
struct merkle_tree_node* construct_leaf_nodes(struct merkle_tree_node* root, FILE* bpkg_file, uint32_t nchunks);

// PART 1


/**
 * Loads the package for when a valid path is given
 */
struct bpkg_obj* bpkg_load(const char* path) {
    struct bpkg_obj* obj = malloc(sizeof(struct bpkg_obj));
    if (!obj) {
        d_print("bpkg_load", "Memory allocation failed for bpkg_obj");
        return NULL;
    }

    FILE* bpkg_file = fopen(path, "r");
    if (bpkg_file == NULL) {
        d_print("bpkg_load", "Couldn't open the bpkg file");
        printf("Cannot open file\n");
        free(obj);
        return NULL;
    }

    char file_line[1200];
    char left[75];
    char right[1025];
    uint32_t right_int; //might not be used
    char* endptr; //might not be used
    struct merkle_tree* tree = malloc(sizeof(struct merkle_tree));
    obj->merkle_tree = tree;

    // Flags to ensure all required fields are read
    bool ident_found = false, filename_found = false, size_found = false;
    bool nhashes_found = false, nchunks_found = false;

    // read the file and load into obj  
    while (fgets(file_line, sizeof(file_line), bpkg_file) != NULL){
        split_on_first_colon(file_line, left, right);
        if (strcmp(left, "ident") == 0){
            delete_newline_in_the_end(right);
            strcpy(obj->ident, right);
            ident_found = true;
        } else if (strcmp(left, "filename") == 0){
            delete_newline_in_the_end(right);
            strcpy(obj->filename, right);
            filename_found = true;
        } else if (strcmp(left, "size") == 0){
            right_int = strtol(right, &endptr, 10);
            if (*endptr != '\0' && !isspace(*endptr)){
                d_print("bpkg_load", "The string in field size is not an int");
                fclose(bpkg_file);
                free(obj);
                return NULL;
            }
            obj->size = right_int;
            size_found = true;
        } else if (strcmp(left, "nhashes") == 0){
            right_int = strtol(right, &endptr, 10);
            if (*endptr != '\0' && !isspace(*endptr)){
                d_print("bpkg_load", "The string in field nhashes is not an int");
                fclose(bpkg_file);
                free(obj);
                return NULL;
            }
            d_print("bpkg_load", "The right_int in nhashes field is %d", right_int);
            obj->nhashes = right_int;
            nhashes_found = true;
        } else if (strcmp(left, "hashes") == 0){
            struct merkle_tree_node* root = construct_non_leaf_nodes(bpkg_file, obj->nhashes);
            obj->merkle_tree->root = root;
        } else if (strcmp(left, "nchunks") == 0){
            right_int = strtol(right, &endptr, 10);
            if (*endptr != '\0' && !isspace(*endptr)){
                d_print("bpkg_load", "The string in field nchunks is not an int");
                fclose(bpkg_file);
                free(obj);
                return NULL;
            }
            obj->nchunks = right_int;
            nchunks_found = true;
        } else if (strcmp(left, "chunks") == 0){
            obj->merkle_tree->root = construct_leaf_nodes(obj->merkle_tree->root, bpkg_file, obj->nchunks);
        }
    }

    fclose(bpkg_file);

    // Check if all required fields were found
    if (!(ident_found && filename_found && size_found && nhashes_found && nchunks_found)) {
        d_print("bpkg_load", "Not all required fields were found");
        printf("Unable to parse bpkg file");
        free(obj);
        return NULL;
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
    strncpy(root->expected_hash, file_line, SHA256_HEXLEN);

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
        strncpy(current->expected_hash, file_line, SHA256_HEXLEN);

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
        struct merkle_tree_node* parent = (struct merkle_tree_node*)queue_get(queue);
        if (parent->left != NULL){
            enqueue(queue, (void*)parent->left);
        }else if (parent->left == NULL){
            break;
        }
        if (parent->right != NULL) enqueue(queue, (void*)parent->right);
        dequeue(queue);
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
        strncpy(leaf->expected_hash, splited_file_line.hash, SHA256_HEXLEN);
        free(splited_file_line.hash);
        leaf->offset_in_file = splited_file_line.offset;
        leaf->size_in_file = splited_file_line.size;
        leaf->is_leaf = 1; // Mark this node as a leaf node

        if (!is_queue_empty(queue)) {
            struct merkle_tree_node* parent = (struct merkle_tree_node*)queue_get(queue);
            if (parent->left == NULL) {
                d_print("construst_leaf_nodes", "We have assgined the leaf to parent's left");
                parent->left = leaf;
            } else if (parent->right == NULL) {
                d_print("construst_leaf_nodes", "We have assgined the leaf to parent's right");
                parent->right = leaf;
                dequeue(queue);
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
    struct stat st;

    qry.hashes = malloc(sizeof(char*));  // Allocate memory for a pointer to strings
    if (!qry.hashes) {
        d_print("bpkg_file_check", "Failed to allocate memory for qry.hashes (qry.hashse is a pointer to the pointer of the string)");
        exit(EXIT_FAILURE);
    }

    qry.hashes[0] = malloc(64 * sizeof(char));  // Assume 64 bytes are enough to store the message
    if (!qry.hashes[0]) {
        d_print("bpkg_file_check", "Failed to allocate memory for qry.hashes[0]");
        free(qry.hashes);
        exit(EXIT_FAILURE);
    }

    if (access(bpkg->filename, F_OK) == 0){
        // File exists
        strcpy(qry.hashes[0], "File Exists");
        d_print("bpkg_file_check", "The file exists");
        qry.len = 1;
        if (stat(bpkg->filename, &st) == 0){
            off_t actual_size = st.st_size;
            if (actual_size == bpkg->size){
                d_print("bpkg_file_check", "The size of the existing file is correct");
                ;
            } else if (actual_size != bpkg->size){
                fd = open(bpkg->filename, O_RDWR);
                if (fd == -1){
                    d_print("bpkg_file_check", "File exists. Error opening file");
                    exit(EXIT_FAILURE);
                }

                if (ftruncate(fd, bpkg->size) == -1) {
                    d_print("bpkg_file_check", "File exists. Error truncating file");
                    close(fd);
                    exit(EXIT_FAILURE);
                }

                d_print("bpkg_file_check", "File size adjusted with ftruncate");
                close(fd);
            }
        }else{
            d_print("bpkg_file_check", "File exists. Error getting file stats");
            exit(EXIT_FAILURE);
        }
    } else {
        // File does not exist, create it
        fd = open(bpkg->filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        if (fd == -1) {
            d_print("bpkg_file_check", "Error creating file");
            exit(EXIT_FAILURE);
        }

        if (ftruncate(fd, bpkg->size) == -1) {
            d_print("bpkg_file_check", "Error setting initial file size");
            close(fd);
            exit(EXIT_FAILURE);
        }

        // Map the file and fill with zeros
        void *map = mmap(NULL, bpkg->size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (map == MAP_FAILED) {
            d_print("bpkg_file_check", "Error mapping the new file");
            close(fd);
            exit(EXIT_FAILURE);
        }

        memset(map, 0, bpkg->size); // Set all bytes to zero
        munmap(map, bpkg->size);    // Unmap the file from memory

        strcpy(qry.hashes[0], "File Created");
        d_print("bpkg_file_check", "File not exists, created and initialized to size");
        qry.len = 1;
        close(fd);
    }

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
        strncpy(qry.hashes[count], current->expected_hash, SHA256_HEXLEN);
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

struct bpkg_query bpkg_get_all_chunks(struct bpkg_obj* bpkg){
    /**
     * Getting all the chunks
     * This function is not used if following the specification
     * I am just using it for self-testing of the merkle tree
    */
   struct bpkg_query qry = { 0 };

    if (bpkg == NULL || bpkg->merkle_tree == NULL || bpkg->merkle_tree->root == NULL) {
        d_print("bpkg_get_all_hashes", "Invalid Merkle tree data.\n");
        return qry;  // Return empty query result if input is invalid
    }

    // Initialize queue for BFS
    struct Queue* queue = createQueue();
    enqueue(queue, bpkg->merkle_tree->root);

    // Temporary storage for collecting hashes
    qry.hashes = malloc((bpkg->nchunks) * sizeof(char*));
    if (qry.hashes == NULL) {
        d_print("bpkg_get_all_hashes", "Failed to allocate memory for hash array.");
        exit(EXIT_FAILURE);
    }
    size_t count = 0;

    while (!is_queue_empty(queue)) {
        struct merkle_tree_node* current = dequeue(queue);

         if (current->is_leaf == 1) {
            // Allocate memory for each hash and copy it
            qry.hashes[count] = malloc(SHA256_HEXLEN + 10);  // SHA256_HEX_LEN is a defined constant for hash size
            if (qry.hashes[count] == NULL) {
                d_print("bpkg_get_all_hashes", "Failed to allocate memory for a hash.");
                exit(EXIT_FAILURE);
            }
       
            //d_print("bpkg_get_all_chunks", "The expected hash (in leaf) in current loop is %s", current->expected_hash);
            strncpy(qry.hashes[count], current->expected_hash, SHA256_HEXLEN);
            count++;
        }

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

void check_chunk_completed(struct merkle_tree_node* node, void* binary_data, size_t binary_data_size){
    /**
     * Compute the sha256 hex from binary data and write to node's computed hash
     * Called by check_chunks_comleted
    */
    struct sha256_compute_data sha_data;

    // init sha256 computing
    sha256_compute_data_init(&sha_data);

    // provide binary to the computing algorithm
    sha256_update(&sha_data, binary_data, binary_data_size);

    // finalize computation
    uint8_t hash_output[SHA256_INT_SZ]; 
    sha256_finalize(&sha_data, hash_output);

    // prepare some space for the hash
    char hash_hex[SHA256_HEXLEN];

    // Get hex format of the hash
    sha256_output_hex(&sha_data, hash_hex);

    strncpy(node->computed_hash, hash_hex, SHA256_HEXLEN);

    return;
}

void check_chunks_completed_fix_size(struct merkle_tree_node* root, const char* data_filepath, const uint32_t nhashes){
    /**
     * Compute the sha_256 hash for all the leaf nodes in the tree
     * Here we assume every chunk is in the same size
     * Used in self auto-testing
    */
   FILE* file = fopen(data_filepath, "rb"); // Read binary file
    if (!file) {
        d_print("check_chunks_completed", "Failed to open file");
        return;
    }

    // Get the size
    fseek(file, 0, SEEK_END);
    size_t filesize = ftell(file);
    fseek(file, 0, SEEK_SET);
    d_print("check_chunks_completed", "the file size is %zu", filesize);

    // Compute size for every chunk
    size_t chunk_size = (filesize + nhashes - 1) / nhashes;
    d_print("check_chunks_completed", "the nhashes is %d", nhashes);
    d_print("check_chunks_completed", "the chunk size is %zu", chunk_size);
    size_t read_size;
    uint8_t* buffer = malloc(chunk_size);

    if (!buffer) {
        fclose(file);
        d_print("check_chunks_completed", "Memory allocation failed for buffer");
        return;
    }

    // Initialize queue for BFS
    struct Queue* queue = createQueue();
    enqueue(queue, root);

    int count = 0;

    while (!is_queue_empty(queue)) {
        struct merkle_tree_node* current = dequeue(queue);

         if (current->is_leaf == 1) {
            read_size = fread(buffer, 1, chunk_size, file);
            if (read_size == 0) {
                break; // File end or read error
            }
            d_print("check_chunks_completed", "check_chunk_complete ready to call for node with expected hash %.*s", SHA256_HEXLEN, current->expected_hash);
            check_chunk_completed(current, buffer, read_size);
            d_print("check_chunks_completed", "check_chunk_complete called for node with expected hash %.*s", SHA256_HEXLEN, current->expected_hash);
            count += 1;
        }

        // Enqueue child nodes
        if (current->left != NULL) {
            enqueue(queue, current->left);
        }
        if (current->right != NULL) {
            enqueue(queue, current->right);
        }
    }
    d_print("check_chunks_completed", "The number of chunks are %d", count);

    free(buffer);
    free_queue(queue);
    fclose(file);
}

void check_chunks_completed(struct merkle_tree_node* root, const char* data_filepath, const uint32_t nhashes) {
    FILE* file = fopen(data_filepath, "rb");
    if (!file) {
        d_print("check_chunks_completed", "Failed to open file");
        return;
    }

    // Initialize queue for BFS
    struct Queue* queue = createQueue();
    enqueue(queue, root);

    uint8_t* buffer;
    size_t read_size;
    int count = 0;

    while (!is_queue_empty(queue)) {
        struct merkle_tree_node* current = dequeue(queue);

        if (current->is_leaf == 1) {
            buffer = malloc(current->size_in_file); // Allocate buffer according to size_in_file
            if (!buffer) {
                d_print("check_chunks_completed", "Memory allocation failed for buffer");
                break;
            }

            read_size = fread(buffer, 1, current->size_in_file, file);
            if (read_size == 0) {
                free(buffer);
                break; // Error reading
            }

            d_print("check_chunks_completed", "check_chunk_complete ready to call for node with expected hash %.*s", SHA256_HEXLEN, current->expected_hash);
            check_chunk_completed(current, buffer, read_size);
            d_print("check_chunks_completed", "check_chunk_complete called for node with expected hash %.*s", SHA256_HEXLEN, current->expected_hash);
            count += 1;

            free(buffer); // Free after reading
        }

        if (current->left != NULL) {
            enqueue(queue, current->left);
        }
        if (current->right != NULL) {
            enqueue(queue, current->right);
        }
    }

    d_print("check_chunks_completed", "The number of chunks are %d", count);
    free_queue(queue);
    fclose(file);
}

void check_tree_completed(struct merkle_tree_node* root){
    /**
     * From the leaves, compute the computed hash of the whole merkle tree
    */
    //d_print("check_tree_completed", "flag1 reached");
    if (!root) return;  // Basic error detection
    //d_print("check_tree_completed", "flag2 reached");

    // Return if leaves
    if (root->is_leaf == 1) return;
    //d_print("check_tree_completed", "flag3 reached");

    // Recursively compute hash for left and right
    if (root->left) {
        check_tree_completed(root->left);
    }
    if (root->right) {
        check_tree_completed(root->right);
    }

    // Space for storing the combined hash of the left and the right
    char combined_hashes[2 * SHA256_HEXLEN+1]; 

    if (root->left && root->right) {
        strncpy(combined_hashes, root->left->computed_hash, SHA256_HEXLEN);
        combined_hashes[SHA256_HEXLEN] = '\0';
        strncat(combined_hashes, root->right->computed_hash, SHA256_HEXLEN);
    } else if (root->left) {
        // If only left, not likely happen
        strncpy(combined_hashes, root->left->computed_hash, SHA256_HEXLEN);
    } else if (root->right) {
        // If only right, not likely happen
        strncpy(combined_hashes, root->right->computed_hash, SHA256_HEXLEN);
    } else {
        // No children, not likely happen
        return;
    }

    d_print("check_tree_completed", "For parent with expected hash %.*s, the combined hash is %.*s", SHA256_HEXLEN, root->expected_hash, 2*SHA256_HEXLEN, combined_hashes);
    check_chunk_completed(root, combined_hashes, strlen(combined_hashes));
}

struct bpkg_query bpkg_get_all_chunks_computed(struct bpkg_obj* bpkg){
    /**
     * Get all the computed hashes from the leaves
     * Used for self-testing
    */
   struct bpkg_query qry = { 0 };

    if (bpkg == NULL || bpkg->merkle_tree == NULL || bpkg->merkle_tree->root == NULL) {
        d_print("bpkg_get_all_hashes", "Invalid Merkle tree data.\n");
        return qry;  // Return empty query result if input is invalid
    }

    // Initialize queue for BFS
    struct Queue* queue = createQueue();
    enqueue(queue, bpkg->merkle_tree->root);

    // Temporary storage for collecting hashes
    qry.hashes = malloc((bpkg->nchunks) * sizeof(char*));
    if (qry.hashes == NULL) {
        d_print("bpkg_get_all_hashes", "Failed to allocate memory for hash array.");
        exit(EXIT_FAILURE);
    }
    size_t count = 0;

    while (!is_queue_empty(queue)) {
        struct merkle_tree_node* current = dequeue(queue);

         if (current->is_leaf == 1) {
            // Allocate memory for each hash and copy it
            qry.hashes[count] = malloc(SHA256_HEXLEN + 10);  // SHA256_HEX_LEN is a defined constant for hash size
            if (qry.hashes[count] == NULL) {
                d_print("bpkg_get_all_hashes", "Failed to allocate memory for a hash.");
                exit(EXIT_FAILURE);
            }
       
            //d_print("bpkg_get_all_chunks", "The expected hash (in leaf) in current loop is %s", current->computed_hash);
            strncpy(qry.hashes[count], current->computed_hash, SHA256_HEXLEN);
            count++;
        }

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

struct bpkg_query bpkg_get_all_hashes_computed(struct bpkg_obj* bpkg){
    /**
     * Get all computed hashes of all the nodes
     * Used for self-testing
    */
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
        strncpy(qry.hashes[count], current->computed_hash, SHA256_HEXLEN);
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

    check_chunks_completed(bpkg->merkle_tree->root, bpkg->filename, bpkg->nchunks);

    struct bpkg_query qry = { 0 };
    
    if (bpkg == NULL || bpkg->merkle_tree == NULL || bpkg->merkle_tree->root == NULL) {
        d_print("bpkg_get_completed_hashes", "Invalid Merkle tree data.\n");
        return qry;  // Return empty query result if input is invalid
    }

    // Initialize queue for BFS
    struct Queue* queue = createQueue();
    enqueue(queue, bpkg->merkle_tree->root);

    // Temporary storage for collecting hashes
    qry.hashes = malloc((bpkg->nchunks) * sizeof(char*));
    if (qry.hashes == NULL) {
        d_print("bpkg_get_completed_hashes", "Failed to allocate memory for hash array.");
        exit(EXIT_FAILURE);
    }
    size_t count = 0;

    while (!is_queue_empty(queue)) {
        struct merkle_tree_node* current = dequeue(queue);

        if (current->is_leaf == 1){
            d_print("bpkg_get_completed_hashes", "The expected hash for node in current loop is %.*s", SHA256_HEXLEN, current->expected_hash);
            d_print("bpkg_get_completed_hashes", "The computed hash for node in current loop is %.*s", SHA256_HEXLEN, current->computed_hash);
        } 

        if (current->is_leaf == 1 && strncmp(current->computed_hash, current->expected_hash, SHA256_HEXLEN) == 0) {
            // Allocate memory for each hash and copy it
            qry.hashes[count] = malloc(SHA256_HEXLEN + 10);  // SHA256_HEX_LEN is a defined constant for hash size
            if (qry.hashes[count] == NULL) {
                d_print("bpkg_get_completed_hashes", "Failed to allocate memory for a hash.");
                exit(EXIT_FAILURE);
            }
       
            //d_print("bpkg_get_completed_chunks", "The expected hash (in leaf) in current loop is %s", current->expected_hash);
            strncpy(qry.hashes[count], current->expected_hash, SHA256_HEXLEN);
            count++;
        }

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
 * Gets the mininum of hashes to represented the current completion state
 * Gets only the required/min hashes to represent the current completion state
 * Return the smallest set of hashes of completed branches to represent the completion state of the file.
 *
 * @param bpkg, constructed bpkg object
 * @return query_result, This structure will contain a list of hashes
 * 		and the number of hashes that have been retrieved
 */
struct bpkg_query bpkg_get_min_completed_hashes(struct bpkg_obj* bpkg) {
    check_chunks_completed(bpkg->merkle_tree->root, bpkg->filename, bpkg->nchunks);
	check_tree_completed(bpkg->merkle_tree->root);

    struct bpkg_query qry = {0};

    if (bpkg == NULL || bpkg->merkle_tree == NULL || bpkg->merkle_tree->root == NULL) {
        d_print("bpkg_get_min_completed_hashes", "Invalid input data.");
        return qry;  // Return an empty result if input is invalid
    }

    // Initialize the queue for BFS
    struct Queue* queue = createQueue();
    enqueue(queue, bpkg->merkle_tree->root);
    qry.hashes = malloc((bpkg->nchunks + bpkg->nhashes) * sizeof(char*)); // Assume the worst case
    if (qry.hashes == NULL) {
        d_print("bpkg_get_min_completed_hashes", "Failed to allocate memory for hash array.");
        exit(EXIT_FAILURE);
    }

    size_t count = 0;

    while (!is_queue_empty(queue)) {
        struct merkle_tree_node* current = dequeue(queue);

        d_print("bpkg_get_min_completed_hashes", "For node in current loop, the computed hash is %.*s", SHA256_HEXLEN, current->computed_hash);
        
        if (strncmp(current->computed_hash, current->expected_hash, SHA256_HEXLEN) == 0) {
            // If computed hash matches the expected hash, this subtree is complete and correct
            qry.hashes[count] = malloc(SHA256_HEXLEN);
            if (qry.hashes[count] == NULL) {
                d_print("bpkg_get_min_completed_hashes", "Failed to allocate memory for a hash.");
                exit(EXIT_FAILURE);
            }
            strncpy(qry.hashes[count], current->computed_hash, SHA256_HEXLEN);
            count++;
            // Do not enqueue children because this subtree is verified
        } else {
            // If hashes do not match, enqueue children to check them
            if (current->left != NULL) {
                enqueue(queue, current->left);
            }
            if (current->right != NULL) {
                enqueue(queue, current->right);
            }
        }
    }

    qry.len = count;  // Set the number of valid hashes
    free_queue(queue);  // Clean up the queue

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
struct bpkg_query bpkg_get_all_chunk_hashes_from_hash(struct bpkg_obj* bpkg, char* hash) {
    struct bpkg_query qry = { 0 };
    if (bpkg == NULL || bpkg->merkle_tree == NULL || hash == NULL) {
        d_print("bpkg_get_all_chunk_hashes_from_hash", "Invalid input data.\n");
        return qry;  // Return empty query result if input is invalid
    }

    // Find the node with the given hash
    struct merkle_tree_node* root = bpkg->merkle_tree->root;
    struct Queue* queue = createQueue();
    enqueue(queue, (void*)root);
    struct merkle_tree_node* target_node = NULL;

    while (!is_queue_empty(queue) && target_node == NULL) {
        struct merkle_tree_node* current = (struct merkle_tree_node*)dequeue(queue);
        d_print("bpkg_get_all_chunk_hashes_from_hash", "the current expected hash is %.*s", SHA256_HEXLEN, current->expected_hash);
        d_print("bpkg_get_all_chunk_hashes_from_hash", "the hash wanted is  %.*s", SHA256_HEXLEN, hash);
        if (strncmp(current->expected_hash, hash, SHA256_HEXLEN) == 0) {
            target_node = current;
            break;
        }
        if (current->left != NULL) {
            enqueue(queue, current->left);
        }
        if (current->right != NULL) {
            enqueue(queue, current->right);
        }
    }

    free_queue(queue);

    if (target_node == NULL) {
        d_print("bpkg_get_all_chunk_hashes_from_hash", "No node found with the given hash.\n");
        return qry;
    }

    // Collect all hashes from the target node's subtree
    queue = createQueue();
    enqueue(queue, target_node);
    qry.hashes = malloc((bpkg->nchunks + bpkg->nhashes) * sizeof(char*));  // Assume enough space for all hashes
    if (qry.hashes == NULL) {
        d_print("bpkg_get_all_chunk_hashes_from_hash", "Failed to allocate memory for hash array.");
        exit(EXIT_FAILURE);
    }
    size_t count = 0;

    while (!is_queue_empty(queue)) {
        struct merkle_tree_node* current = (struct merkle_tree_node*)dequeue(queue);
        if (current->is_leaf == 1) {
            qry.hashes[count] = malloc(SHA256_HEXLEN + 1);  // Allocate space for each hash
            if (qry.hashes[count] == NULL) {
                d_print("bpkg_get_all_chunk_hashes_from_hash", "Failed to allocate memory for a hash.");
                exit(EXIT_FAILURE);
            }
            d_print("bpkg_get_all_chunk_hashes_from_hash", "The expected hash (in leaf) in current loop is %.*s", SHA256_HEXLEN, current->expected_hash);
            strncpy(qry.hashes[count], current->expected_hash, SHA256_HEXLEN);
            count++;
        }
        if (current->left != NULL) {
            enqueue(queue, current->left);
        }
        if (current->right != NULL) {
            enqueue(queue, current->right);
        }
    }

    qry.len = count;
    free_queue(queue);

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
