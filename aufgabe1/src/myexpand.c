/**
 * @file myexpand.c  
 * @author David Pfahler (1126287) <e1126287@student.tuwien.ac.at> 
 * @date Tue Mar 18 11:22:07 CET 2014
 */

/* === INCLUDES === */
#include <stdio.h>

/* === CONST === */

/* === PROTOTYPES === */
char **parseInput(int argc, char **argv, int *tabstop);

/**
 * The main entry point of the program.
 *
 * @param argc The number of command-line parameters in argv.
 * @param argv The array of command-line parameters, argc elements long.
 * @return The exit code of the program. 0 on success, non-zero on failure.
 */
int main(int argc, char **argv)
{
    int tabstop = 8;
    char** files = parseInput(argc,argv,&tabstop);
    if(!files){
        exit(EXIT_FAILURE);    
    }
    printf("Hello, world!\n");
    return 0;
}

/**
 * @brief Parses the command line inputs
 * @detail if the -t flag is set the variable tabstop gets changed.
 * @param argc The number of command-line parameters in argv.
 * @param argv The array of command-line parameters, argc elements long.
 * @param tabstop The number of spaces to replace one tab
 * @return Pointer to an array of files
 */
char **parseInput(int argc, char **argv, int *tabstop){
   return NULL; 
}
