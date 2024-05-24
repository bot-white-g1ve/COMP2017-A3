#include <stdio.h>
#include <stdlib.h>
#include <bpkg.h>
#include <config.h>
#include <debug/debug.h>
#include <btide.h>
#include <peer.h>
#include <pthread.h>
#include <string.h>
#include <utils/linked_list.h>
#include <utils/str.h>

//
// PART 2
//
char directory[PATH_MAX_LEN] = {0};

int process_command(char* command);

int main(int argc, char** argv) {

    if (argc < 2) {  // Check if argv is given
        d_error("main", "No args given");
        exit(-1);
    }

    struct config_obj* config = load_config(argv[1]);
    if (config == NULL) {
        d_error("main", "Failed to load configuration.");
        exit(-1);
    }

    d_init(config->port);
    
    d_print("main", "Config load successfully");
    d_print("main", "Directory: %s", config->directory);
    d_print("main", "Max Peers: %d", config->max_peers);
    d_print("main", "Port: %u", config->port);
    strcpy(directory, config->directory);

    pthread_t thread1, thread2;

    if (pthread_create(&thread1, NULL, server_thread, (void*)&config->port) != 0){
        d_error("main", "Failed to create server thread");
        exit(-1);
    }

    if (pthread_create(&thread2, NULL, cli_thread, NULL) != 0) {
        d_error("main", "Failed to create cli thread");
        exit(-1);
    }

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    // free everyhing
    free(config);
}

void* cli_thread(void* arg){
    char input[MAX_CLI_INPUT];
    int return_code;

    //printf("Welcome to btide, enter your commands below\n");

    while (1){
        // Read from standard input
        if (fgets(input, MAX_CLI_INPUT, stdin) == NULL) {
            continue; // If fail, read next line
        }

        // Remove \n
        input[strcspn(input, "\n")] = 0;

        d_print("cli_thread", "user inputs %s", input);

        return_code = process_command(input);
        d_print("cli_thread", "the return code is %d", return_code);

        if (return_code == 0){
            printf("Invalid Input.\n");
        } else if (return_code == -1){
            free_packages();
            break;
        }
    }

    pthread_exit(NULL); 
}

int process_command(char* command){
    if (command == NULL || strlen(command) == 0) {
        return 0;
    }

    char* token;

    token = strtok(command, " ");
    if (token == NULL) {
        return 0;
    }

    else if (strcmp(token, "QUIT") == 0){
        quit_signal = 1;
        return -1;
    }

    else if (strcmp(token, "CONNECT") == 0){
        // The second token should be ip:port
        token = strtok(NULL, " ");
        if (token == NULL) {
            d_error("process_command", "No second token");
            return 0;
        } else if (is_peer_exist(token)){
            printf("Already connected to peer\n");
            return 1;
        }

        char* ip = strtok(token, ":");
        char* port_str = strtok(NULL, ":");
        if (ip == NULL || port_str == NULL) {
            printf("Missing address and port argument\n");
            return 1;
        }

        //Create a client thread
        pthread_t client_tid;
        char** args = (char**)malloc(2 * sizeof(char*));
        args[0] = strdup(ip);
        args[1] = strdup(port_str);

        if (pthread_create(&client_tid, NULL, client_thread, args) != 0){
            d_error("process_command", "Client thread creation failed");
            free(args[0]);
            free(args[1]);
            free(args);
            return 1;
        }

        // Wait for thread to end
        d_print("process_command.CONNECT", "wait for thread to end");
        if (pthread_join(client_tid, NULL) != 0) {
            d_error("process_command", "Failed to join client thread");
            return 1;
        }

        return 1;
    }

    else if (strcmp(token, "DISCONNECT") == 0){
        token = strtok(NULL, " ");
        d_print("process_command", "the token is %s", token);
        if (token == NULL) {
            d_error("process_command", "No second token");
            return 0;
        } if (!is_peer_exist(token)){
            printf("Unknown peer, not connected\n");
            return 1;
        }

        char* ip = strtok(token, ":");
        char* port_str = strtok(NULL, ":");
        if (ip == NULL || port_str == NULL) {
            printf("Missing address and port argument\n");
            return 1;
        }

        int port = atoi(port_str);

        int sock = get_peer(ip, port);

        if (sock != -1) {
            // Send DSN and close sock
            client_socket_disconnect(sock);

            // remove from peer_list
            remove_peer(ip, port);

            printf("Disconnected from peer\n");
        } else {
            printf("Peer not found: %s:%d\n", ip, port);
        }

        return 1;
    }

    else if (strcmp(token, "PEERS") == 0){
        print_peer_list();
        ping_peers();
        return 1;
    }

    else if (strcmp(token, "ADDPACKAGE") == 0){
        token = strtok(NULL, " ");
        if (token == NULL) {
            printf("Missing file argument\n");
            return 1;
        }

        char* path = concat_file_path(directory, token);

        struct bpkg_obj* package = bpkg_load(path);

        if (NULL != package){
            d_print("process_command.ADDPACKAGE", "package's filename is %s", package->filename);
            struct bpkg_query qry = bpkg_file_check(package);
            bpkg_query_destroy(&qry);
            add_package(package);
        } else {
            d_error("process_command.ADDPACKAGE", "Failed to load package");
        }

        return 1;
    } 

    else if (strcmp(token, "REMPACKAGE") == 0){
        token = strtok(NULL, " ");
        if (token == NULL) {
            printf("Missing identifier argument, please specify whole 1024 character or at least 20 characters.\n");
            return 1;
        }

        remove_package(token);

        return 1;
    }
    
    else if (strcmp(token, "PACKAGES") == 0) {
        print_packages(directory);
        return 1;
    }

    else if (strcmp(token, "FETCH") == 0) {
        token = strtok(NULL, " ");
        if (token == NULL) {
            d_print("process_command.FETCH", "Missing <ip:port> argument");
            printf("Missing arguments from command\n");
            return 1;
        }
        char* ip_port = strdup(token);

        // Get <identifier>
        token = strtok(NULL, " ");
        if (token == NULL) {
            d_print("process_command.FETCH", "Missing <identifier> argument");
            printf("Missing arguments from command\n");
            free(ip_port);
            return 1;
        }
        char* identifier = strdup(token);

        // Get <hash>
        token = strtok(NULL, " ");
        if (token == NULL) {
            d_print("process_command.FETCH", "Missing <hash> argument");
            printf("Missing arguments from command\n");
            free(ip_port);
            free(identifier);
            return 1;
        }
        char* hash = strdup(token);

        // Split <ip:port> into ip and port
        char* ip = strtok(ip_port, ":");
        char* port_str = strtok(NULL, ":");
        if (ip == NULL || port_str == NULL) {
            d_print("process_command.FETCH", "Invalid <ip:port> argument");
            printf("Missing arguments from command\n");
            free(ip_port);
            return 1;
        }

        int port = atoi(port_str);
        
        int sock = get_peer(ip, port);

        if (sock != -1) {
            PackageNode* target_package = get_package(identifier);

            if (NULL != target_package){
                struct merkle_tree_node* target_chunk = get_chunk(target_package->package, hash);
                if (NULL != target_chunk){
                    client_socket_fetch(sock, target_chunk, target_package->package->ident, hash);
                } else {
                    printf("Unable to request chunk, chunk hash does not belong to package");
                }
            } else{
                printf(" Unable to request chunk, package is not managed");
            }
        } else {
            printf("Unable to request chunk, peer not in list\n");
        }

        // Free allocated memory
        free(ip_port);
        free(identifier);
        free(hash);
        return 1;
    }

    return 0;
}
