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

void *server_thread(int port){
    /**
     * Keep listening on port
    */
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[PACKET_LEN] = {0};

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        d_error("server_thread", "server socket failed");
        pthread_exit(NULL);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        d_error("server_thread", "setsockopt failed");
        pthread_exit(NULL);
    }

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        d_error("server_thread", "bind failed");
        pthread_exit(NULL);
    }

    // Set socket
    if (listen(server_fd, 4) < 0) {
        d_error("server_thread", "listening launch failed");
        pthread_exit(NULL);
    }

    while (1){
        d_print("server_thread", "Waiting for connections ...");

        // Accept
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
            d_error("server_thread","Accept failed");
            continue; // TODO: If error
        }

        // Read
        memset(buffer, 0, PACKET_LEN);
        read(new_socket, buffer, PACKET_LEN);
        d_print("server_thread", "Received %s\n", buffer);

        // Close current socket
        close(new_socket);
    }

    // Close server socket
    close(server_fd);
    pthread_exit(NULL);
}