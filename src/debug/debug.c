#include <stdio.h>
#include <stdarg.h>

void d_print(const char* func, const char* format, ...) {
    printf("(In %s) ", func);
    
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("\n");
}