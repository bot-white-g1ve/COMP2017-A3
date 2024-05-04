#include <stdio.h>
#include <stdarg.h>

void d_print(const char* func, const char* format, ...) {
    // Open file, if not exist, create.
    FILE *fp = fopen("debug.txt", "a");
    if (fp == NULL) {
        perror("Failed to create file");
        return;
    }

    // print func name
    fprintf(fp, "(In %s) ", func);

    // print content
    va_list args;
    va_start(args, format);
    vfprintf(fp, format, args);
    va_end(args);

    // print new line
    fprintf(fp, "\n");
    fclose(fp);
}

int contains_null(const char *str, size_t len){
    /**
     * Check if the string contains a null character
    */
    for (size_t i = 0; i < len; i++){
        if (str[i] == '\0') return 1;
    }
    return 0; // If not found
}