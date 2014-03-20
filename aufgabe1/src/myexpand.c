/**
 * @file myexpand.c  
 * @author David Pfahler (1126287) <e1126287@student.tuwien.ac.at> 
 * @date Tue Mar 18 11:22:07 CET 2014
 */

/* === INCLUDES === */
#include <stdio.h>
#include <stdlib.h>
#include <limits.h> /* for INT_MIN, INT_MAX */
#include <unistd.h> /* for access() */

/* === MACTROS === */
#define NRELEMENTS(a) (sizeof(a) / sizeof(a[0]))

/* === CONST === */
static char* pgm_name;
static const char usage[] = "SYNOPSIS:\n\tmyexpand [-t tabstop] [file...]";

/* === PROTOTYPES === */
int parseInput(int argc, char **argv, unsigned int *tabstop, unsigned int *firstFile);

/**
 * The main entry point of the program.
 *
 * @param argc The number of command-line parameters in argv.
 * @param argv The array of command-line parameters, argc elements long.
 * @return The exit code of the program. 0 on success, non-zero on failure.
 */
int main(int argc, char **argv)
{
    unsigned int tabstop = 8;
    static const char usage[] = "SYNOPSIS:\n\tmyexpand [-t tabstop] [file...]";
    int firstFile = 1;
	
	//Check if the program exists
    if(argc > 0){
        pgm_name = argv[0];
    } else{
        (void) fprintf(stderr, "Fatal error. There is no program name Cannot continue.\n");
        exit(EXIT_FAILURE);
    }
    //Check if there are too less arguments
    if (argc < 2){
        (void) fprintf(stderr, "%s: Too less arguments.\n%s\n", pgm_name, usage);
        exit(EXIT_FAILURE);
    }
    //Check if the -t flag is set
    if (argv[1][0] == '-'){
		//We found a flag! Check if it is t
		if(argv[1][1] == 't'){
			long buff = strtol(argv[2],NULL,10);
			if (buff >= 0 && buff <= INT_MAX) {
				tabstop = buff;
				firstFile = 3;
			}
			else {
				(void) fprintf(stderr, "%s: The tabstop was too big.\n", pgm_name);
				exit(EXIT_FAILURE);
			}
		}
		//This was not the t flag
		else{
			(void) fprintf(stderr, "%s: The flag %c is unknown!\n%s\n", pgm_name,argv[1][1],usage);
			exit(EXIT_FAILURE);
		}
	}

	//Now check if all files exist
	
	for(int i = 0; i < argc - firstFile;++i)
	{
		if( access( argv[firstFile+i], W_OK  ) != -1 ) {
			// file exists
		} else {
		    // file doesn't exist
			(void) fprintf(stderr, "%s: The file %s does not exist or you don't have write permissions!\n%s\n", pgm_name,argv[firstFile+i],usage);
			exit(EXIT_FAILURE);
		}
	}

    return EXIT_SUCCESS;
}

int parseInput(int argc, char **argv, unsigned int *tabstop, unsigned int *firstFile){
	
    //Check if there are too less arguments
    if (argc < 2){
        (void) fprintf(stderr, "%s: Too less arguments.\n%s\n", pgm_name, usage);
        exit(EXIT_FAILURE);
    }
    //Check if the -t flag is set
    if (argv[1][0] == '-'){
		//We found a flag! Check if it is t
		if(argv[1][1] == 't'){
			long buff = strtol(argv[2],NULL,10);
			if (buff >= 0 && buff <= INT_MAX) {
				*tabstop = buff;
				*firstFile = 3;
			}
			else {
				(void) fprintf(stderr, "%s: The tabstop was too big.\n", pgm_name);
				exit(EXIT_FAILURE);
			}
		}
		//This was not the t flag
		else{
			(void) fprintf(stderr, "%s: The flag %c is unknown!\n%s\n", pgm_name,argv[1][1],usage);
			exit(EXIT_FAILURE);
		}
	}

	//Now check if all files exist
	
	for(int i = 0; i < argc - *firstFile;++i)
	{
		if( access( argv[*firstFile+i], W_OK  ) != -1 ) {
			// file exists
		} else {
		    // file doesn't exist
			(void) fprintf(stderr, "%s: The file %s does not exist or you don't have write permissions!\n%s\n", pgm_name,argv[*firstFile+i],usage);
			exit(EXIT_FAILURE);
		}
	}
	return 0;
}

