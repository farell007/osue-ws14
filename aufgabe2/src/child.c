/**
 * @file child.c
 * @author David Pfahler (1126287) <e1126287@student.tuwien.ac.at>
 * @brief This file contains the implementation of the child process of the calculator. It parses the input strings and does the calculations. 
 * @date 27.04.2014
 */

#include "calculator.h"

/**
*@brief global variable for reading from the pipe from the parent process
*/
FILE * reading;

/**
*@brief global variable for writing to the pipe to the parent process
*/
FILE * writing;

/**
 * @brief gloabal variable for cleanup
 */
int * pipes_global;

/* STATIC FUNCTIONS */

static void free_child_resources( void ){
	DEBUG("Start closing child process\n");
	(void) fclose(writing);
	(void) fclose(reading);
	(void) close(*(pipes_global) + 0);
	(void) close(*(pipes_global) + 3);
	DEBUG("child closed\n");
}

static void bail_out_child(int eval, const char * fmt, ...){
	DEBUG("bail out child started");
	free_child_resources();
	va_list arglist;
	va_start(arglist, fmt);
	bail_out(eval,fmt,arglist);
	va_end(arglist);
}


/* IMPLEMENTATIONS */

void childProcess( int* pipes){

	DEBUG("starting child process\n");

	pipes_global = pipes;

	reading = fdopen(*(pipes + 0), "r");
	if (reading == NULL) {
		bail_out_child(EXIT_FAILURE,"child failed reading pipe");
	}
	writing = fdopen(*(pipes + 3), "w");
	if (writing == NULL) {
		bail_out_child(EXIT_FAILURE,"child failed writing pipe");
	}

	if(close(*(pipes + 1)) != 0) {
		bail_out_child(EXIT_FAILURE,"close + 1 failed");
	}
	if(close(*(pipes + 2)) != 0) {
		bail_out_child(EXIT_FAILURE,"close + 2 failed");
	}
	
	char readbuffer[INPUT_BUFFER_LENGTH + 1];
	char result[RESULT_BUFFER_LENGTH + 1];

	while(fgets(readbuffer, INPUT_BUFFER_LENGTH, reading) != NULL){
		DEBUG("child received: %s",readbuffer);
		
		sprintf(result,"%s",readbuffer);
		
		if( fprintf(writing, "%s",result) < 0){
			bail_out_child(EXIT_FAILURE,"writing to the parent pipe failed");
		}
		if( fflush(writing) != 0){
			bail_out_child(EXIT_FAILURE,"flushing the child pipe failed");
		}
	}
	
	if (feof(reading) != 0){
		bail_out_child(EXIT_FAILURE,"reading from the parent pipe failed");
	}

	free_child_resources();
}
