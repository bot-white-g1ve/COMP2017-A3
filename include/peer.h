#ifndef PEER_H
#define PEER_H

#define PACKET_LEN (4096)
#define PEER_STR_LEN (22)

extern void *server_thread(void* arg);
extern void *client_thread(void* arg);
extern int is_peer_exist(const char* peer);
extern void print_peer_list();

extern volatile int quit_signal;

#endif
