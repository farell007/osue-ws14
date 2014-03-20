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
int replaceTabsOfFile(const char* filename,const int tabstop);

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

	//open each file and replace all tabs with tabstop spaces
	
	for(int i = 0; i < argc - firstFile;++i){
		if(replaceTabsOfFile(argv[firstFile+i],tabstop) != 0){
			(void) fprintf(stderr, "Fatal error. There was an error while writing to the file '%s'. Cannot continue.",argv[firstFile+i]);
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
	
    //Check if there are too less arguments
    if (argc < 2){
        (void) fprintf(stderr, "%s: Too less arguments.\n%s\n", pgm_name, usage);
        exit(EXIT_FAILURE);
    }
    //Check if the -t flag is set
    if (argv[1][0] == '-'){
		//We found a flag! Check if it is t
		if(argv[1][1] == 't'){
			//Check if there are filenames after the flag
			if(argc < 4){
				(void) fprintf(stderr, "%s: Too less arguments.\n%s\n", pgm_name, usage);
				exit(EXIT_FAILURE);
			}

			long buff = strtol(argv[2],NULL,10);
			if (buff >= 0 && buff <= INT_MAX) {
				*tabstop = buff;
				*firstFile = 3;
			}
		}
		//This was not the t flag
		else{
			(void) fprintf(stderr, "%s: The flag %c is unknown!\n%s\n", pgm_name,argv[1][1],usage);
			exit(EXIT_FAILURE);
		}
		if(argc < 3){
			(void) fprintf(stderr, "%s: Too less arguments.\n%s\n", pgm_name, usage);
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


int replaceTabsOfFile(const char* filename,const int tabstop){
	int nChar;
	FILE *file_open;
	char tabs[tabstop];
	//fill array tabs with spaces
	for(int i = 0; i < tabstop; ++i){
		tabs[i] = ' ';
	}

	if((file_open = fopen(filename, "r+")) == 0){
		(void) fprintf(stderr, "%s: The file %s does not exist or you don't have write permissions!\n", pgm_name,filename);
		return -1;
	} 
	// else it worked:
	printf("File: %s opened\n",filename);
	
	while((nChar = fgetc(file_open)) != EOF){
		if(nChar == '\t'){
			fwrite(tabs, sizeof(tabs[0]),NRELEMENTS(tabs),file_open);	
		}
	}

	fclose(file_open);
	return 0;
}
