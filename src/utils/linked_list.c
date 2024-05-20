#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <utils/linked_list.h>
#include <config.h>
#include <utils/str.h>
#include <bpkg.h>
#include <stdbool.h>
#include <debug/debug.h>

// linked list head
PackageNode* head = NULL;
pthread_mutex_t package_list_mutex = PTHREAD_MUTEX_INITIALIZER;

void add_package(struct bpkg_obj* package) {
    // Create a new node
    PackageNode* new_node = (PackageNode*)malloc(sizeof(PackageNode));
    new_node->package = package;
    new_node->next = NULL;

    pthread_mutex_lock(&package_list_mutex);
    d_print("add_packege", "mutex_lock obtained");

    // If the list is empty, set the new node as the head
    if (head == NULL) {
        head = new_node;
    } else {
        // Otherwise, find the last node and add the new node
        PackageNode* current = head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
    }

    pthread_mutex_unlock(&package_list_mutex);
    d_print("add_packege", "mutex_lock unlocked");
}

void print_packages(char* directory) {
    pthread_mutex_lock(&package_list_mutex);
    d_print("print_packeges", "mutex_lock obtained");

    PackageNode* current = head;
    if (current == NULL) {
        printf("No packages managed\n");
    } else {
        int i = 1;
        while (current != NULL) {
            char* file_path = concat_file_path(directory, current->package->filename);
            char* completed = bpkg_complete_check(current->package, directory);

            printf("%d. %.32s, %s : %s\n", i, current->package->ident, file_path, completed);
            i++;

            current = current->next;
        }
    }

    pthread_mutex_unlock(&package_list_mutex);
    d_print("print_packeges", "mutex_lock unlocked");
}

void remove_package(const char* ident) {
    bool package_found = false;

    pthread_mutex_lock(&package_list_mutex);
    d_print("remove_packege", "mutex_lock obtained");

    PackageNode* current = head;
    PackageNode* prev = NULL;

    while (current != NULL) {
        if (strncmp(current->package->ident, ident, 20) == 0) {
            if (prev == NULL) { // Node to be removed is the head
                head = current->next;
            } else {
                prev->next = current->next;
            }
            bpkg_obj_destroy(current->package);
            free(current);
            package_found = true;
            break;
        }
        prev = current;
        current = current->next;
    }

    pthread_mutex_unlock(&package_list_mutex);
    d_print("remove_packege", "mutex_lock unlocked");

    if (package_found == true){
        printf("Package has been removed\n");
    } else if (package_found == false){
        printf("Identifier provided does not match managed packages\n");
    }
}

void free_packages() {
    pthread_mutex_lock(&package_list_mutex);
    d_print("free_packeges", "mutex_lock obtained");

    PackageNode* current = head;
    while (current != NULL) {
        PackageNode* next = current->next;
        // Assuming bpkg_obj has a free function
        bpkg_obj_destroy(current->package);
        free(current);
        current = next;
    }
    head = NULL;

    pthread_mutex_unlock(&package_list_mutex);
    d_print("free_packege", "mutex_lock unlocked");
}

PackageNode* get_package(const char* ident) {
    d_print("get_package", "get_package called");
    pthread_mutex_lock(&package_list_mutex);
    d_print("get_packege", "mutex_lock obtained");

    PackageNode* current = head;

    while (current != NULL) {
        if (strncmp(current->package->ident, ident, 20) == 0) {
            pthread_mutex_unlock(&package_list_mutex);
            return current;
        }
        current = current->next;
    }

    pthread_mutex_unlock(&package_list_mutex);
    d_print("get_packege", "mutex_lock unlocked");

    return NULL;
}


