/*
Basic hash table for strings to use as a varaiable LUT

Written by Adam Billings
*/

#ifndef StringTable_h
#define StringTable_h

#include <stdint.h>
#include <string.h>
#include "List.h"

// "nodes" in the LUT lists
typedef struct KeyValuePair {
    char* key;
    int keyLen;
    void* valueptr;
} KeyValuePair;

// to separate this "special" array in type signatures; MUST BE FREED
typedef List** StringTable;

/*
creates and returns a new StringTable

returns: new StringTable
*/
StringTable newStringTable() {
    StringTable newTable = (StringTable)malloc(256 * sizeof(List*));
    for (int i = 0; i < 256; i++) {
        newTable[i] = newList();
    }
    return newTable;
}

/*
deletes a StringTable

stringTable: StringTable to delete
*/
void deleteStringTable(StringTable stringTable) {
    for (int i = 0; i < 256; i++) {
        for (Node* j = stringTable[i]->head; j != NULL; j = j->next) {
            KeyValuePair nodeVal = *(KeyValuePair*)(j->dataptr);
            free(nodeVal.key);
            free(nodeVal.valueptr);
        }
        deleteList(stringTable[i]);
    }
    free(stringTable);
}

/*
checks for string equality, safer than strcmp

string1: first string
string1Length: maximum length of the first string
string2: second string
string2Length: maximum length of the second string

returns: if the strings are equal
*/
static char stringsAreEqual(char* string1, int string1Length, char* string2, int string2Length) {
    char areEqual = 1;
    for (int i = 0; i < string1Length && i < string2Length; i++) {
        if (string1[i] != string2[i]) {areEqual = 0;}
        if (string1[i] == '\0' || string2[i] == '\0') {return areEqual;}
    }
    return (string1Length == string2Length) && areEqual;
}

/*
returns the length of a string, safer than strlen

string: string to evaluate
maxLen: maximum possible length

returns: length of the string
*/
static int getStringLength(char* string, int maxLen) {
    for (int i = 0; i < maxLen; i++) {
        if (string[i] == '\0') {
            return i + 1;
        }
    }
    return maxLen;
}

/*
calculates the hash index to use in the string table

string: string to hash
stringLength: maximum length of the string

returns: hash index
*/
static unsigned char getStringHash(char* string, int stringLength) {
    unsigned char hash = 0;
    for (int i = 0; i < stringLength; i++) {
        if (string[i] == '\0') {break;}
        hash = (char)((hash << 1) + string[i]);
    }
    return hash;
}

/*
adds or updates an entry into a StringTable

table: table to update
string: string to add
stringLength: maximum length of the string
valueptr: pointer to value to enter
dataSize: size of the value (in bytes)
*/
void setStringTableValue(StringTable table, char* string, int stringLength, const void* valueptr, size_t dataSize) {
    // get the list to update
    List* updateList = table[getStringHash(string, stringLength)];

    // try to update existing values
    for (Node* i = updateList->head; i != NULL; i = i->next) {
        KeyValuePair* curValue = (KeyValuePair*)(i->dataptr);
        if (stringsAreEqual(string, stringLength, curValue->key, curValue->keyLen)) {
            free(curValue->valueptr);
            curValue->valueptr = malloc(dataSize);
            memcpy(curValue->valueptr, valueptr, dataSize);
            return;
        }
    }

    // add a new value
    KeyValuePair newPair = {NULL, 0, NULL};
    newPair.keyLen = getStringLength(string, stringLength);
    newPair.key = (char*)malloc(newPair.keyLen * sizeof(char));
    memcpy(newPair.key, string, newPair.keyLen * sizeof(char));
    newPair.valueptr = malloc(dataSize);
    memcpy(newPair.valueptr, valueptr, dataSize);
    appendList(updateList, &newPair, sizeof(KeyValuePair));
}

/*
reads a value from a StringTable

table: table to read
string: index string
stringLength: length of index string

returns: data pointer at the table entry, NULL if the value is not present
*/
void* readStringTable(StringTable table, char* string, int stringLength) {
    List* checkList = table[getStringHash(string, stringLength)];
    for (Node* i = checkList->head; i != NULL; i = i->next) {
        KeyValuePair nodeVal = *(KeyValuePair*)(i->dataptr);
        if (stringsAreEqual(string, stringLength, nodeVal.key, nodeVal.keyLen)) {
            return nodeVal.valueptr;
        }
    }
    return NULL;
}

/*
removes an entry from a StringTable, if present

table: table to update
string: index to remove
stringLength: length of the string index
*/
void removeStringTableValue(StringTable table, char* string, int stringLength) {
    List* checkList = table[getStringHash(string, stringLength)];
    for (Node* i = checkList->head; i != NULL; i = i->next) {
        KeyValuePair* nodeVal = (KeyValuePair*)(i->dataptr);
        if (stringsAreEqual(string, stringLength, nodeVal->key, nodeVal->keyLen)) {
            if (i->next != NULL) {i->next->prev = i->prev;}
            if (i->prev != NULL) {i->prev->next = i->next;}
            if (i == checkList->head) {checkList->head = i->next;}
            if (i == checkList->tail) {checkList->tail = i->prev;}
            free(nodeVal->key);
            free(nodeVal->valueptr);
            free(i->dataptr);
            free(i);
            return;
        }
    }
}

#endif