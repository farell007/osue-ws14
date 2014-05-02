/**
 * @file calculator.c
 * @author David Pfahler (1126287) <e1126287@student.tuwien.ac.at>
 * @brief This program calculates simple equations with two operants and one operator
 * @date 24.04.2014
 */

#include "calculator.h"


/* STATIC FUNCTIONS */


/*IMPLEMENTATIONS*/

void bail_out(int eval, const char *fmt, ...){
	
	va_list ap;

	(void) fprintf(stderr, "%s: ", program_name);
	if (fmt != NULL) {
		va_start(ap,fmt);
		(void) vfprintf(stderr, fmt, ap);
		va_end(ap);
	}
	if (errno != 0){
		(void) fprintf(stderr, ": %s", strerror(errno));
	}
	(void) fprintf(stderr, "\n");

	exit(eval);

}

void usage(void){
	(void) fprintf(stderr, "%s: SYNOPSIS:\n"
	"\tcalculator\n"
	"\t$> <zahl1> <zahl2> <operator>\n\n"
	"BNF:\n"
	"\t<zahl>\t::= -?[0-9]+\n"
	"\t<operator>\t::= +|-|*/\n", program_name);
}

/* MAIN FUNCTION*/

/**
 * @brief The program is forked into a parent and a child process.
 * The parent handles the input of the calculations and sends them
 * to the child process. This process parses the input and calculates
 * it. The result gets sent back to the parent and printed onto stdout.
 *
 * @param argc The argument counter
 * @param argv The argument vector
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on failure
 */
int main(int argc, char *argv[]){

	if(argc != 1){
		usage();
		bail_out(EXIT_FAILURE,"wrong usage.");
	} else{
		program_name = argv[0];
	}

	if (pipe((int *)&pipes[PARENT]) != 0) {
		bail_out(EXIT_FAILURE,"Creation of pipe 1 failed");
	}
	if (pipe((int *)&pipes[CHILD]) != 0) {
		bail_out(EXIT_FAILURE,"Creation of pipe 2 failed");
	}

	//fork program
	pid_t pid = fork();
	//from here the program is seperated into two programms

	switch (pid) {
	case -1:
		bail_out(EXIT_FAILURE,"can't fork");
		break;
	case 0:
		/* start child */
		childProcess();
		exit(EXIT_SUCCESS);
		break;
	default:
		/* start parent  */
		parentProcess();
		exit(EXIT_SUCCESS);
		break;
	}
	/* This should never happen*/
	assert(0);
}
