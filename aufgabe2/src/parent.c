/**
 * @file parent.c
 * @author David Pfahler (1126287) <e1126287@student.tuwien.ac.at>
 * @brief This file contains the implementation of the parent process of the calculator. It waits for user input on the stdin and prints the output from the child process to the stdout 
 * @date 27.04.2014
 */

#include "calculator.h"


/* STATIC FUNCTIONS */

void free_parent_resources( void ){
	pid_t pid;
	int status;

	DEBUG("Start closing parent process\n");

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
	
	DEBUG("Wait for the child to close\n");

	pid = wait(&status);
	
	if(WEXITSTATUS(status) != EXIT_SUCCESS){
		(void) fprintf(stderr, "%s: ", program_name);
		(void) fprintf(stderr,"child with pid %d returned exit code %d.\n", pid, WEXITSTATUS(status));
		exit(EXIT_FAILURE);
	}
	

	DEBUG("parent closed\n");
}

/* IMPLEMENTATIONS */

void bail_out_parent(int eval, const char * fmt, ...){
	DEBUG("bail out parent started\n");
	free_parent_resources();
	va_list arglist;
	va_start(arglist, fmt);
	bail_out(eval,fmt,arglist);
	va_end(arglist);
}

void parentProcess( void ){

	DEBUG("starting parent process\n");

	reading = fdopen(pipes[PARENT][READ], "r");
	if (reading == NULL) {
		bail_out_parent(EXIT_FAILURE,"parent failed reading pipe");
	}
	writing = fdopen(pipes[CHILD][WRITE], "w");
	if (writing == NULL) {
		bail_out_parent(EXIT_FAILURE,"parent failed writing pipe");
	}

	if(close(pipes[CHILD][READ]) != 0) {
		bail_out_parent(EXIT_FAILURE,"close + 1 failed");
	}
	if(close(pipes[PARENT][WRITE]) != 0) {
		bail_out_parent(EXIT_FAILURE,"close + 2 failed");
	}

	/* Get Input */
	char input[INPUT_BUFFER_LENGTH + 2];
	char result[RESULT_BUFFER_LENGTH + 1];	

	while(fgets(input,INPUT_BUFFER_LENGTH + 2, stdin) != NULL){
		DEBUG("parent received: %s\n",input);
		if( fprintf(writing, "%s", input)<0){
			bail_out_parent(EXIT_FAILURE,"writing to child via pipe failed");
		}
		if( fflush(writing) != 0){
			bail_out_parent(EXIT_FAILURE,"flushing the pipe to child failed");
		}
		DEBUG("parent sent: %s to child\n",input);

		if( fgets(result, RESULT_BUFFER_LENGTH, reading) != NULL){
			DEBUG("parent received %s from child\n",result);
			(void) fprintf(stdout, "%s", result);
		} else{
			bail_out_parent(EXIT_FAILURE,"the reading of the result from the client got an error");
		}
	}

	if ( feof(stdin) == 0 ){
		bail_out_parent(EXIT_FAILURE,"reading from stdin failed");
	}

	free_parent_resources();
}
