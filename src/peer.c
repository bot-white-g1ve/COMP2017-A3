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
#include <tree/merkletree.h>
#include <globals.h>
#include <utils/linked_list.h>
#include <utils/str.h>

volatile int quit_signal = 0;
Peer peer_list[MAX_PEERS] = {0};
int peer_count = 0;
pthread_mutex_t peer_list_mutex = PTHREAD_MUTEX_INITIALIZER;

void add_peer(const char* ip, int port, int sock);

void* tiny_server_thread(void* arg){
    int new_socket = *(int*)arg;
    free(arg); // Free the dynamically allocated memory

    struct sockaddr_in client_address;
    socklen_t client_addrlen = sizeof(client_address);
    struct btide_packet buffer;

    if (getpeername(new_socket, (struct sockaddr *)&client_address, &client_addrlen) < 0) {
        d_error("tiny_server_thread", "Getpeername failed");
        close(new_socket);
        pthread_exit(NULL);
    }

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_address.sin_addr, client_ip, INET_ADDRSTRLEN);
    int client_port = ntohs(client_address.sin_port);

    fd_set readfds;
    while (!quit_signal) {
        FD_ZERO(&readfds);
        FD_SET(new_socket, &readfds);

        struct timeval timeout;
        timeout.tv_sec = 1;  // Timeout of 1 second
        timeout.tv_usec = 0;

        int activity = select(new_socket + 1, &readfds, NULL, NULL, &timeout);
        if (activity < 0 && errno != EINTR) {
            d_error("tiny_server_thread", "select error");
            break;
        }

        if (activity > 0 && FD_ISSET(new_socket, &readfds)) {
            int valread = read(new_socket, &buffer, PACKET_LEN);
            if (valread == 0) {
                d_print("tiny_server_thread", "Read zero");
                continue;
            } else if (valread < 0) {
                d_error("tiny_server_thread", "Read error");
                break;
            }

            d_print("tiny_server_thread", "Received packet with msg_code: %hu", buffer.msg_code);

            // Process the packet
            if (0x03 == buffer.msg_code) {
                remove_peer(client_ip, client_port);
                break;
            } else if (0xFF == buffer.msg_code) {
                struct btide_packet pong_packet = create_small_packet(0x00);
                send(new_socket, &pong_packet, sizeof(struct btide_packet), 0);
            } else if (0x06 == buffer.msg_code) { // Handle REQ packet
                struct req_packet_data req_data = parse_req_packet(&buffer);
                struct PackageNode* target_package = get_package(req_data.identifier);
                if (target_package != NULL) {
                    struct btide_packet res_packet = create_res_packet(
                        target_package->package, 
                        req_data.chunk_hash, 
                        req_data.offset, 
                        req_data.data_len, 
                        directory
                    );
                    if (res_packet.error == 0) {
                        send(new_socket, &res_packet, PACKET_LEN, 0);
                    } else {
                        d_error("tiny_server_thread", "Failed to create RES packet");
                    }
                } else {
                    d_error("tiny_server_thread", "Package not managed");
                }
            }
        }
    }

    close(new_socket);
    d_print("tiny_server_thread", "Client handler quits");
    pthread_exit(NULL);
}

