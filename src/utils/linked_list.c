#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <utils/linked_list.h>
#include <config.h>
#include <utils/str.h>

// linked list head
PackageNode* head = NULL;
pthread_mutex_t package_list_mutex = PTHREAD_MUTEX_INITIALIZER;

void add_package(struct bpkg_obj* package) {
    // Create a new node
    PackageNode* new_node = (PackageNode*)malloc(sizeof(PackageNode));
    new_node->package = package;
    new_node->next = NULL;

    pthread_mutex_lock(&package_list_mutex);

    // Add to the head of the list
    new_node->next = head;
    head = new_node;

    pthread_mutex_unlock(&package_list_mutex);
}

void print_packages(char* directory) {
    pthread_mutex_lock(&package_list_mutex);

    PackageNode* current = head;
    if (current == NULL) {
        printf("No packages managed\n");
    } else {
        int i = 1;
        while (current != NULL) {
            char* file_path = concat_file_path(directory, current->package->filename);

            printf("%d. %.32s, %s : INCOMPLETE\n", i, current->package->ident, file_path);
            i++;

            current = current->next;
        }
    }

    pthread_mutex_unlock(&package_list_mutex);
}

void free_packages() {
    pthread_mutex_lock(&package_list_mutex);

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
}

