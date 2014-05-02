/**
 * @file child.h
 * @author David Pfahler (1126287) <e1126287@student.tuwien.ac.at>
 * @brief This headerfilchildains the Prototypes, Constants and Macros of "child.c"
 * @date 26.04.2014
 */

/**
 * prevent multible inclusion
 */
#ifndef dp_child_h  
#define dp_child_h

#include "calculator.h"

/* CONSTANTS */

/* MACROS */

/* TYPEDEF */

/**
 * @brief the operator of the calculation
 */
typedef enum {
	/*! @brief addition*/
	plus,
	/*! @brief subtraction*/
	minus,
	/*! @brief division*/
	divide,
	/*! @brief multiplication*/
	time
} operator;

/* GLOBAL VARIABLES */

/**
*@brief global variable for reading from the pipe from the parent process
*/
FILE * reading;

/**
*@brief global variable for writing to the pipe to the parent process
*/
FILE * writing;

/* PROTOTYPES */

/**
 * @brief exits the child process and closes all resources
 */
void bail_out_child(int eval, const char * fmt, ...); 

/**
 * @brief free all resources from the child
 * @details closes pipes: global variables (reading, writing)
 */
void free_child_resources( void );

/**
 * @brief the main function of the child process. this method is called from the main function of calculator 
 * @details pipes the pipes for the communication between the child process and the parent process
 */
void childProcess( void );


#endif /*ifndef dp_child_h*/