void *server_thread(void* arg){
    /**
     * Keep listening on port
    */
    int port = *(int*)arg;
    int server_fd;
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
            int* new_socket = malloc(sizeof(int)); // Dynamically allocate memory for new socket
            if ((*new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
                d_error("server_thread","Accept failed");
                free(new_socket);
                continue; // TODO: If error
            }

            // Get client address
            struct sockaddr_in client_address;
            socklen_t client_addrlen = sizeof(client_address);
            if (getpeername(*new_socket, (struct sockaddr *)&client_address, &client_addrlen) < 0) {
                d_error("server_thread", "Getpeername failed");
                close(*new_socket);
                free(new_socket);
                continue;
            }

            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_address.sin_addr, client_ip, INET_ADDRSTRLEN);
            int client_port = ntohs(client_address.sin_port);

            // Read
            // memset(buffer, 0, PACKET_LEN);
            // read(new_socket, buffer, PACKET_LEN);
            // d_print("server_thread", "Received %s", buffer);

            struct btide_packet ACP_packet = create_small_packet(0x02);
            send(*new_socket, &ACP_packet, sizeof(struct btide_packet), 0);
            d_print("server_thread", "Send packet with msg_code %hu", ACP_packet.msg_code);

            int valread = read(*new_socket, &buffer, PACKET_LEN);
            d_print("server_thread", "Valread read is %d", valread);
            d_print("server_thread", "Client response package with msg_code: %hu", buffer.msg_code);

            if (0x0c == buffer.msg_code){
                add_peer(client_ip, client_port, *new_socket);
                pthread_t tiny_thread;
                if (pthread_create(&tiny_thread, NULL, tiny_server_thread, new_socket) != 0) {
                    d_error("server_thread", "Failed to create thread");
                    close(*new_socket);
                    free(new_socket);
                }
                pthread_detach(tiny_thread); // Detach the thread so that it cleans up after itself
            } else {
                close(*new_socket);
                free(new_socket);
            }
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
    read(sock, &buffer, PACKET_LEN);
    d_print("client thread", "Server response package with msg_code: %hu", buffer.msg_code);

    if (0x02 == buffer.msg_code){
        //Add ip:port to peer_list
        add_peer(server_ip, port, sock);

        struct btide_packet ACK_packet = create_small_packet(0x0c);
        send(sock, &ACK_packet, sizeof(struct btide_packet), 0);
        d_print("client_thread", "Send packet with msg_code %hu", ACK_packet.msg_code);

        printf("Connection established with peer\n");
    } else {
        close(sock);
    }

    free(((char**)arg)[0]);
    free(((char**)arg)[1]);
    free(arg);
    pthread_exit(NULL);
}

void client_socket_disconnect(int sock) {
    struct btide_packet DSN_packet = create_small_packet(0x03);
    if (send(sock, &DSN_packet, sizeof(struct btide_packet), 0) < 0) {
        d_error("client_socket_disconnect", "Send error");
    } else {
        d_print("client_socket_disconnect", "Send packet with msg_code %hu", DSN_packet.msg_code);
    }

    // Close the socket
    close(sock);
}

void client_socket_fetch(int sock, struct merkle_tree_node* target_chunk, const char* identifier, const char* hash) {
    // Create REQ packet
    struct btide_packet req_packet = create_req_packet(identifier, hash, target_chunk->offset_in_file, target_chunk->size_in_file);

    // Send REQ packet
    if (send(sock, &req_packet, PACKET_LEN, 0) < 0) {
        d_error("client_socket_fetch","Send failed");
        close(sock);
        return;
    }

    d_print("client_socket_fetch", "REQ packet sent to peer\n");

    // Receive RES packet
    struct btide_packet res_packet;
    ssize_t bytes_received = recv(sock, &res_packet, PACKET_LEN, 0);
    if (bytes_received < 0) {
        d_error("client_socket_fetch","Receive failed");
        close(sock);
        return;
    } else if (bytes_received == 0) {
        d_error("client_socket_fetch","Peer closed the connection\n");
        close(sock);
        return;
    }

    // Handle the received RES packet
    if (res_packet.msg_code == 0x07) { // Assuming 0x07 is the code for RES
        // Extract offset, data length, and data from the received packet
        uint32_t offset;
        uint16_t data_len;
        memcpy(&offset, res_packet.pl.data, sizeof(uint32_t));
        memcpy(&data_len, res_packet.pl.data + sizeof(uint32_t) + MAX_DATA_LEN, sizeof(uint16_t));
        uint8_t* data = res_packet.pl.data + sizeof(uint32_t);

        // Get the package object using the identifier
        struct PackageNode* packageNode = get_package(identifier);
        if (packageNode == NULL) {
            d_error("client_socket_fetch", "Package not found\n");
            return;
        }
        struct bpkg_obj* package = packageNode->package;
        // Construct the file path
        char* file_path = concat_file_path(directory, package->filename);

        // Write the received data to the file
        if (write_data_to_file(file_path, offset, (char*)data, data_len) < 0) {
            d_error("client_socket_fetch", "Failed to write data to file\n");
        } else {
            d_print("client_socket_fetch", "Data written to file successfully\n");
        }

        free(file_path);
    } else {
        d_error("client_socket_fetch","Unexpected packet received\n");
    }
}

