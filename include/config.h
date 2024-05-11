#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

#define PATH_MAX_LEN (1024)

struct config_obj{
    char directory[PATH_MAX_LEN];
    int max_peers;
    uint16_t port;
};

#endif