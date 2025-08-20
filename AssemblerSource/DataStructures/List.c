/*
Basic list and list operations

Written by Adam Billings
*/

#include <stdlib.h>
#include <string.h>
#include "DataStructures/Node.h"
#include "DataStructures/List.h"

// reference empty list
const List EMPTY_LIST = {NULL, NULL, 0};

/*
Create an empty list

returns: pointer to new list
*/
List* newList() {
    return (List*)memcpy(malloc(sizeof(List)), &EMPTY_LIST, sizeof(List));
}

/*
frees all memory associated with a list

list: list to delete

returns: NULL
*/
List* deleteList(List* list) {
    if (list->head != NULL) {
        deleteNode(list->head);
    }
    free(list);
    return NULL;
}

/*
Prepend to the list

list: list to operate on
dataptr: pointer to the data to use
dataSize: size of data (in bytes)
*/
void prependList(List* list, const void* dataptr, size_t dataSize) {
    // create the new node
    Node* newNodeVal = newNode(dataptr, dataSize);

    // add the node
    if (list->head == NULL) {
        deleteNode(list->tail); // safety
        list->tail = newNodeVal;
    } else {
        list->head->prev = newNodeVal;
        newNodeVal->next = list->head;
    }
    list->size++;
    list->head = newNodeVal;
}

/*
Append to the list

list: list to operate on
dataptr: pointer to the data to use
datasize: size of data (in bytes)
*/
void appendList(List* list, const void* dataptr, size_t dataSize) {
    // create the new node
    Node* newNodeVal = newNode(dataptr, dataSize);

    // add the node
    if (list->tail == NULL) {
        deleteNode(list->head); // safety
        list->head = newNodeVal;
    } else {
        list->tail->next = newNodeVal;
        newNodeVal->prev = list->tail;
    }
    list->size++;
    list->tail = newNodeVal;
}

/*
gets the node at an index in a list

list: list to index
index: node index

returns: node at the index
*/
Node* getListNode(List* list, int index) {
    // allow for bidirectional indexing
    if (index >= 0) {
        // cap the bounds
        if (index >= list->size) {return NULL;}

        // get the data
        Node* curNode = list->head;
        for (; index > 0; index--) {curNode = curNode->next;}
        return curNode;
    } else {
        // cap the bounds
        if (index < -(list->size)) {return NULL;}

        // get the data
        Node* curNode = list->tail;
        for (index++; index < 0; index++) {curNode = curNode->prev;}
        return curNode;
    }
}

/*
reads a value fron the list

list: list to read
index: index to read

returns: pointer at index
*/
void* indexList(List* list, int index) {
    Node* node = getListNode(list, index);
    if (node == NULL) {return NULL;}
    return node->dataptr;
}

/*
removes a value from the list by index
note: this will free the node data

list: list to modify
index: index to remove
*/
void removeListElement(List* list, int index) {
    // declare node to work with
    Node* curNode;

    // allow for bidirectional indexing
    if (index >= 0) {
        // cap the bounds
        if (index >= list->size) {return;}

        // get the node
        curNode = list->head;
        for (int i = index; i > 0; i--) {curNode = curNode->next;}
    } else {
        // cap the bounds
        if (index < -(list->size)) {return;}

        // get the node
        curNode = list->tail;
        for (int i = index + 1; i < 0; i++) {curNode = curNode->next;}
    }

    // patch the gap
    list->size--;
    if (curNode->prev != NULL) {curNode->prev->next = curNode->next;}
    if (curNode->next != NULL) {curNode->next->prev = curNode->prev;}
    if (index == 0) {
        list->head = list->head->next;
        if (list->size == 0) {list->tail = NULL;}
    } else if (index == -1) {
        list->tail = list->tail->prev;
        if (list->size == 0) {list->head = NULL;}
    }

    // delete the node
    free(curNode->dataptr);
    free(curNode);
}

/*
converts an array into a list
note: will free the current list data
note: will allocate new list it list is NULL

list: list to output to
arr: array to convert
dataSize: size of the array elements (in bytes)
arrSize: size of the array (in bytes)

returns: pointer to the list
*/
List* arrayToList(List* list, void* arr, size_t dataSize, size_t arrSize) {
    // ensure that there is a list to output to
    if (list == NULL) {list = newList();}
    else {
        deleteNode(list->head);
        list->size = 0;
    }

    // add the array to the list
    for (void* i = arr; i < arr + arrSize; i = i + dataSize) {appendList(list, i, dataSize);}

    // return the list
    return list;
}

/*
converts the list to an array
note: the array (if not NULL) should be large enough to accept all elements, or data may be lost
note: will allocate a new array if arr is NULL; use list size to get the element count

arr: array to copy to
list: list to copy
dataSize: size of array elements (in bytes)
arrSize: size of array (in bytes)

returns: pointer to the array
*/
void* listToArray(void* arr, List* list, size_t dataSize, size_t arrSize) {
    // ensure that there is an array to output to
    if (arr == NULL) {
        arrSize = dataSize * list->size;
        arr = malloc(arrSize);
    }

    // copy the data
    void* writeAddr = arr;
    for (Node* curNode = list->head; curNode != NULL; curNode = curNode->next) {
        memcpy(writeAddr, curNode->dataptr, dataSize);
        writeAddr = writeAddr + dataSize;
    }

    // return the array
    return arr;
}