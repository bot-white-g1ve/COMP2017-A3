#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

struct config_obj* load_config(char* config_file_path){
    /**
     * Load config
     * Perform field checks, folder creations
     * Input: config file path
     * Output: a config object containing all the information
    */
    struct config_obj* config_object = malloc(sizeof(struct config_obj));
    if (!config_object) {
        d_print("load_config", "Memory allocation failed for config_object");
        return NULL;
    }

    FILE* config_file = fopen(config_file_path, "r");
    if (config_file == NULL) {
        d_print("load_config", "Fail opening config_file_path");
        free(config_object);
        return NULL;
    }

    char file_line[PATH_MAX_LEN+32];
    while (fgets(file_line, sizeof(file_line), file_line)) {
        char key[32];
        if (sscanf(file_line, "%s", key) == 1) {
            if (strcmp(key, "directory:") == 0) {
                sscanf(file_line, "directory:%s", config_object->directory);
            } else if (strcmp(key, "max_peers:") == 0) {
                sscanf(file_line, "max_peers:%d", &config_object->max_peers);
            } else if (strcmp(key, "port:") == 0) {
                sscanf(file_line, "port:%hu", &config_object->port);
            }
        }
    }
}

void config_directory_check(char* directory){
    /**
     * Check the field 'directory' in config file and perform related operations
    */
   struct stat statbuf;

    // Check if path exists
    if (stat(directory, &statbuf) == 0) {
        // Check if it is a directory
        if (!S_ISDIR(statbuf.st_mode)) {
            fprintf(stderr, "The path %s is not a directory.\n", directory);
            exit(3);
        }
    } else {
        // Attempt creating this directory
        if (mkdir(directory, 0755) != 0) {
            perror("Failed to create directory");
            exit(3);
        }
    }
}