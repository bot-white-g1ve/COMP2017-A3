#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <peer.h>
#include <debug/debug.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <net/package.h>
#include <sys/select.h>

volatile int quit_signal = 0;
char peer_list[2048][PEER_STR_LEN] = {0};
int peer_count = 0;
pthread_mutex_t peer_list_mutex = PTHREAD_MUTEX_INITIALIZER;

void add_peer(const char* ip, int port);

void *server_thread(void* arg){
    /**
     * Keep listening on port
    */
    int port = *(int*)arg;
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    struct btide_packet buffer;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        d_error("server_thread", "server socket failed");
        pthread_exit(NULL);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // int opt = 1;
    // if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
    //     d_error("server_thread", "setsockopt failed");
    //     pthread_exit(NULL);
    // }

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        d_error("server_thread", "bind failed");
        pthread_exit(NULL);
    }

    // Set socket
    if (listen(server_fd, 4) < 0) {
        d_error("server_thread", "listening launch failed");
        pthread_exit(NULL);
    }

    // Set server_fd to non-blocking
    // fcntl(server_fd, F_SETFL, O_NONBLOCK);

    fd_set readfds;

    d_print("server_thread", "the quit_signal is %d", quit_signal);
    while (!quit_signal){
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);

        struct timeval timeout;
        timeout.tv_sec = 1;  // Timeout of 1 second
        timeout.tv_usec = 0;

        int activity = select(server_fd + 1, &readfds, NULL, NULL, &timeout);
        if (activity < 0 && errno != EINTR) {
            d_error("server_thread", "select error");
        }

        if (activity > 0 && FD_ISSET(server_fd, &readfds)) {
            // Accept
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
                d_error("server_thread","Accept failed");
                continue; // TODO: If error
            }

            // Read
            // memset(buffer, 0, PACKET_LEN);
            // read(new_socket, buffer, PACKET_LEN);
            // d_print("server_thread", "Received %s", buffer);

            struct btide_packet ACP_packet = create_small_packet(0x02);
            send(new_socket, &ACP_packet, sizeof(struct btide_packet), 0);
            d_print("server_thread", "Send packet with msg_code %hu", ACP_packet.msg_code);

            int valread = read(new_socket, &buffer, PACKET_LEN);
            d_print("server_thread", "Valread read is %d", valread);
            d_print("server_thread", "Client response package with msg_code: %hu", buffer.msg_code);

            // Close current socket
            close(new_socket);
        }
    }

    // Close server socket
    close(server_fd);
    d_print("server_thread", "server thread quits");
    pthread_exit(NULL);
}

void *client_thread(void* arg) {
    char* server_ip = ((char**)arg)[0];
    int port = atoi(((char**)arg)[1]);
    int sock = 0;
    struct sockaddr_in serv_addr;
    struct btide_packet buffer;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        d_error("client_thread", "Socket creation error");
        free(((char**)arg)[0]);
        free(((char**)arg)[1]);
        free(arg);
        pthread_exit(NULL);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        d_error("client_thread", "Invalid address / Address not supported");
        close(sock);
        free(((char**)arg)[0]);
        free(((char**)arg)[1]);
        free(arg);
        pthread_exit(NULL);
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        d_error("client_thread", "Connection failed");
        printf("Unable to connect to request peer\n");
        close(sock);
        free(((char**)arg)[0]);
        free(((char**)arg)[1]);
        free(arg);
        pthread_exit(NULL);
    }

    // Send a message to the server
    // const char* hello = "Hello from client";
    // send(sock, hello, strlen(hello), 0);
    // d_print("Client thread", "Message sent: %s", hello);

    // Read server's response
    int valread = read(sock, &buffer, PACKET_LEN);
    d_print("client thread", "Server response package with msg_code: %hu", buffer.msg_code);

    if (0x02 == buffer.msg_code){
        //Add ip:port to peer_list
        add_peer(server_ip, port);

        struct btide_packet ACK_packet = create_small_packet(0x0c);
        send(sock, &ACK_packet, sizeof(struct btide_packet), 0);
        d_print("client_thread", "Send packet with msg_code %hu", ACK_packet.msg_code);

        printf("Connection established with peer\n");
    }

    // Close the socket
    close(sock);
    free(((char**)arg)[0]);
    free(((char**)arg)[1]);
    free(arg);
    pthread_exit(NULL);
}

void add_peer(const char* ip, int port) {
    pthread_mutex_lock(&peer_list_mutex);

    snprintf(peer_list[peer_count], PEER_STR_LEN, "%s:%d", ip, port);
    peer_count++;
    d_print("add_peer", "Added peer: %s:%d", ip, port);

    pthread_mutex_unlock(&peer_list_mutex);
}

int is_peer_exist(const char* peer) {

    pthread_mutex_lock(&peer_list_mutex);

    for (int i = 0; i < peer_count; i++) {
        if (strcmp(peer_list[i], peer) == 0) {
            pthread_mutex_unlock(&peer_list_mutex); 
            return 1;  // Exist
        }
    }

    pthread_mutex_unlock(&peer_list_mutex);
    return 0;  // Not exist
}

void print_peer_list() {
    pthread_mutex_lock(&peer_list_mutex);

    if (peer_count == 0){
        printf("Not connected to any peers\n");
        pthread_mutex_unlock(&peer_list_mutex);
        return;
    }

    printf("Connected to:\n\n");
    for (int i = 0; i < peer_count; i++) {
        printf("%d. %s\n", i + 1, peer_list[i]);
    }

    pthread_mutex_unlock(&peer_list_mutex);
}

void remove_peer(const char* ip, const char* port_str) {
    char peer[PEER_STR_LEN];
    snprintf(peer, PEER_STR_LEN, "%s:%s", ip, port_str);
    d_print("remove_peer", "the peer is %s", peer);

    pthread_mutex_lock(&peer_list_mutex);

    int found_index = -1;
    for (int i = 0; i < peer_count; i++) {
        if (strcmp(peer_list[i], peer) == 0) {
            d_print("remove_peer", "find target in index %d", i);
            found_index = i;
            break;
        }
    }

    if (found_index != -1) {
        // Move elements forward
        for (int i = found_index; i < peer_count - 1; i++) {
            strcpy(peer_list[i], peer_list[i + 1]);
        }
        peer_count--;
        d_print("remove_peer", "Removed peer: %s", peer);
    } else {
        d_print("remove_peer", "Peer not found: %s", peer);
    }

    pthread_mutex_unlock(&peer_list_mutex);
}

