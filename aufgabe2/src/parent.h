/**
 * @file parent.h
 * @author David Pfahler (1126287) <e1126287@student.tuwien.ac.at>
 * @brief This headerfile contains the Prototypes, Constants and Macros of "parent.c"
 * @date 26.04.2014
 */

#ifndef dp_parent_h /*prevent multible inclusion*/ 
#define dp_parent_h

#include "calculator.h"

/* CONSTANTS */

/* MACROS */

/* GLOBAL VARIABLES */

/* PROTOTYPES */

/**
 * @brief free all resources from the parent
 * @details closes pipes
 */
void free_parent_resources( void );

/**
 * @brief exits the parent process and closes all resources
 */
void bail_out_parent(int eval, const char * fmt, ...); 

/**
 * @brief the main function of the parent process. this method is called from the main function of calculator 
 * @param pipes the pipes for the communication between the child process and the parent process
 */
void parentProcess( int* pipes);

#endif /*ifndef dp_parent_h*/
