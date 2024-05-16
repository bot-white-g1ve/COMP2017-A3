#ifndef PEER_H
#define PEER_H

#define PACKET_LEN (4096)
#define PEER_STR_LEN (22)
#define MAX_PEERS (2048)

typedef struct {
    char address[PEER_STR_LEN];
    int socket;
} Peer;

extern void *server_thread(void* arg);
extern void *client_thread(void* arg);
extern void client_socket_disconnect(int sock);
extern void ping_peers();
extern int is_peer_exist(const char* peer);
extern void print_peer_list();
extern void remove_peer(const char* ip, int port);
extern int get_peer(const char* ip, int port);

extern volatile int quit_signal;

#endif
