/**
 * @file shared.c
 * @author David Pfahler (1126287) <e1126287@student.tuwien.ac.at>
 * @brief TODO
 * @date 26.04.2014
 */

#ifndef dp_shared_h /*prevent multible inclusion*/ 
#define dp_shared_h

/* INCLUDES */

#include <sys/shm.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sem182.h>
#include "commands.h"

/* === CONSTANTS === */

#define POWER_OF_TWO_DEFAULT	(11)
#define POWER_OF_TWO_LIMIT 		(2)
#define POWER_OF_TWO_MAX 		(256)

#define ID_UNSET 				(0)
#define ID_MAX					(65535)
#define ID_MIN					(1)

#define SHM_KEY					(112233)
#define SEM_KEY 				(0x1234)
#define PERMISSION				(0600)

#define FIELD_SIZE_X			(4)
#define FIELD_SIZE_Y			(4)

/* === MACROS === */

/**
 * @def DEBUG(...)
 * @brief Prints formatted debug message to stderr
 */
#ifdef ENDEBUG /*Flag to set from compiler for debugging*/
#define DEBUG(...) do { fprintf(stderr, __VA_ARGS__);} while(0)
#else
#define DEBUG(...)
#endif

/**
 * @def COUNT_OF(x)
 * @brief Length of an array
 */
#define COUNT_OF(x) (sizeof(x)/sizeof(x[0]))

#define ERROR_P(x) do{ if (P(x) < 0) { (void) bail_out(EXIT_FAILURE,"Semaphore P");} } while(0)
#define ERROR_V(x) do{ if (V(x) < 0) { (void) bail_out(EXIT_FAILURE,"Semaphore V");} } while(0)

/* === GLOBAL VARIABLES === */

/**
 * @brief Program name for usage and error messages
 */
const char * program_name;

/**
 * TODO
 */
struct shared_struct 
{
	unsigned int field[FIELD_SIZE_Y][FIELD_SIZE_X];
    unsigned int status;
    unsigned int command;
};


/* === PROTOTYPES === */

/**
 * @brief Signal handler
 * @param sig Signal number catched
 */
void signal_handler(int sig);

/**
 * mandatory exit function
 * @brief terminate program on program error closes all open files and
 * print error message
 * @details if errno is set it is printed too, program_name
 * @param eval the exit code 
 * @param fmt format string to print
 */
void bail_out(int eval, const char *fmt, ...);

/**
 * @brief free allocated resources
 */
void free_resources(void);

/**
 * @brief TODO
 */
void clean_close(void);

/*
 * @brief TODO
 */
void setup_signal_handler(void);


#endif /*ifndef dp_shared_h*/
