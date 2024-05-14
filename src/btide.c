#include <stdio.h>
#include <stdlib.h>
#include <config.h>
#include <debug/debug.h>
#include <btide.h>
#include <peer.h>

//
// PART 2
//

int main(int argc, char** argv) {
    d_init();

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

    free(config);
}

void* cli_thread(){
    char input[MAX_CLI_INPUT];
    int return_code;

    printf("Welcome to btide, enter your commands below");

    while (1){
        // Read from standard input
        if (fgets(input, MAX_CLI_INPUT, stdin) == NULL) {
            continue; // If fail, read next line
        }

        d_print("cli_thread", "user inputs %s", input);

        // Remove \n
        input[strcspn(input, "\n")] = 0;

        return_code = process_command(input);
        d_print("cli_thread", "the return code is %d", return_code);

        if (return_code == 0){
            printf("Invalid Input.");
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

    if (strcmp(token, "QUIT") == 0){
        return -1;
    }
}