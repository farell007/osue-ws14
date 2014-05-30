/**
 * @file shared.h
 * @author David Pfahler (1126287) <e1126287@student.tuwien.ac.at>
 * @brief The headerfile of the shared files of a 2048 game
 * @date 26.04.2014
 */

#ifndef dp_shared_h /*prevent multiple inclusion*/ 
#define dp_shared_h

/* INCLUDES */

#include <sys/shm.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sem182.h>
#include "commands.h"

/* === CONSTANTS === */

/**
 * @def POWER_OF_TWO_DEFAULT
 * @brief The default of the power of two to win the game (2^11 = 2048)
 */
#define POWER_OF_TWO_DEFAULT	(11)

/**
 * @def POWER_OF_TWO_LIMIT
 * @brief The limit of the power of two to win the game
 */
#define POWER_OF_TWO_LIMIT 		(2)
/**
 * @def POWER_OF_TWO_MAX
 * @brief The maximum of the power of two to win the game 
 */
#define POWER_OF_TWO_MAX 		(13)

/**
 * @def ID_UNSET
 * @brief The Value if a id is unset of a game
 */
#define ID_UNSET 				(0)
/**
 * @def ID_MAX
 * @brief The maximum number of clients
 */
#define ID_MAX					(65535)
/**
 * @def ID_MIN
 * @brief The minimum of the id of a game
 */
#define ID_MIN					(1)

/**
 * @def SHM_KEY
 * @brief an offset of the keys of the shared memory
 */
#define SHM_KEY					(112233)
/**
 * @def SEM_KEY
 * @brief an offset of the keys of the semaphores
 */
#define SEM_KEY 				(0x1234)
/**
 * @def PERMISSION
 * @brief The permission of the semaphores and the shared memory 
 */
#define PERMISSION				(0600)

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

/**
 * @def MY_P(x,y)
 * @brief Decreases semaphore x with 1 and locks if it is negative. If there is an error the program closes controlled
 */
#define MY_P(x,y) do{\
 if (P(x) < 0) {\
 	(void) bail_out(EXIT_FAILURE,"Semaphore P %s",y);\
 }\
} while(0)

/**
 * @def MY_V(x,y)
 * @brief Increases semaphore x with 1. If there is an error the program closes controlled
 */
#define MY_V(x,y) do{\
 if (V(x) < 0) {\
 	(void) bail_out(EXIT_FAILURE,"Semaphore V %s",y);\
 }\
} while(0)

/* === GLOBAL VARIABLES === */

/**
 * @brief Program name for usage and error messages
 */
const char * program_name;

/**
 * @brief the shared struct of a game
 */
struct shared_game 
{
	/**
	 * @brief the game field
	 */
	struct tile field[FIELD_SIZE_Y][FIELD_SIZE_X];
	/**
	 * @brief the game status for the client
	 */
    unsigned int status;
    /**
	 * @brief the command from the client
	 */
    unsigned int command;
};

/**
 *  brief the shared struct that handles new clients
 */
struct shared_server
{
	/**
	 * @brief the id of the new client or an old one getting back to a game
	 */
	uint16_t id;
	/**
	 * @brief the number of clients
	 */
	uint16_t number_clients;
};


/* === PROTOTYPES === */

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
 * @details global variables: terminating, shm_id_game, shm_id_clients, s1, s2, s3, s4, sem_client, sem_client2
 */
void free_resources(void);

/**
 * @brief free allocated resources but prints error messages if something fails
 * @details global variables: shm_id_game, shm_id_clients, s1, s2, s3, s4, sem_client, sem_client2
 */
void clean_close(void);

/**
 * @brief initializes the signal handler
 * @details binds the function signal_handler
 */
void setup_signal_handler(void);

/**
 * @brief Signal handler
 * @param sig signal number catched
 */
void signal_handler(int sig);


#endif /*ifndef dp_shared_h*/
