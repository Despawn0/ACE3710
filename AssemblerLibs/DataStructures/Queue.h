/*
basic queue and queue operations
note: many of the functions here could be defined by macro, but are not for documentation reasons

Written by Adam Billings
*/

#ifndef Queue_h
#define Queue_h

#include "List.h"

// borrow code from the list
typedef struct List Queue;

// sample empty queue
const Queue EMPTY_QUEUE = {NULL, NULL, 0};

/*
create an empty queue

returns: empty queue
*/
Queue* newQueue() {return newList();}

/*
frees all memory associated with a queue

queue: queue to free

returns: NULL
*/
Queue* deleteQueue(Queue* queue) {return deleteList(queue);}

/*
pushes an element to a queue

queue: queue to modify
dataptr: pointer to data
dataSize: size of data (in bytes)
*/
void pushQueue(Queue* queue, const void* dataptr, size_t dataSize) {
    appendList(queue, dataptr, dataSize);
}

/*
pops an element from a queue (should be freed after use)

queue: queue to pop from

returns: pointer to element data
*/
void* popQueue(Queue* queue) {
    // get the data
    Node* head = queue->head;
    if (head == NULL) {return NULL;}
    void* dataptr = head->dataptr;

    // pop the node
    (queue->size)--;
    if (head->next != NULL) {
        head->next->prev = NULL;
    }
    queue->head = head->next;
    if (queue->size == 0) {queue->tail = NULL;}
    free(head);

    // return the data
    return dataptr;
}

/*
peek at the top element of the queue

queue: queue to peek at

returns: data pointer at the top of the queue
*/
void* peekQueue(Queue* queue) {
    if (queue->head == NULL) {return NULL;}
    return queue->head->dataptr;
}

#endif