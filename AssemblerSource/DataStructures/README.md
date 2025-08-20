
# About

The data structures library contains basic data structures such as the list, stack, and queue.

Written by Adam Billings

# Node

The node is used as the basic building block for all of the data structures.

The node has the following functions:
    - newNode
    - deleteNode

The node has the following elements:
    - prev
    - next
    - dataptr

# List

The list is composed of nodes. Due to its flexability, it is used in the construction of the other data structures.

The list has the following functions:
    - newList
    - deleteList
    - prependList
    - appendList
    - getListNode
    - indexList
    - removeListElement
    - arrayToList
    - listToArray

The list has the following elements:
    - head
    - tail
    - size

# Stack

The stack is constructed using the list.

The stack has the following functions:
    - newStack
    - deleteStack
    - pushStack
    - popStack
    - peekStack

The stack has the following elements:
    - size

# Queue

The queue is constructed using the list.

The queue has the following functions:
    - newQueue
    - deleteQueue
    - pushQueue
    - popQueue
    - peekQueue

The queue has the following elements:
    - size

# StringTable

The string table is a hash table constructed using an array of lists

The string table has the following functions
    - setStringTableValue
    - readStringTable
    - removeStringTableValue
