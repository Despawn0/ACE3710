
# Overview

The header files contained within this directory are used as headers in the process on a CR16 assembler

The process uses the following steps: First Macro Pass -> Variable Evaluation -> Assembly

Written by Adam Billings

# Expression Evaluation

The expression evaluation header contains all needed code to evaluate constant integer expressions found in assembly code
Expression evaluation contains one needed struct for error propagation:
    - ExprErrorShort

Expression evaluation contains only one function directed toward the outside:
    - evalShortExpr

# General Assembler

General types used by the assembler are given in MiscAssembler.h

# General Macro

General Macro types and functions are given in GeneralMacro.h

# Macro Processing

Macros are processed in the processMacros.h file using the following functions:
    - executeType1Macro
    - executeType2Macro
    - executeType3Macro


# Code Generation

Functions to generate instructions are provided in the CodeGeneration.h file

# Configuration Reading

Configuration is read in the ConfigReader.h file
The folling function is used:
    - readConfigFile

# First Macro Pass

The first macro pass reads files and defined (type 1) macros into memory
The following function is used:
    - readMacros

# Variable Evaluation

Variable evaluation is written in VarEvaluation.h file
Note that type 2 macros are processed here

The following functions are used outside the file:
    - readGlobalVars
    - readLocalVars

# Assembly

The file is assembled by code in the Assemble.h file
Note that type 3 macros are processed here

The following function is used:
    -assemble