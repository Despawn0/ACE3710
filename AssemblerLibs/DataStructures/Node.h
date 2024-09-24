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
const struct Node EMPTY_NODE = {NULL, NULL, NULL, 0};

/*
create a node pointing to a deep copy of the data

dataptr: pointer to data

returns: pointer to new node
*/
Node* newNode(const void* dataptr, size_t dataSize) {
    // create the node
    Node* newNodeVal = (Node*)memcpy(malloc(sizeof(Node)), &EMPTY_NODE, sizeof(Node));

    // set the value
    newNodeVal->dataptr = memcpy(malloc(dataSize), dataptr, dataSize);
    
    // return the node
    return newNodeVal;
}

/*
delete a node and all of its contents

returns: NULL
*/
Node* deleteNode(Node* node) {
    // do nothing for NULL
    if (node == NULL || node->isDeleted) {return NULL;}

    // mark deleted
    node -> isDeleted = 1;

    // deallocate the members
    node->prev = deleteNode(node->prev);
    node->next = deleteNode(node->next);
    free(node->dataptr);
    
    // deallocate the node
    free(node);
    return NULL;
}

#endif