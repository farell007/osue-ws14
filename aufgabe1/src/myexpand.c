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
int replaceTabsOfFile(FILE* fp,const int tabstop);

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
    unsigned int firstFile = 1;
	
	//Check if the program exists
    if(argc > 0){
        pgm_name = argv[0];
    } else{
        (void) fprintf(stderr, "Fatal error. There is no program name. Cannot continue.\n");
        exit(EXIT_FAILURE);
    }
	if (parseInput(argc,argv,&tabstop,&firstFile) != 0){
        (void) fprintf(stderr, "Fatal error. There was an error while parsing the user inputs. Cannot continue.");
        exit(EXIT_FAILURE);
	}

	//If no filename is given read from stdin
	if((firstFile == 3 && argc < 4)  || (firstFile == 1 && argc < 2)){
		//read from stdin
		replaceTabsOfFile(stdin,tabstop);
	}

	//open each file and replace all tabs with tabstop spaces
	for(int i = 0; i < argc - firstFile;++i){
		char *filename = argv[firstFile+i];
		FILE *fp;
		if((fp = fopen(filename, "r+")) == 0){
			(void) fprintf(stderr, "%s: The file %s does not exist or you don't have read permissions!\n", pgm_name,filename);
			exit(EXIT_FAILURE);
		} 

		if(replaceTabsOfFile(fp,tabstop) != 0){
			(void) fprintf(stderr, "Fatal error. There was an error while writing to the file '%s'. Cannot continue.",filename);
			exit(EXIT_FAILURE);
		}

		if(fclose(fp) == EOF){
			(void) fprintf(stderr, "%s: Error closing the file '%s'!\n", pgm_name,filename);
			exit(EXIT_FAILURE);
		}


	}

    return EXIT_SUCCESS;
}

/**
 * Parse the user input of the program.
 *
 * @brief Parse the user input of the program
 * @detail if the flag "-t" is set the "tabstop" variable get adjustet with the following value and the "firstFile" variable also is increased by 2. If there is a parsing error the program terminates.
 * @param argc The number of command-line parameters in argv
 * @param argv The array of command-line parameters, argc elements long.
 * @param tabstop Address to the number of spaces which one tab should get replaced
 * @param firstFile Address to the position of the first filename in the argv array 
 * @return 0 on success, non-zero on failure.
 */
int parseInput(int argc, char **argv, unsigned int *tabstop, unsigned int *firstFile){

	if ( argc < 2 )
		return 0; /*Read from stdin*/
    //Check if the -t flag is set
    if (argv[1][0] == '-'){
		//We found a flag! Check if it is t
		if(argv[1][1] == 't'){
			//Check if there are filenames after the flag
			if(argc < 3){
				(void) fprintf(stderr, "%s: Too less arguments.\n%s\n", pgm_name, usage);
				exit(EXIT_FAILURE);
			}

			long buff = strtol(argv[2],NULL,10);
			if (buff >= 0 && buff <= INT_MAX) {
				*tabstop = buff;
				*firstFile = 3;

				if(argc < 4)
					return 0; /* Read from stdin */
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
		if( access( argv[*firstFile+i], R_OK  ) != -1 ) {
			// file exists
		} else {
		    // file doesn't exist
			(void) fprintf(stderr, "%s: The file %s does not exist or you don't have read permissions!\n%s\n", pgm_name,argv[*firstFile+i],usage);
			exit(EXIT_FAILURE);
		}
	}
	return 0;
}


/**
 * Replaces all tabs of the given file with tabstop spaces and prints it onto the standard output
 * @brief Replaces all tabs of the given file with tabstop spaces
 * @detail iterates over every character of the stream to check if it is the tab character '\t' and replaces it with number of tabstop spaces. Prints the file to the standard output. The file pointer doesn't get closed! 
 * @param fp The pointer of the file which tabs are getting replaced, could also be stdin!
 * @param tabstop the number of spaces a tab gets replaced
 * @return 0 if everything worked out well, otherwise it prints an error message and returns -1
 */
int replaceTabsOfFile(FILE *fp,const int tabstop){
	char nChar;
	char tabs[tabstop];
//	char *content;

	//fill array tabs with spaces
	for(int i = 0; i < tabstop; ++i){
		tabs[i] = '#';
	}

	while((nChar = fgetc(fp)) != EOF){
		if(nChar == '\t'){
			//Write to stdout
			fwrite(tabs, sizeof(char),NRELEMENTS(tabs),stdout);	
		}
		else{
			fputc(nChar,stdout);
		}
	}

	return 0;
}
