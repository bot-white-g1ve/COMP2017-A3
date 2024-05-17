#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <bpkg.h>

// The linked list for storing the package
typedef struct PackageNode {
    struct bpkg_obj* package;
    struct PackageNode* next;
} PackageNode;

extern void add_package(struct bpkg_obj* package);
extern void print_packages(char* directory);
extern void free_packages();
extern void remove_package(const char* ident);

#endif
