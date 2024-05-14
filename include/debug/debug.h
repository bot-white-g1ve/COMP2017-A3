#ifndef DEBUG_H
#define DEBUG_H

extern void d_init(int p);
extern void d_print(const char* func, const char* format, ...);
extern int contains_null(const char *str, size_t len);
extern void d_error(const char* func, const char* format, ...);

#endif