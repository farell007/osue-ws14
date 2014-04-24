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
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include "parent.h"
#include "child.h"

/* CONSTANTS */

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

/* PROTOTYPES */

/**
 * mandatory exit function
 * @brief terminate program on program error closes all open streams and print error message
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

#endif /*ifndef dp_calculator_h*/
