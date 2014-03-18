/**
 * @file myexpand.c  
 * @author David Pfahler (1126287) <e1126287@student.tuwien.ac.at> 
 * @date Tue Mar 18 11:22:07 CET 2014
 */

/* === INCLUDES === */
#include <stdio.h>
#include <stdlib.h>
#include <limits.h> /* for INT_MIN, INT_MAX */

/* === CONST === */
char* pgm_name;

/* === PROTOTYPES === */
char **parseInput(int argc, char **argv, unsigned int *tabstop);

/**
 * The main entry point of the program.
 *
 * @param argc The number of command-line parameters in argv.
 * @param argv The array of command-line parameters, argc elements long.
 * @return The exit code of the program. 0 on success, non-zero on failure.
 */
int main(int argc, char **argv)
{
    if(argc > 0){
        pgm_name = argv[0];
    } else{
        (void) fprintf(stderr, "Fatal error. There is no program name Cannot continue.\n");
        exit(EXIT_FAILURE);
    }
    unsigned int tabstop = 8;
    char** files = parseInput(argc,argv,&tabstop);
    if(!files){
        exit(EXIT_FAILURE);    
    }
    printf("Tabstop = %d!\n",tabstop);
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
char **parseInput(int argc, char **argv, unsigned int *tabstop){
    char* usage = "SYNOPSIS:\n\tmyexpand [-t tabstop] [file...]";
    int firstFile = 1;
    //Check if there are too less arguments
    if (argc < 2){
        (void) fprintf(stderr, "%s: Too less arguments.\n%s\n", pgm_name, usage);
        return NULL; 
    }
    printf("%s\n",strcmp(argv[1],"-t"));
    //Check if the -t flag is set
    if (strcmp(argv[1],"-t") == 0){
        long buff = strtol(argv[2],NULL,10);
        if (buff >= INT_MIN && buff <= INT_MAX) {
            *tabstop = buff;
            firstFile = 3;
            (void) printf("%d\n",buff);
        }
        else {
            (void) fprintf(stderr, "%s: The tabstop was too big.\n", pgm_name);
            return NULL;
        }
    }
    else{
        (void) printf("%s\n",argv[1]);
    }
	return argv+firstFile;

}
