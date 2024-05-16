#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <utils/str.h>
#include <config.h>

void delete_newline_in_the_end(char* str){
    /** 
     * Delete the \n symbol in the end of the string
     * No need for returning
    */
    int len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0'; 
    }
}

void split_on_first_colon(const char *input, char *left, char *right){
    /**
     * Split the input through the first :
     * Assign the splited contents to left and right
     * If : is not find, left and right will be empty
    */
    char *colon = strchr(input, ':'); // find :
    if (colon != NULL) {
        size_t left_length = colon - input;
        // Copy the left content to left
        strncpy(left, input, left_length); 
        left[left_length] = '\0';

        // Copy the right content to right
        strcpy(right, colon + 1);
    } else {
        // If : is not found
        left[0] = '\0';
        right[0] = '\0';
    }
}

void delete_whitespace_in_the_front(char* str){
    /**
     * Delete all whitespaces in the front of str
     * No need for returning
    */
   int i = 0, j = 0;
    while (str[i] != '\0' && (isspace(str[i]))) {
        i++;
    }
    while (str[i] != '\0') {
        str[j++] = str[i++];
    }
    str[j] = '\0';
}

struct split_on_comma_return split_on_comma(const char* str){
    /**
     * Split the string according to comma
     * data.hash needed to be freed after calling
    */
    struct split_on_comma_return result;
    char buffer[256];
    strcpy(buffer, str); // Copy the input into the buffer

    char* token = strtok(buffer, ",");
    if (token == NULL) {
        fprintf(stderr, "Error parsing input: Cannot find hash.\n");
        exit(EXIT_FAILURE);
    }
    result.hash = strdup(token); // Malloc and copy

    token = strtok(NULL, ",");
    if (token == NULL) {
        fprintf(stderr, "Error parsing input: Cannot find offset.\n");
        free(result.hash);
        exit(EXIT_FAILURE);
    }
    result.offset = strtoul(token, NULL, 10); // transform to uint32_t

    token = strtok(NULL, ",");
    if (token == NULL) {
        fprintf(stderr, "Error parsing input: Cannot find length.\n");
        free(result.hash);
        exit(EXIT_FAILURE);
    }
    result.size = strtoul(token, NULL, 10); // transform to uint32_t

    return result;
}

// Function to create and return a concatenated file path
char* concat_file_path(const char* directory, const char* filename) {
    static char file_path[PATH_MAX_LEN + 32]; // Adjust the size as needed

    // Check if the directory ends with '/'
    if (directory[strlen(directory) - 1] == '/') {
        snprintf(file_path, sizeof(file_path), "%s%s", directory, filename);
    } else {
        snprintf(file_path, sizeof(file_path), "%s/%s", directory, filename);
    }

    return file_path;
}

void delete_edd_null(char* str, size_t size){
    /**
     * Delete \0 in the end of the string
     * Unfinished
    */
    ;
}
