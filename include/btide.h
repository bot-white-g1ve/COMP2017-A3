#ifndef BTIDE_H
#define BTIDE_H

#include <chk/pkgchk.h>

#define MAX_CLI_INPUT 5520

struct package {
    char identifier[33];
    char filename[FILENAME_LEN];
    char complete[11];
};

extern void* cli_thread(void* arg);

#endif
