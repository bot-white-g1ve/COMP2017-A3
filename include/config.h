#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

#define PATH_MAX_LEN (1024)

struct config_obj{
    char directory[PATH_MAX_LEN];
    int max_peers;
    uint16_t port;
};

extern struct config_obj* load_config(char* config_file_path);
extern void config_directory_check(char* directory);
extern void config_max_peer_check(int max_peers);
extern void config_port_check(uint16_t port);

#endif