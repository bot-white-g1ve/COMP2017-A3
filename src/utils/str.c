#include <stdio.h>
#include <string.h>
<<<<<<< HEAD
=======
#include <ctype.h>
>>>>>>> 5e5cc2a (finish constructing non-leaf nodes)

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
<<<<<<< HEAD
=======
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
>>>>>>> 5e5cc2a (finish constructing non-leaf nodes)
}