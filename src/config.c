#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <debug/debug.h>
#include <stdbool.h>
#include <string.h>

void config_directory_check(char* directory);
void config_max_peer_check(int max_peers);
void config_port_check(uint16_t port);

struct config_obj* load_config(char* config_file_path){
    /**
     * Load config
     * Perform field checks, folder creations
     * Input: config file path
     * Output: a config object containing all the information
    */
    struct config_obj* config_object = malloc(sizeof(struct config_obj));
    if (!config_object) {
        d_error("load_config", "Memory allocation failed for config_object");
        return NULL;
    }

    bool directory_set = false;
    bool max_peers_set = false;
    bool port_set = false;

    FILE* config_file = fopen(config_file_path, "r");
    if (config_file == NULL) {
        d_error("load_config", "Fail opening config_file_path");
        free(config_object);
        return NULL;
    }

    char file_line[PATH_MAX_LEN+32];
    while (fgets(file_line, sizeof(file_line), config_file)) {
        // Trim newline character
        file_line[strcspn(file_line, "\n")] = 0;
        d_print("load_config", "the file_line is %s", file_line);

        if (sscanf(file_line, "directory:%1023s", config_object->directory) == 1) {
            d_print("load_config", "directory field read");
            config_directory_check(config_object->directory);
            directory_set = true;
        } else if (sscanf(file_line, "max_peers:%d", &config_object->max_peers) == 1) {
            d_print("load_config", "max_peers field read");
            config_max_peer_check(config_object->max_peers);
            max_peers_set = true;
        } else if (sscanf(file_line, "port:%hu", &config_object->port) == 1) {
            d_print("load_config", "port field read");
            config_port_check(config_object->port);
            port_set = true;
        }
    }

    fclose(config_file);

    if (!directory_set || !max_peers_set || !port_set) {
        d_error("load_config", "Configuration file is missing required fields.");
        free(config_object);
        return NULL;
    }

    return config_object;
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
            d_error("config_directory_check", "The path %s is not a directory.\n", directory);
            exit(3);
        }
    } else {
        // Attempt creating this directory
        if (mkdir(directory, 0755) != 0) {
            d_error("config_directory_check", "Failed to create directory");
            exit(3);
        }
    }
}

void config_max_peer_check(int max_peers) {
    if (max_peers < 1 || max_peers > 2048) {
        d_error("config_max_peer_check", "Invalid max_peers value: %d. Must be between 1 and 2048.\n", max_peers);
        exit(4);
    }
}

void config_port_check(uint16_t port) {
    if (port <= 1024 || port > 65535) {
        d_error("config_port_check", "Invalid port value: %d. Must be between 1024 (not included) and 65535.\n", port);
        exit(5);
    }
}