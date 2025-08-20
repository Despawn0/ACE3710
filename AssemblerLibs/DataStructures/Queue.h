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
extern const Queue EMPTY_QUEUE;

/*
create an empty queue

returns: empty queue
*/
Queue* newQueue();

/*
frees all memory associated with a queue

queue: queue to free

returns: NULL
*/
Queue* deleteQueue(Queue* queue);

/*
pushes an element to a queue

queue: queue to modify
dataptr: pointer to data
dataSize: size of data (in bytes)
*/
void pushQueue(Queue* queue, const void* dataptr, size_t dataSize);

/*
pops an element from a queue (should be freed after use)

queue: queue to pop from

returns: pointer to element data
*/
void* popQueue(Queue* queue);

/*
peek at the top element of the queue

queue: queue to peek at

returns: data pointer at the top of the queue
*/
void* peekQueue(Queue* queue);

#endif