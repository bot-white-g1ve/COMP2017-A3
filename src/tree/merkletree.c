#include "merkletree.h"

struct bt_node {
    void* key;
    void* value;
    struct bt_node* left;
    struct bt_node* right;
};
