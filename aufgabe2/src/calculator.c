/**
 * @file calculator.c
 * @author David Pfahler (1126287) <e1126287@student.tuwien.ac.at>
 * @brief This program calculates simple equations with two operants and one operator
 * @date 24.04.2014
 */

#include "calculator.h"

/* STATIC FUNCTIONS */

static void free_resources(){
	//TODO
}

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

	free_resources();
	exit(eval);

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
	return 0;
}

