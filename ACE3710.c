/*
Simple assembler for the CR16

Written by Adam Billings
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "AssemblerLibs/Help/MainHelp.h"
#include "AssemblerLibs/MiscAssembler.h"
#include "AssemblerLibs/DataStructures/List.h"
#include "AssemblerLibs/ConfigReader.h"
#include "AssemblerLibs/MacroReading.h"
#include "AssemblerLibs/VarEvaluation.h"
#include "AssemblerLibs/Assemble.h"
#include "AssemblerLibs/Quotes.h"

/*
set a default configuration

returns: default configuratiojn list
*/
List* getDefaultConfig() {
    List* cfg = newList();
    SegmentDef code = {"CODE", 0x0000, 0x8000, 1, ro, 1};
    SegmentDef data = {"DATA", 0x8000, 0x1000, 1, rw, 1};
    SegmentDef dispatchTable = {"DISPATCH_TABLE", 0x9000, 0x1000, 1, rw, 1};
    SegmentDef _bss = {"BSS", 0xa000, 0x4000, 1, bss, 0};
    SegmentDef stack = {"STACK", 0xe000, 0x0800, 1, bss, 0};
    SegmentDef interruptStack = {"INTERRUPT_STACK", 0xe800, 0x0800, 1, bss, 0};
    appendList(cfg, &code, sizeof(SegmentDef));
    appendList(cfg, &data, sizeof(SegmentDef));
    appendList(cfg, &dispatchTable, sizeof(SegmentDef));
    appendList(cfg, &_bss, sizeof(SegmentDef));
    appendList(cfg, &stack, sizeof(SegmentDef));
    appendList(cfg, &interruptStack, sizeof(SegmentDef));
    return cfg;
}

/*
prints version information
*/
void printVersion() {
    printf("  -- Assembler for CS/ECE 3710 --\n");
    printf("    Version 1.0.12\n");
    printf("    Written by Adam Billings\n\n");
    printf("    Happy programming!\n\n");
}

