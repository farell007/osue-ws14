/**
 * @file calculator.h
 * @author David Pfahler (1126287) <e1126287@student.tuwien.ac.at>
 * @brief This headerfile contains the Prototypes, Constants and Macros of "calculator.c"
 * @date 24.04.2014
 *
 * Headerfile for the main file "calculator.c"
 */

#ifndef dp_calculator_h /*prevent multible inclusion*/ 
#define dp_calculator_h

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <limits.h>

#include "child.h"
#include "parent.h"
/* CONSTANTS */

/**
 * @brief The length of the input string that is received with stdin
 */
#define INPUT_BUFFER_LENGTH		(15)
/**
 * @brief the length of the result string from the child process
 */
#define RESULT_BUFFER_LENGTH 	(15)

/**
 * @brief The index of the the pipe of the parent process
 */
#define PARENT 			(0)

/**
 * @brief The index of the the pipe of the child process
 */
#define CHILD 			(1)

/**
 * @brief The index of the write end of the pipe
 */
#define WRITE 			(1)

/**
 * @brief The index of the read end of the pipe
 */
#define READ 			(0)

/* MACROS */

/**
 * @def DEBUG(...)
 * @brief Prints formatted debug message to stderr
 */
#ifdef ENDEBUG /*Flag to set from compiler for debugging*/
#define DEBUG(...) do { fprintf(stderr, __VA_ARGS__);} while(0)
#else
#define DEBUG(...)
#endif

/* GLOBAL VARIABLES */

/**
 * @brief Program name for usage and error messages
 */
char* program_name;

/**
 * @brief this global variable contains the pipes for cleanup
 * @details use the defines READ, WRITE, PARENT and CHILD to access
 */
int pipes[2][2];


/* PROTOTYPES */

/**
 * mandatory exit function
 * @brief terminate program on program error closes all open streams and
 * print error message
 * @details if errno is set it is printed too
 * @param eval the exit code 
 * @param fmt format string to print
 */
void bail_out(int eval, const char *fmt, ...);

/**
 * mandatory usage function
 * @brief This function prints the usage information (SYNOPSIS) onto stderr
 * @details uses global variable: program_name
 */
void usage(void);

/**
 * @brief free all resources used by child and parent process
 */
void free_resources( void );

#endif /*ifndef dp_calculator_h*/
