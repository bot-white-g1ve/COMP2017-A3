#ifndef STR_H
#define STR_H

struct split_on_comma_return {
    char* hash;
    uint32_t offset;
    uint32_t size;
};

extern void delete_newline_in_the_end(char* str);
extern void split_on_first_colon(const char *input, char *left, char *right);
extern void delete_whitespace_in_the_front(char* str);
extern struct split_on_comma_return split_on_comma(const char* str);
extern void add_edd_null(char* str, size_t size);
extern char* concat_file_path(const char* directory, const char* filename);

#endif
