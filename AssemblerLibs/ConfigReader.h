/*
code to read a basic config file

Written by Adam Billings
*/

#ifndef ConfigReader_h
#define ConfigReader_h

#include <stdint.h>
#include "DataStructures/StringTable.h"
#include "ExpressionEvaluation.h"
#include "MiscAssembler.h"

// memory access type
typedef enum AccessType {
    rw, ro, bss
} AccessType;

// segment information
typedef struct SegmentDef {
    char* name;
    uint16_t startAddr;
    uint16_t size;
    uint16_t align;
    AccessType accessType;
    char fill;
    uint16_t writeAddr;
    uint8_t* outputArr;
} SegmentDef;

// information on what has been parsed for a segment
typedef struct SegmentParseData {
    char readSize;
    char readStart;
    char readType;
    char readLoad;
    char readFill;
    char readAlign;
} SegmentParseData;

// sample uninitialized segment
extern const SegmentDef NEW_SEGMENT;

/*
determines if the line ending is valid

line: line to inspect
length: line length
isLineEnding: if the ending is a line ending

returns: if the line ending is valid
*/
char isValidConfigEnding(char *line, unsigned int length, char isLineEnding);

/*
read one attribute for a segment definition

file: source file
errorList: list of errors
line: line to read from
curLine: current line count
curCol: start column
segment: segment to update
segmentData: parse data for the segment
nullST: null string table (avoid malloc)

returns: number of chars read
*/
unsigned int readMemoryAttribute(FileHandle* handle, List* errorList, char* line, int curLine, int curCol, SegmentDef* segment, SegmentParseData* segmentData, StringTable nullST);

/*
read memory information

handle: file handle
errorList: list of errors

returns: memory information as segments
*/
List* readMemory(FileHandle* handle, List* errorList, unsigned int* curLine);

/*
read one attribute for a segment definition

file: source file
errorList: list of errors
line: line to read from
curLine: current line count
curCol: start column
segment: segment to update
segmentData: parse data for the segment
nullST: null string table (avoid malloc)

returns: number of chars read
*/
unsigned int readSegmentAttribute(FileHandle* handle, List* errorList, char* line, int curLine, int curCol, SegmentDef* segment, SegmentParseData* segmentData, List* memoryData, StringTable nullST);

/*
read segment information

handle: file handle
errorList: list of errors

returns: memory information as segments
*/
List* readSegment(FileHandle* handle, List* errorList, List* memoryList, unsigned int* curLine);

/*
reads a configuration file

handle: file handle
errorList: list of errors

returns: configuration segment information
*/
List* readConfigFile(FileHandle* handle, List* errorList);

#endif