/**
 * @file mysum.c
 * @author David Pfahler (1126287) <david@pfahler.at>
 * @date 09.04.2014
 * @brief creates a sum of min 1 and max 4 arguments
 */

/* INCLUDES */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

/* MACROS and CONST*/

#define MAXARGS (4)

/* TYPES */

/* GLOBALS */
static char* progname = "mysum";
static const char usage[] = "USAGE\n\tmysum <number 1> [number 2] [number 3] [number 4]";

/* PROTOTYPES */
static void parse_args(const int argc, char **argv, int *cnumbers, long *numbers);
static void bail_out(int error, const char *fmt, ...);

int main(int argc, char **argv){
	int cnumbers;
	long numbers[MAXARGS];
	long sum = 0;
	(void) parse_args(argc,argv,&cnumbers,numbers);
	for(int i = 0; i < cnumbers; ++i){
		sum += numbers[i];
	}
	printf("%ld\n",sum);
	exit(EXIT_SUCCESS);
}

static void parse_args(const int argc, char **argv, int *cnumbers, long *numbers){
	if(argc < 1){
		(void) bail_out(EXIT_FAILURE, "Too less arguments!\n%s",usage);
	}
	else{
		progname = argv[0];
	}
	if(argc < 2){
		(void) bail_out(EXIT_FAILURE, "Too less arguments!\n%s",usage);
	}

	
	*cnumbers = argc - 1;
	if(*cnumbers > MAXARGS){
		(void) bail_out(EXIT_FAILURE, "Too many arguments!\n%s",usage);
	}
	//parsing numbers:
	for(int i = 0; i < *cnumbers; ++i){
		char *endptr;
		char *str = argv[i+1];
		long val;
		errno = 0;
		val = strtol(str,&endptr, 10);
		
		if((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) 
			|| (errno != 0 && val == 0)){
			(void) bail_out(EXIT_FAILURE, "Parsing of number %d failed!\n%s",i+1,usage);
		}
		if(endptr == str){
			(void) bail_out(EXIT_FAILURE, "Parsing of number %d failed! No numbers where found.\n%s",i+1,usage);
		}
		if(*endptr != '\0'){
			(void) bail_out(EXIT_FAILURE, "Further characters after the number %d: %s\n%s",i+1,endptr,usage);
		}

		numbers[i] = val;
	}
}


static void bail_out(int error, const char *fmt, ...){
	va_list ap;

	(void) fprintf(stderr, "%s: ",progname);
	if(fmt != NULL){
		va_start(ap, fmt);
		(void) vfprintf(stderr, fmt, ap);
		va_end(ap);
	}
	if(errno != 0){
		(void) fprintf(stderr, ": %s", strerror(errno));
	}
	(void) fprintf(stderr, "\n");
	exit(error);
}
