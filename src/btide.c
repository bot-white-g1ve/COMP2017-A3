#include <stdio.h>
#include <stdlib.h>
#include <config.h>
#include <debug/debug.h>
#include <btide.h>
#include <peer.h>
#include <pthread.h>
#include <string.h>

//
// PART 2
//
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
    
    d_print("main", "Config load successfully");
    d_print("main", "Directory: %s", config->directory);
    d_print("main", "Max Peers: %d", config->max_peers);
    d_print("main", "Port: %u", config->port);

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

        return 1;
    }

    else if (strcmp(token, "DISCONNECT") == 0){
        token = strtok(NULL, " ");
        if (token == NULL) {
            d_error("process_command", "No second token");
            return 0;
        } if (!is_peer_exist(token)){

        }
    }

    return 1;
}
