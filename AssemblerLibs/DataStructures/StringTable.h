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
StringTable newStringTable();

/*
deletes a StringTable

stringTable: StringTable to delete
*/
void deleteStringTable(StringTable stringTable);

/*
checks for string equality, safer than strcmp

string1: first string
string1Length: maximum length of the first string
string2: second string
string2Length: maximum length of the second string

returns: if the strings are equal
*/
static char stringsAreEqual(char* string1, int string1Length, char* string2, int string2Length);

/*
returns the length of a string, safer than strlen

string: string to evaluate
maxLen: maximum possible length

returns: length of the string
*/
static int getStringLength(char* string, int maxLen);

/*
calculates the hash index to use in the string table

string: string to hash
stringLength: maximum length of the string

returns: hash index
*/
static unsigned char getStringHash(char* string, int stringLength);

/*
adds or updates an entry into a StringTable

table: table to update
string: string to add
stringLength: maximum length of the string
valueptr: pointer to value to enter
dataSize: size of the value (in bytes)
*/
void setStringTableValue(StringTable table, char* string, int stringLength, const void* valueptr, size_t dataSize);

/*
reads a value from a StringTable

table: table to read
string: index string
stringLength: length of index string

returns: data pointer at the table entry, NULL if the value is not present
*/
void* readStringTable(StringTable table, char* string, int stringLength);

/*
removes an entry from a StringTable, if present

table: table to update
string: index to remove
stringLength: length of the string index
*/
void removeStringTableValue(StringTable table, char* string, int stringLength);

#endif