int main(int argc, char* argv[]) {
    // handle args
    char* fileName = NULL;
    char hasConfig = 0;
    char hasSize = 0;
    char hasEndian = 0;
    char isDefaultConfig = 1;
    char isLittleEndian = 1;
    char hasQuote = 0;
    char isHex = 0;
    char* outputFileName = NULL;
    unsigned int wordSize = 2;
    List* segments = getDefaultConfig();
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--help")) {
            if (i < argc - 1 && argv[i + 1][0] != '-') {
                i++;
                if (!isDefaultConfig) {
                    for (Node* node = segments->head; node != NULL; node = node->next) {
                        SegmentDef* segDef = ((SegmentDef*)(node->dataptr));
                        free(segDef->name);
                        if (segDef->outputArr != NULL) {free(segDef->outputArr);}
                    }
                }
                deleteList(segments);
                if (!strcmp(argv[i], "expressions")) {printExprHelp(); return 0;}
                else if (!strcmp(argv[i], "variables")) {printVarHelp(); return 0;}
                else if (!strcmp(argv[i], "config")) {printConfigHelp(); return 0;}
                else if (!strcmp(argv[i], "default-config")) {printDefaultConfigHelp(); return 0;}
                else if (!strcmp(argv[i], "macros")) {printMacroHelp(); return 0;}
                else if (!strcmp(argv[i], "instructions")) {printInstructionHelp(); return 0;}
                else if (!strcmp(argv[i], "sample-assembly")) {printSampleHelp(); return 0;}
                else {
                    printf("\e[1;31mERROR:\e[0m Help page \"%s\" does not exist\n\n", argv[i]);
                    return -2;
                }
            }
            printHelp();
            return 0;
        }
        else if (!strcmp(argv[i], "--text-byte")) {isHex = 1; continue;}
        else if (!strcmp(argv[i], "--text-word")) {isHex = 2; continue;}
        else if (!strcmp(argv[i], "--raw")) {isHex = 0; continue;}
        else if (!strcmp(argv[i], "--config")) {
            if (hasConfig) {
                // delete segments
                if (!isDefaultConfig) {
                    for (Node* node = segments->head; node != NULL; node = node->next) {
                        SegmentDef* segDef = ((SegmentDef*)(node->dataptr));
                        free(segDef->name);
                        if (segDef->outputArr != NULL) {free(segDef->outputArr);}
                    }
                }
                deleteList(segments);
                printf("\e[1;31mERROR:\e[0m Multiple configurations proveided\n\n");
                return -2;
            }

            // clear default configuration
            deleteList(segments);
            segments = NULL;
            isDefaultConfig = 0;

            // read new configuration
            hasConfig = 1;
            i++;
            if (i >= argc || argv[i][0] == '-') {
                printf("\e[1;31mERROR:\e[0m Expected configuration file\n\n");
                return -2;
            }
            FileHandle cfgHandle = {fopen(argv[i], "r"), argv[i], 0, 0};
            if (cfgHandle.fptr == NULL) {
                printf("\e[1;31mERROR:\e[0m Could not open %s\n\n", cfgHandle.name);
                return -2;
            }
            fseek(cfgHandle.fptr, 0, SEEK_END);
            cfgHandle.length = ftell(cfgHandle.fptr);
            rewind(cfgHandle.fptr);
            List* errorList = newList();
            validateFile(&cfgHandle, errorList);
            if (errorList->size == 0) {
                segments = readConfigFile(&cfgHandle, errorList);
            }

            // handle the errors
            if (errorList->size > 0) {
                // print errors
                for (Node* node = errorList->head; node != NULL; node = node->next) {
                    ErrorData errorData = *(ErrorData*)(node->dataptr);
                    printError(errorData);
                    free(errorData.errorMsg);
                }
                deleteList(errorList);

                // free the segments
                if (segments != NULL) {
                    for (Node* node = segments->head; node != NULL; node = node->next) {
                        free(((SegmentDef*)(node->dataptr))->name);
                    }
                    deleteList(segments);
                }
                fclose(cfgHandle.fptr);
                return -2;
            }
            fclose(cfgHandle.fptr);
            deleteList(errorList);
            continue;
        } else if (!strcmp(argv[i], "--config-default")) {
            if (hasConfig) {
                // delete segments
                if (!isDefaultConfig) {
                    for (Node* node = segments->head; node != NULL; node = node->next) {
                        SegmentDef* segDef = ((SegmentDef*)(node->dataptr));
                        free(segDef->name);
                        if (segDef->outputArr != NULL) {free(segDef->outputArr);}
                    }
                }
                deleteList(segments);
                printf("\e[1;31mERROR:\e[0m Multiple configurations proveided\n\n");
                return -2;
            }
            hasConfig = 1;
            continue;
        } else if (!strcmp(argv[i], "--byte")) {
            if (hasSize) {
                // delete segments
                if (!isDefaultConfig) {
                    for (Node* node = segments->head; node != NULL; node = node->next) {
                        SegmentDef* segDef = ((SegmentDef*)(node->dataptr));
                        free(segDef->name);
                        if (segDef->outputArr != NULL) {free(segDef->outputArr);}
                    }
                }
                deleteList(segments);
                printf("\e[1;31mERROR:\e[0m Multiple word sizes proveided\n\n");
                return -2;
            }
            hasSize = 1;
            wordSize = 2;
            continue;
        }
        else if (!strcmp(argv[i], "--word")) {
            if (hasSize) {
                // delete segments
                if (!isDefaultConfig) {
                    for (Node* node = segments->head; node != NULL; node = node->next) {
                        SegmentDef* segDef = ((SegmentDef*)(node->dataptr));
                        free(segDef->name);
                        if (segDef->outputArr != NULL) {free(segDef->outputArr);}
                    }
                }
                deleteList(segments);
                printf("\e[1;31mERROR:\e[0m Multiple word sizes proveided\n\n");
                return -2;
            }
            hasSize = 1;
            wordSize = 1;
            continue;
        } else if (!strcmp(argv[i], "--little-endian")) {
            if (hasEndian) {
                // delete segments
                if (!isDefaultConfig) {
                    for (Node* node = segments->head; node != NULL; node = node->next) {
                        SegmentDef* segDef = ((SegmentDef*)(node->dataptr));
                        free(segDef->name);
                        if (segDef->outputArr != NULL) {free(segDef->outputArr);}
                    }
                }
                deleteList(segments);
                printf("\e[1;31mERROR:\e[0m Multiple word sizes proveided\n\n");
                return -2;
            }
            hasEndian = 1;
            isLittleEndian = 1;
            continue;
        } else if (!strcmp(argv[i], "--big-endian")) {
            if (hasEndian) {
                // delete segments
                if (!isDefaultConfig) {
                    for (Node* node = segments->head; node != NULL; node = node->next) {
                        SegmentDef* segDef = ((SegmentDef*)(node->dataptr));
                        free(segDef->name);
                        if (segDef->outputArr != NULL) {free(segDef->outputArr);}
                    }
                }
                deleteList(segments);
                printf("\e[1;31mERROR:\e[0m Multiple word sizes proveided\n\n");
                return -2;
            }
            hasEndian = 1;
            isLittleEndian = 0;
            continue;
        } else if (!strcmp(argv[i], "--quote")) {hasQuote = 1; continue;}
        else if (!strcmp(argv[i], "--version")) {
            // delete segments
            if (!isDefaultConfig) {
                for (Node* node = segments->head; node != NULL; node = node->next) {
                    SegmentDef* segDef = ((SegmentDef*)(node->dataptr));
                    free(segDef->name);
                    if (segDef->outputArr != NULL) {free(segDef->outputArr);}
                }
            }
            deleteList(segments);
            printVersion();
            return 0;
        } else if (!strcmp(argv[i], "--output")) {
            if (outputFileName != NULL) {
                // delete segments
                if (!isDefaultConfig) {
                    for (Node* node = segments->head; node != NULL; node = node->next) {
                        SegmentDef* segDef = ((SegmentDef*)(node->dataptr));
                        free(segDef->name);
                        if (segDef->outputArr != NULL) {free(segDef->outputArr);}
                    }
                }
                deleteList(segments);
                printf("\e[1;31mERROR:\e[0m Multiple output file names declared\n\n");
                return -2;
            }
            i++;
            if (i >= argc || argv[i][0] == '-') {
                // delete segments
                if (!isDefaultConfig) {
                    for (Node* node = segments->head; node != NULL; node = node->next) {
                        SegmentDef* segDef = ((SegmentDef*)(node->dataptr));
                        free(segDef->name);
                        if (segDef->outputArr != NULL) {free(segDef->outputArr);}
                    }
                }
                deleteList(segments);
                printf("\e[1;31mERROR:\e[0m Multiple output file names declared\n\n");
                return -2;
            }
            outputFileName = argv[i];
            continue;
        } else if (argv[i][0] == '-') {
            if (argv[i][1] == '-') {
                // delete segments
                if (!isDefaultConfig) {
                    for (Node* node = segments->head; node != NULL; node = node->next) {
                        SegmentDef* segDef = ((SegmentDef*)(node->dataptr));
                        free(segDef->name);
                        if (segDef->outputArr != NULL) {free(segDef->outputArr);}
                    }
                }
                deleteList(segments);
                printf("\e[1;31mERROR:\e[0m Unrecognized option %s\n\n", argv[i]);
                return -2;
            } else if (argv[i][1] == '\0') {
                // delete segments
                if (!isDefaultConfig) {
                    for (Node* node = segments->head; node != NULL; node = node->next) {
                        SegmentDef* segDef = ((SegmentDef*)(node->dataptr));
                        free(segDef->name);
                        if (segDef->outputArr != NULL) {free(segDef->outputArr);}
                    }
                }
                deleteList(segments);
                printf("\e[1;31mERROR:\e[0m Expected option value\n\n");
                return -2;
            } else {
                int k = i;
                for (int j = 1; j < strlen(argv[i]); j++) {
                    if (argv[i][j] == 'h') {
                        if (i < argc - 1 && argv[i + 1][0] != '-') {
                            k++;
                            if (!isDefaultConfig) {
                                for (Node* node = segments->head; node != NULL; node = node->next) {
                                    SegmentDef* segDef = ((SegmentDef*)(node->dataptr));
                                    free(segDef->name);
                                    if (segDef->outputArr != NULL) {free(segDef->outputArr);}
                                }
                            }
                            deleteList(segments);
                            if (!strcmp(argv[k], "expressions")) {printExprHelp(); return 0;}
                            else if (!strcmp(argv[k], "variables")) {printVarHelp(); return 0;}
                            else if (!strcmp(argv[k], "config")) {printConfigHelp(); return 0;}
                            else if (!strcmp(argv[k], "default-config")) {printDefaultConfigHelp(); return 0;}

                            else if (!strcmp(argv[k], "macros")) {printMacroHelp(); return 0;}
                            else if (!strcmp(argv[k], "instructions")) {printInstructionHelp(); return 0;}
                            else if (!strcmp(argv[k], "sample-assembly")) {printSampleHelp(); return 0;}
                            else if (argv[k][0] != '-') {
                                printf("\e[1;31mERROR:\e[0m Help page \"%s\" does not exist\n\n", argv[k]);
                                return -2;
                            }
                        }
                        printHelp();
                        return 0;
                    }
                    else if (argv[i][j] == 't') {isHex = 1; continue;}
                    else if (argv[i][j] == 'T') {isHex = 2; continue;}
                    else if (argv[i][j] == 'r') {isHex = 0; continue;}
                    else if (argv[i][j] == 'c') {
                        if (hasConfig) {
                            // delete segments
                            if (!isDefaultConfig) {
                                for (Node* node = segments->head; node != NULL; node = node->next) {
                                    SegmentDef* segDef = ((SegmentDef*)(node->dataptr));
                                    free(segDef->name);
                                    if (segDef->outputArr != NULL) {free(segDef->outputArr);}
                                }
                            }
                            deleteList(segments);
                            printf("\e[1;31mERROR:\e[0m Multiple configurations proveided\n\n");
                            return -2;
                        }

                        // clear default configuration
                        deleteList(segments);
                        segments = NULL;
                        isDefaultConfig = 0;

                        // read new configuration
                        hasConfig = 1;
                        k++;
                        if (k >= argc || argv[k][0] == '-') {
                            // delete segments
                            if (!isDefaultConfig) {
                                for (Node* node = segments->head; node != NULL; node = node->next) {
                                    SegmentDef* segDef = ((SegmentDef*)(node->dataptr));
                                    free(segDef->name);
                                    if (segDef->outputArr != NULL) {free(segDef->outputArr);}
                                }
                            }
                            printf("\e[1;31mERROR:\e[0m Expected configuration file\n\n");
                            return -2;
                        }
                        FileHandle cfgHandle = {fopen(argv[k], "r"), argv[k], 0, 0};
                        if (cfgHandle.fptr == NULL) {
                            printf("\e[1;31mERROR:\e[0m Could not open %s\n\n", cfgHandle.name);
                            return -2;
                        }
                        fseek(cfgHandle.fptr, 0, SEEK_END);
                        cfgHandle.length = ftell(cfgHandle.fptr);
                        rewind(cfgHandle.fptr);
                        List* errorList = newList();
                        validateFile(&cfgHandle, errorList);
                        if (errorList->size == 0) {
                            segments = readConfigFile(&cfgHandle, errorList);
                        }

                        // handle the errors
                        if (errorList->size > 0) {
                            // print errors
                            for (Node* node = errorList->head; node != NULL; node = node->next) {
                                ErrorData errorData = *(ErrorData*)(node->dataptr);
                                printError(errorData);
                                free(errorData.errorMsg);
                            }
                            deleteList(errorList);

                            // free the segments
                            if (segments != NULL) {
                                for (Node* node = segments->head; node != NULL; node = node->next) {
                                    free(((SegmentDef*)(node->dataptr))->name);
                                }
                                deleteList(segments);
                            }
                            fclose(cfgHandle.fptr);
                            return -2;
                        }
                        fclose(cfgHandle.fptr);
                        deleteList(errorList);
                        continue;
                    } else if (argv[i][j] == 'd') {
                        if (hasConfig) {
                            // delete segments
                            if (!isDefaultConfig) {
                                for (Node* node = segments->head; node != NULL; node = node->next) {
                                    SegmentDef* segDef = ((SegmentDef*)(node->dataptr));
                                    free(segDef->name);
                                    if (segDef->outputArr != NULL) {free(segDef->outputArr);}
                                }
                            }
                            deleteList(segments);
                            printf("\e[1;31mERROR:\e[0m Multiple configurations proveided\n\n");
                            return -2;
                        }
                        hasConfig = 1;
                        continue;
                    } else if (argv[i][j] == 'b') {
                        if (hasSize) {
                            // delete segments
                            if (!isDefaultConfig) {
                                for (Node* node = segments->head; node != NULL; node = node->next) {
                                    SegmentDef* segDef = ((SegmentDef*)(node->dataptr));
                                    free(segDef->name);
                                    if (segDef->outputArr != NULL) {free(segDef->outputArr);}
                                }
                            }
                            deleteList(segments);
                            printf("\e[1;31mERROR:\e[0m Multiple word sizes proveided\n\n");
                            return -2;
                        }
                        hasSize = 1;
                        wordSize = 2;
                        continue;
                    }
                    else if (argv[i][j] == 'w') {
                        if (hasSize) {
                            // delete segments
                            if (!isDefaultConfig) {
                                for (Node* node = segments->head; node != NULL; node = node->next) {
                                    SegmentDef* segDef = ((SegmentDef*)(node->dataptr));
                                    free(segDef->name);
                                    if (segDef->outputArr != NULL) {free(segDef->outputArr);}
                                }
                            }
                            deleteList(segments);
                            printf("\e[1;31mERROR:\e[0m Multiple word sizes proveided\n\n");
                            return -2;
                        }
                        hasSize = 1;
                        wordSize = 1;
                        continue;
                    } else if (argv[i][j] == 'L') {
                        if (hasEndian) {
                            // delete segments
                            if (!isDefaultConfig) {
                                for (Node* node = segments->head; node != NULL; node = node->next) {
                                    SegmentDef* segDef = ((SegmentDef*)(node->dataptr));
                                    free(segDef->name);
                                    if (segDef->outputArr != NULL) {free(segDef->outputArr);}
                                }
                            }
                            deleteList(segments);
                            printf("\e[1;31mERROR:\e[0m Multiple word sizes proveided\n\n");
                            return -2;
                        }
                        hasEndian = 1;
                        isLittleEndian = 1;
                        continue;
                    } else if (argv[i][j] == 'B') {
                        if (hasEndian) {
                            // delete segments
                            if (!isDefaultConfig) {
                                for (Node* node = segments->head; node != NULL; node = node->next) {
                                    SegmentDef* segDef = ((SegmentDef*)(node->dataptr));
                                    free(segDef->name);
                                    if (segDef->outputArr != NULL) {free(segDef->outputArr);}
                                }
                            }
                            deleteList(segments);
                            printf("\e[1;31mERROR:\e[0m Multiple word sizes proveided\n\n");
                            return -2;
                        }
                        hasEndian = 1;
                        isLittleEndian = 0;
                        continue;
                    } else if (argv[i][j] == 'q') {hasQuote = 1; continue;}
                    else if (argv[i][j] == 'v') {
                        // delete segments
                        if (!isDefaultConfig) {
                            for (Node* node = segments->head; node != NULL; node = node->next) {
                                SegmentDef* segDef = ((SegmentDef*)(node->dataptr));
                                free(segDef->name);
                                if (segDef->outputArr != NULL) {free(segDef->outputArr);}
                            }
                        }
                        deleteList(segments);
                        printVersion();
                        return 0;
                    } else if (argv[i][j] == 'o') {
                        if (outputFileName != NULL) {
                            // delete segments
                            if (!isDefaultConfig) {
                                for (Node* node = segments->head; node != NULL; node = node->next) {
                                    SegmentDef* segDef = ((SegmentDef*)(node->dataptr));
                                    free(segDef->name);
                                    if (segDef->outputArr != NULL) {free(segDef->outputArr);}
                                }
                            }
                            deleteList(segments);
                            printf("\e[1;31mERROR:\e[0m Multiple output file names declared\n\n");
                            return -2;
                        }
                        k++;
                        if (k >= argc || argv[k][0] == '-') {
                            // delete segments
                            if (!isDefaultConfig) {
                                for (Node* node = segments->head; node != NULL; node = node->next) {
                                    SegmentDef* segDef = ((SegmentDef*)(node->dataptr));
                                    free(segDef->name);
                                    if (segDef->outputArr != NULL) {free(segDef->outputArr);}
                                }
                            }
                            deleteList(segments);
                            printf("\e[1;31mERROR:\e[0m Expected output file\n\n");
                            return -2;
                        }
                        outputFileName = argv[k];
                        continue;
                    } else {
                        // delete segments
                        if (!isDefaultConfig) {
                            for (Node* node = segments->head; node != NULL; node = node->next) {
                                SegmentDef* segDef = ((SegmentDef*)(node->dataptr));
                                    free(segDef->name);
                                if (segDef->outputArr != NULL) {free(segDef->outputArr);}
                            }
                        }
                        deleteList(segments);
                        printf("\e[1;31mERROR:\e[0m invalid option -%c\n\n", argv[i][j]);
                        return -2;
                    }
                }
                i = k;
                continue;
            }
        }
        
        // don't open the file twice
        if (fileName != NULL) {
            printf("\e[1;31mERROR:\e[0m Argument \"%s\" unexpected\n\n", argv[i]);
            if (!isDefaultConfig) {
                for (Node* node = segments->head; node != NULL; node = node->next) {
                    SegmentDef* segDef = ((SegmentDef*)(node->dataptr));
                    free(segDef->name);
                    if (segDef->outputArr != NULL) {free(segDef->outputArr);}
                }
            }
            deleteList(segments);
            return -2;
        }
        fileName = memcpy(malloc(strlen(argv[i]) + 1), argv[i], strlen(argv[i]) + 1);
    }

    // handle no config
    if (!hasConfig) {
        printf("\e[1;33mWARNING:\e[0m No configuration spacified, using default (consider using -d option)\n\n");
    }

    // ensure a file was passed in
    if (fileName == NULL) {
        printf("\e[1;31mERROR:\e[0m Expected input file\n\n");
        if (!isDefaultConfig) {
            for (Node* node = segments->head; node != NULL; node = node->next) {
                SegmentDef* segDef = ((SegmentDef*)(node->dataptr));
                free(segDef->name);
                if (segDef->outputArr != NULL) {free(segDef->outputArr);}
            }
        }
        deleteList(segments);
        return -2;
    }

    // set default output file
    if (outputFileName == NULL) {
        outputFileName = "a.out";
    }

    // open the file
    List* errorList = newList();
    List* handles = newList();
    List* macroDeleteTracker = newList();
    FileHandle mainFileHandle = {fopen(fileName, "r"), fileName, 0, 0};

    // ensure handle is valid
    if (mainFileHandle.fptr != NULL) {
        // get the file size
        fseek(mainFileHandle.fptr, 0, SEEK_END);
        mainFileHandle.length = ftell(mainFileHandle.fptr);
        rewind(mainFileHandle.fptr);
        appendList(handles, &mainFileHandle, sizeof(FileHandle));

        // validate the main file
        validateFile(&mainFileHandle, errorList);

        // assemble
        StringTable macros = NULL;
        StringTable vars = NULL;
        if (errorList->size == 0) {macros = readMacros(&mainFileHandle, errorList, handles, macroDeleteTracker);}
        if (errorList->size == 0) {rewind(mainFileHandle.fptr); vars = readGlobalVars(&mainFileHandle, errorList, handles, segments, macros, wordSize);}
        if (errorList->size == 0) {rewind(mainFileHandle.fptr); assemble(&mainFileHandle, errorList, handles, segments, macros, vars, wordSize, isLittleEndian);}

        // output
        if (errorList->size == 0) {
            FILE* output;
            output = fopen(outputFileName, isHex ? "w" : "wb");
            if (output == NULL) {
                printf("\e[1,31mERROR:\e[0m could not open output file\n\n");
            } else {
                long pos = 0;
                for (Node* node = segments->head; node != NULL; node = node->next) {
                    SegmentDef* seg = (SegmentDef*)(node->dataptr);
                    if (seg->align > 1) {
                        if (wordSize == 1) {pos /= 2;}
                        uint16_t buffer = seg->align - (pos % seg->align);
                        if (buffer == seg->align) {buffer = 0;}
                        const uint8_t zero = 0;
                        for (int i = 0; i < buffer * (wordSize == 1 ? 2 : 1); i++) {
                            if (isHex == 1) {
                                int linePos = (pos + i) % 16;
                                if (linePos == 15) {fprintf(output, "00\n");}
                                else if (linePos == 7) {fprintf(output, "00  ");}
                                else {fprintf(output, "00 ");}
                            } else if (isHex == 2 && (i & 0x0001)) {
                                int linePos = (pos + i / 2) % 16;
                                if (linePos == 15) {fprintf(output, "0000\n");}
                                else if (linePos == 7) {fprintf(output, "0000  ");}
                                else {fprintf(output, "0000 ");}
                            } else {fwrite(&zero, 1, 1, output);}
                        }
                        pos += buffer * (wordSize == 1 ? 2 : 1);
                    }
                    if (seg->accessType != bss) {
                        if (seg->fill) {
                            if (isHex) {
                                uint16_t data;
                                for (int i = 0; i < seg->size * (wordSize == 1 ? 2 : 1); i++) {
                                    if (isHex == 1) {
                                        int linePos = (pos + i) % 16;
                                        if (linePos == 15) {fprintf(output, "%02x\n", seg->outputArr[i]);}
                                        else if (linePos == 7) {fprintf(output, "%02x  ", seg->outputArr[i]);}
                                        else {fprintf(output, "%02x ", seg->outputArr[i]);}
                                    } else {
                                        if (i == seg->size * (wordSize == 1 ? 2 : 1) - 1 && !(i & 0x0001)) {
                                            int linePos = (pos + i + 1) % 16;
                                            if (linePos == 15) {fprintf(output, "%04x\n", seg->outputArr[i]);}
                                            else if (linePos == 7) {fprintf(output, "%04x  ", seg->outputArr[i]);}
                                            else {fprintf(output, "%04x ", seg->outputArr[i]);}
                                        }
                                        else if (!(i & 0x0001)) {data = isLittleEndian ? seg->outputArr[i] : (seg->outputArr[i] << 8);}
                                        else {
                                            data |= isLittleEndian ? (seg->outputArr[i] << 8) : seg->outputArr[i];
                                            int linePos = (pos + i) % 16;
                                            if (linePos == 15) {fprintf(output, "%04x\n", data);}
                                            else if (linePos == 7) {fprintf(output, "%04x  ", data);}
                                            else {fprintf(output, "%04x ", data);}
                                        }
                                    }
                                }
                            } else {fwrite(seg->outputArr, 1, seg->size * (wordSize == 1 ? 2 : 1), output);}
                            pos += seg->size * (wordSize == 1 ? 2 : 1);
                        } else {
                            if (isHex) {
                                uint16_t data;
                                for (int i = 0; i < seg->writeAddr; i++) {
                                    if (isHex == 1) {
                                        int linePos = (pos + i) % 16;
                                        if (linePos == 15) {fprintf(output, "%02x\n", seg->outputArr[i]);}
                                        else if (linePos == 7) {fprintf(output, "%02x  ", seg->outputArr[i]);}
                                        else {fprintf(output, "%02x ", seg->outputArr[i]);}
                                    } else {
                                        if (i == seg->writeAddr - 1 && !(i & 0x0001)) {
                                            int linePos = (pos + i + 1) % 16;
                                            if (linePos == 15) {fprintf(output, "%04x\n", seg->outputArr[i]);}
                                            else if (linePos == 7) {fprintf(output, "%04x  ", seg->outputArr[i]);}
                                            else {fprintf(output, "%04x ", seg->outputArr[i]);}
                                        }
                                        else if (!(i & 0x0001)) {data = isLittleEndian ? seg->outputArr[i] : (seg->outputArr[i] << 8);}
                                        else {
                                            data |= isLittleEndian ? (seg->outputArr[i] << 8) : seg->outputArr[i];
                                            int linePos = (pos + i) % 16;
                                            if (linePos == 15) {fprintf(output, "%04x\n", data);}
                                            else if (linePos == 7) {fprintf(output, "%04x  ", data);}
                                            else {fprintf(output, "%04x ", data);}
                                        }
                                    }
                                }
                            } else {fwrite(seg->outputArr, 1, seg->writeAddr, output);}
                            pos += seg->writeAddr;
                        }
                    }
                }
                fclose(output);
            }
        }

        // assembly cleanup
        if (macros != NULL) {deleteStringTable(macros);}
        if (vars != NULL) {deleteStringTable(vars);}
    } else {
        printf("\e[1;31mERROR:\e[0m Could not open %s\n\n", mainFileHandle.name);
        // delete segments
        if (!isDefaultConfig) {
            for (Node* node = segments->head; node != NULL; node = node->next) {
                SegmentDef* segDef = ((SegmentDef*)(node->dataptr));
                        free(segDef->name);
                if (segDef->outputArr != NULL) {free(segDef->outputArr);}
            }
        }
        deleteList(segments);
        return -2;
    }

    // delete macro args
    for (Node* nodei = macroDeleteTracker->head; nodei != NULL; nodei = nodei->next) {
        List* argList = *(List**)(nodei->dataptr);
        for (Node* nodej = argList->head; nodej != NULL; nodej = nodej->next) {
            free(*(char**)(nodej->dataptr));
        }
        deleteList(argList);
    }
    deleteList(macroDeleteTracker);

    // print all errors
    int hasError = errorList->size;
    for (Node* node = errorList->head; node != NULL; node = node->next) {
        ErrorData errorData = *(ErrorData*)(node->dataptr);
        printError(errorData);
        free(errorData.errorMsg);
    }
    deleteList(errorList);

    // close all files
    for (Node* node = handles->head; node != NULL; node = node->next) {
        FileHandle handle = *(FileHandle*)(node->dataptr);
        free(handle.name);
        fclose(handle.fptr);
    }
    deleteList(handles);

    // delete segments
    if (!isDefaultConfig) {
        for (Node* node = segments->head; node != NULL; node = node->next) {
            SegmentDef* segDef = ((SegmentDef*)(node->dataptr));
            free(segDef->name);
            if (segDef->outputArr != NULL) {free(segDef->outputArr);}
        }
    }
    deleteList(segments);

    // fail on errors
    if (hasError) {return -1;}

    // easter egg
    if (hasQuote) {printQuote();}

    // return success
    return 0;
}