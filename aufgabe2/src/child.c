/**
 * @file child.c
 * @author David Pfahler (1126287) <e1126287@student.tuwien.ac.at>
 * @brief This file contains the implementation of the child process of the calculator. It parses the input strings and does the calculations. 
 * @date 27.04.2014
 */

#include "calculator.h"
/**
 * @brief the first operand
 */
long operand1;

/**
 * @brief the second operand
 */
long operand2;

/**
 * @brief the operator
 */
operator op;



/* STATIC FUNCTIONS */

/**
 * @brief extract the operands and the operator from the input string
 * @details saves the operands into the global variables "operand1" and "operand2" and the operator into the global variable "op"
 * @param input the input string in the form: "<zahl1> <zahl2> <operator>"
 */
static void parse_arguments(char* input){
	char *tokens = strtok(input, " ");
	char* endptr;

	operand1 = strtol(tokens,&endptr, 10);
	if(endptr == tokens){
		usage();
		bail_out_child(EXIT_FAILURE,"parsing of operand 1 failed");
	}
	if(operand1 < LONG_MIN || operand1 > LONG_MAX){
		usage();
		bail_out_child(EXIT_FAILURE,"parsing of operand 1 failed");
	}
	
	operand2 = strtol(strtok(NULL," "),&endptr, 10);
	if(endptr == tokens){
		usage();
		bail_out_child(EXIT_FAILURE,"parsing of operand 2 failed");
	}
	if(operand2 < LONG_MIN || operand2 > LONG_MAX){
		usage();
		bail_out_child(EXIT_FAILURE,"parsing of operand 2 failed");
	}

	char *c_op = strtok(NULL, " ");

	switch(c_op[0]){
		case '+':
			op = plus;
			break;
		case '-':
			op = minus;
			break;
		case '*':
			op = time;
			break;
		case '/':
			op = divide;
			break;
		default:
			usage();
			bail_out_child(EXIT_FAILURE,"parsing of operator 2 failed");
			break;
	}

	DEBUG("o1 = %ld, o2 = %ld, op = %d\n",operand1, operand2, op);

}

/* IMPLEMENTATIONS */

void free_child_resources( void ){
	DEBUG("Start closing child process\n");
	if( fclose(writing) != 0){
		int errcode = errno;
		(void) fprintf(stderr, "%s: ", program_name);
		(void) fprintf(stderr,"closing writing pipe error. Code: %s\n", strerror(errcode));
	}
	if( fclose(reading) != 0){
		int errcode = errno;
		(void) fprintf(stderr, "%s: ", program_name);
		(void) fprintf(stderr,"closing reading pipe error. Code: %s\n", strerror(errcode));
	}
	DEBUG("child closed\n");
}

void bail_out_child(int eval, const char * fmt, ...){
	DEBUG("bail out child started\n");
	free_child_resources();
	va_list arglist;
	va_start(arglist, fmt);
	bail_out(eval,fmt,arglist);
	va_end(arglist);
}

void childProcess( void ){

	DEBUG("starting child process\n");

	reading = fdopen(pipes[CHILD][READ], "r");
	if (reading == NULL) {
		bail_out_child(EXIT_FAILURE,"child failed reading pipe");
	}
	writing = fdopen(pipes[PARENT][WRITE], "w");
	if (writing == NULL) {
		bail_out_child(EXIT_FAILURE,"child failed writing pipe");
	}

	if(close(pipes[CHILD][WRITE]) != 0) {
		bail_out_child(EXIT_FAILURE,"close + 1 failed");
	}
	if(close(pipes[PARENT][READ]) != 0) {
		bail_out_child(EXIT_FAILURE,"close + 2 failed");
	}
	
	char readbuffer[INPUT_BUFFER_LENGTH + 2];
	char result[RESULT_BUFFER_LENGTH + 1];

	while(fgets(readbuffer, INPUT_BUFFER_LENGTH + 2 , reading) != NULL){
		DEBUG("child received: %s\n",readbuffer);
		
		(void) parse_arguments(readbuffer);
		
		long val = 0;

		switch (op){
			case plus:
				val = operand1 + operand2;
				break;
			case minus:
				val = operand1 - operand2;
				break;
			case time:
				val = operand1 * operand2;
				break;
			case divide:
				val = operand1 / operand2;
				break;
			default:
				assert(0);
		}

		snprintf(result,RESULT_BUFFER_LENGTH,"%ld",val);
		
		if( fprintf(writing, "%s\n",result) < 0){
			bail_out_child(EXIT_FAILURE,"writing to the parent pipe failed");
		}
		if( fflush(writing) != 0){
			bail_out_child(EXIT_FAILURE,"flushing the child pipe failed");
		}
	}

	free_child_resources();
}
