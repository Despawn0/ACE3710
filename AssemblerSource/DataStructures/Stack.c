/*
basic stack and stack operations
note: many of the functions here could be defined by macro, but are not for documentation reasons

Written by Adam Billings
*/

#include "DataStructures/List.h"
#include "DataStructures/Stack.h"

// sample empty stack
const Stack EMPTY_STACK = {NULL, NULL, 0};

/*
create an empty stack

returns: empty stack
*/
Stack* newStack() {return newList();}

/*
frees all memory associated with a stack

stack: stack to free

returns: NULL
*/
Stack* deleteStack(Stack* stack) {return deleteList(stack);}

/*
pushes an element to a stack

stack: stack to modify
dataptr: pointer to data
dataSize: size of data (in bytes)
*/
void pushStack(Stack* stack, const void* dataptr, size_t dataSize) {
    prependList(stack, dataptr, dataSize);
}

/*
pops an element from a stack (should be freed after use)

stack: stack to pop from

returns: pointer to element data
*/
void* popStack(Stack* stack) {
    // get the data
    Node* head = stack->head;
    if (head == NULL) {return NULL;}
    void* dataptr = head->dataptr;
    // pop the node
    (stack->size)--;
    if (head->next != NULL) {
        head->next->prev = NULL;
    }
    stack->head = head->next;
    if (stack->size == 0) {
        stack->tail = NULL;
        stack->head = NULL;
    }
    free(head);

    // return the data
    return dataptr;
}

/*
peek at the top element of the stack

stack: stack to peek at

returns: data pointer at the top of the stack
*/
void* peekStack(Stack* stack) {
    if (stack->head == NULL) {return NULL;}
    return stack->head->dataptr;
}