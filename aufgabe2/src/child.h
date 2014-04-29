/**
 * @file child.h
 * @author David Pfahler (1126287) <e1126287@student.tuwien.ac.at>
 * @brief This headerfile contains the Prototypes, Constants and Macros of "child.c"
 * @date 26.04.2014
 */

#ifndef dp_child_h /*prevent multible inclusion*/ 
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

/* PROTOTYPES */

/**
 * @brief exits the parent process and closes all resources
 */
void bail_out_child(int eval, const char * fmt, ...); 

/**
 * @brief free all resources from the child
 * @details closes pipes
 */
void free_child_resources( void );

/**
 * @brief the main function of the child process. this method is called from the main function of calculator 
 * @param pipes the pipes for the communication between the child process and the parent process
 */
void childProcess( int* pipes);


#endif /*ifndef dp_child_h*/
