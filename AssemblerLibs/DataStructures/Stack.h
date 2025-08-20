/*
basic stack and stack operations
note: many of the functions here could be defined by macro, but are not for documentation reasons

Written by Adam Billings
*/

#ifndef Stack_h
#define Stack_h

#include "List.h"

// borrow code from the list
typedef struct List Stack;

// sample empty stack
extern const Stack EMPTY_STACK;

/*
create an empty stack

returns: empty stack
*/
Stack* newStack();

/*
frees all memory associated with a stack

stack: stack to free

returns: NULL
*/
Stack* deleteStack(Stack* stack);

/*
pushes an element to a stack

stack: stack to modify
dataptr: pointer to data
dataSize: size of data (in bytes)
*/
void pushStack(Stack* stack, const void* dataptr, size_t dataSize);

/*
pops an element from a stack (should be freed after use)

stack: stack to pop from

returns: pointer to element data
*/
void* popStack(Stack* stack);

/*
peek at the top element of the stack

stack: stack to peek at

returns: data pointer at the top of the stack
*/
void* peekStack(Stack* stack);

#endif