void ping_peers() {
    pthread_mutex_lock(&peer_list_mutex);

    for (int i = 0; i < peer_count; i++) {
        int sock = peer_list[i].socket;
        struct btide_packet ping_packet = create_small_packet(0xFF);

        if (send(sock, &ping_packet, sizeof(struct btide_packet), 0) < 0) {
            d_error("ping_peers", "Send error to peer: %s", peer_list[i].address);
            continue;
        } else {
            d_print("ping_peers", "Sent ping packet to peer: %s", peer_list[i].address);
        }

        struct btide_packet response_packet;
        int valread = read(sock, &response_packet, PACKET_LEN);

        if (valread > 0 && response_packet.msg_code == 0x00) {
            d_print("ping_peers", "Received response from peer: %s", peer_list[i].address);
        } else {
            d_error("ping_peers", "No valid response from peer: %s", peer_list[i].address);
        }
    }

    pthread_mutex_unlock(&peer_list_mutex);
}

void add_peer(const char* ip, int port, int sock) {
    pthread_mutex_lock(&peer_list_mutex);

    if (peer_count < MAX_PEERS) {
        snprintf(peer_list[peer_count].address, PEER_STR_LEN, "%s:%d", ip, port);
        peer_list[peer_count].socket = sock;
        peer_count++;
        d_print("add_peer", "Added peer: %s:%d", ip, port);
    } else {
        d_error("add_peer", "Peer list is full");
    }

    pthread_mutex_unlock(&peer_list_mutex);
}


int is_peer_exist(const char* peer) {
    pthread_mutex_lock(&peer_list_mutex);

    for (int i = 0; i < peer_count; i++) {
        if (strcmp(peer_list[i].address, peer) == 0) {
            pthread_mutex_unlock(&peer_list_mutex);
            return 1;  // Exist
        }
    }

    pthread_mutex_unlock(&peer_list_mutex);
    return 0;  // Not exist
}


void print_peer_list() {
    pthread_mutex_lock(&peer_list_mutex);

    if (peer_count == 0) {
        printf("Not connected to any peers\n");
    } else {
        printf("Connected to:\n\n");
        for (int i = 0; i < peer_count; i++) {
            printf("%d. %s\n", i + 1, peer_list[i].address);
        }
    }

    pthread_mutex_unlock(&peer_list_mutex);
}


void remove_peer(const char* ip, int port) {
    char peer[PEER_STR_LEN];
    snprintf(peer, PEER_STR_LEN, "%s:%d", ip, port);
    d_print("remove_peer", "the peer is %s", peer);

    pthread_mutex_lock(&peer_list_mutex);

    int found_index = -1;
    for (int i = 0; i < peer_count; i++) {
        if (strcmp(peer_list[i].address, peer) == 0) {
            d_print("remove_peer", "find target in index %d", i);
            found_index = i;
            break;
        }
    }

    if (found_index != -1) {
        // Close the socket before removing the peer
        // close(peer_list[found_index].socket);
        // Move elements forward
        for (int i = found_index; i < peer_count - 1; i++) {
            peer_list[i] = peer_list[i + 1];
        }
        peer_count--;
        d_print("remove_peer", "Removed peer: %s", peer);
    } else {
        d_print("remove_peer", "Peer not found: %s", peer);
    }

    pthread_mutex_unlock(&peer_list_mutex);
}

int get_peer(const char* ip, int port) {
    char peer[PEER_STR_LEN];
    snprintf(peer, PEER_STR_LEN, "%s:%d", ip, port);
    d_print("get_peer", "Looking for peer: %s", peer);

    pthread_mutex_lock(&peer_list_mutex);

    for (int i = 0; i < peer_count; i++) {
        if (strcmp(peer_list[i].address, peer) == 0) {
            int sock = peer_list[i].socket;
            pthread_mutex_unlock(&peer_list_mutex);
            d_print("get_peer", "Found peer: %s with socket %d", peer, sock);
            return sock;
        }
    }

    pthread_mutex_unlock(&peer_list_mutex);
    d_print("get_peer", "Peer not found: %s", peer);
    return -1;  // Not found
}

