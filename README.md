
# About

ACE3710 is an assembler written for CS/ECE 3710 at the University of Utah.
All files were written by Adam Billings in 2024.

# Compiling

It is suggested that the assembler be compiled using the GNU C Compiler (gcc).
To compile, run the following command (this has been tested with O2 optimizations):
    gcc -O2 -o ace3710 ACE3710.c

Verify successful compilation by running the following:
    ./ace3710 --version

# Using the assembler

To use the assembler, simple run the executable with arguments.
All options are documented in the help page.
It is recommended that new users view the "sample-assembly" and "default-config" help pages.
To access the help page, run the following:
    ./ace3710 --help

The following are provided to assist in basic assembly (note that "-q" is optional):
    ./ace3710 -Twd INPUT_FILE_NAME -q
    ./ace3710 -Tw -c CONFIG_FILE_NAME INPUT_FILE_NAME -q
    ./ace3710 -Twd -o OUTPUT_FILE_NAME INPUT_FILE_NAME -q
    ./ace3710 -Tw -c CONFIG_FILE_NAME -o OUTPUT_FILE_NAME INPUT_FILE_NAME -q


# A Note on file extensions

The standard extension for a configuration file is *.cfg
The standard extension for an assembly file is either *.s or *.asm
