/*
Basic list and list operations

Written by Adam Billings
*/

#ifndef List_h
#define List_h

#include <stdlib.h>
#include <string.h>
#include "Node.h"


// basic List structure
typedef struct List {
    struct Node* head;
    struct Node* tail;
    int size;
} List;

// reference empty list
extern const List EMPTY_LIST;

/*
Create an empty list

returns: pointer to new list
*/
List* newList();

/*
frees all memory associated with a list

list: list to delete

returns: NULL
*/
List* deleteList(List* list);

/*
Prepend to the list

list: list to operate on
dataptr: pointer to the data to use
dataSize: size of data (in bytes)
*/
void prependList(List* list, const void* dataptr, size_t dataSize);

/*
Append to the list

list: list to operate on
dataptr: pointer to the data to use
datasize: size of data (in bytes)
*/
void appendList(List* list, const void* dataptr, size_t dataSize);

/*
gets the node at an index in a list

list: list to index
index: node index

returns: node at the index
*/
Node* getListNode(List* list, int index);

/*
reads a value fron the list

list: list to read
index: index to read

returns: pointer at index
*/
void* indexList(List* list, int index);

/*
removes a value from the list by index
note: this will free the node data

list: list to modify
index: index to remove
*/
void removeListElement(List* list, int index);

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
List* arrayToList(List* list, void* arr, size_t dataSize, size_t arrSize);

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
void* listToArray(void* arr, List* list, size_t dataSize, size_t arrSize);

#endif