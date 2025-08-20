/*
Simple node for data structures

Written by Adam Billings
*/

#ifndef Node_h
#define Node_h

#include <stdlib.h>
#include <string.h>

// simple node struct
typedef struct Node {
    struct Node* prev;
    struct Node* next;
    void* dataptr;
    char isDeleted; // leave as 0 until freed
} Node;

// reference empty node
extern const struct Node EMPTY_NODE;

/*
create a node pointing to a deep copy of the data

dataptr: pointer to data

returns: pointer to new node
*/
Node* newNode(const void* dataptr, size_t dataSize);

/*
delete a node and all of its contents

returns: NULL
*/
Node* deleteNode(Node* node);

#endif