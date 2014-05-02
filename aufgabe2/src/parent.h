/**
 * @file parent.h
 * @author David Pfahler (1126287) <e1126287@student.tuwien.ac.at>
 * @brief This headerfile contains the Prototypes, Constants and Macros of "parent.c"
 * @date 26.04.2014
 */

/**
 * prevent multible inclusion
 */ 
#ifndef dp_parent_h 
#define dp_parent_h

#include "calculator.h"

/* CONSTANTS */

/* MACROS */

/* GLOBAL VARIABLES */

/**
*@brief global variable for reading from the pipe from the child process
*/
FILE * reading;

/**
*@brief global variable for writing to the pipe to the child process
*/
FILE * writing;

/* PROTOTYPES */

/**
 * @brief free all resources from the parent and closes the child stream
 * @details closes pipes: global variables "writing" and "reading"
 */
void free_parent_resources( void );

/**
 * @brief exits the parent process and closes all resources
 */
void bail_out_parent(int eval, const char * fmt, ...); 

/**
 * @brief the main function of the parent process. this method is called from the main function of calculator 
 * @details global variable pipes the pipes for the communication between the child process and the parent process
 */
void parentProcess( void );

#endif /*ifndef dp_parent_h*/
