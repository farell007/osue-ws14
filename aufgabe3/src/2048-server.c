/**
 * @file 2048-server.c
 * @author David Pfahler (1126287) <e1126287@student.tuwien.ac.at>
 * @brief The server of a 2048 game. It gets commands from clients handles them, updates the game field and passes the new field and game status to the client
 * @date 26.04.2014
 */

#include "gamelogic.h"
#include "shared.h"
#include <limits.h>
#include <assert.h>
#include <time.h>

extern int shm_id_game;
extern int shm_id_clients;
extern int sem_client;
extern int sem_client2;
extern int s1;
extern int s2;
extern int s3;
extern int s4;

/**
 * @brief the pointer to the shared memory of a game
 */
static struct shared_game *game;

/**
 * @brief the pointer to the shared memory of a server
 */
static struct shared_server *server;

/* === PROTOTYPES */

/**
 * @brief Initializes the semaphores of a new game
 * @param key the id of the new game
 * @details s1,s2,s3,s4
 */
static void init_semaphors( key_t key );

/**
 * @brief initialize shared memory of the server and initializes the semaphores of the server
 * @details shm_id_clients, server, sem_client, sem_client2
 */
static void init_shared_memory( void );

/**
 * @brief initialize shared memory of new game, starts the game and initializes the semaphoes of it
 * @param key the key of the shared memory
 * @details shm_id_game, game
 */
static void init_shared_game( key_t key );

/**
 * mandatory usage function
 * @brief This function prints the usage information (SYNOPSIS) onto stderr
 * @details uses global variable: program_name
 */
static void usage(void);

/**
 * @brief Parse command line options
 * @param argc The argument counter
 * @param argv The argument vector
 * @param power_of_two the power of two that gets set by the parsed argument vector or set to default value
 */
static void parse_args(int argc, char **argv, unsigned int *power_of_two);

/**
 * @brief free allocated resources of one game but not the parent server and prints error messages if something fails
 * @details global variables: shm_id_game, s1, s2, s3, s4
 */
static void close_game( void );


/* === IMPLEMENTATIONS === */


static void init_semaphors( key_t key )
{
	s1 = seminit(SEM_KEY + 5*key + 1, PERMISSION, 0);
	if (s1 < 0) {
		(void) bail_out(EXIT_FAILURE,"seminit (1) failed");
	}
	s2 = seminit(SEM_KEY + 5*key + 2, PERMISSION, 1);
	if (s2 < 0) {
		(void) bail_out(EXIT_FAILURE,"seminit (2) failed");
	}
	s3 = seminit(SEM_KEY + 5*key + 3, PERMISSION, 0);
	if (s3 < 0) {
		(void) bail_out(EXIT_FAILURE,"seminit (2) failed");
	}
	s4 = seminit(SEM_KEY + 5*key + 4, PERMISSION, 0);
	if (s4 < 0) {
		(void) bail_out(EXIT_FAILURE,"seminit (2) failed");
	}
}

static void init_shared_memory( void )
{
	shm_id_clients = shmget(SHM_KEY, sizeof(struct shared_server), IPC_CREAT | PERMISSION);
	if (shm_id_clients < 0) {
		(void) bail_out(EXIT_FAILURE,"shmget failed clients");
	}
	server = shmat(shm_id_clients, NULL, 0);
	if (server == (struct shared_server*) -1) {
		(void) bail_out(EXIT_FAILURE,"shmat failed clients");
	}

	server->id = ID_UNSET;
	server->number_clients = 0;

	sem_client = seminit(SEM_KEY, PERMISSION, 0);
	if (sem_client < 0) {
		(void) bail_out(EXIT_FAILURE,"seminit (sem_client) failed");
	}

	sem_client2 = seminit(SEM_KEY+1, PERMISSION, 0);
	if (sem_client2 < 0) {
		(void) bail_out(EXIT_FAILURE,"seminit (sem_client2) failed");
	}
}

static void init_shared_game( key_t key )
{
	DEBUG("Create shared game with key %d\n",key);
	shm_id_game = shmget(SHM_KEY+key, sizeof(struct shared_game), IPC_CREAT | PERMISSION);
	if (shm_id_game < 0) {
		(void) bail_out(EXIT_FAILURE,"shmget failed game");
	}
	game = shmat(shm_id_game, NULL, 0);
	if (game == (struct shared_game*) -1) {
		(void) bail_out(EXIT_FAILURE,"shmat failed game");
	}

	new_game(game->field);

	game->status = ST_ON;
	game->command = CMD_UNSET;

	(void) init_semaphors( key );	
}



static void usage(void)
{
	bail_out(EXIT_FAILURE, "USAGE: 2048-server [-p power_of_two]\n"
	"\t-p:\tPlay until 2^power_of_two is reached (default: 11)\n");
}

static void parse_args(int argc, char **argv, unsigned int *power_of_two)
{
	char *endptr;
	int getopt_result;
	long p_arg = 0;
	int p_flag = 0;

	*power_of_two = POWER_OF_TWO_DEFAULT;

	if(argc > 0) {
		program_name = argv[0];
	}

	
	while ((getopt_result = getopt(argc, argv, "p:")) != -1) {
		switch (getopt_result) {
		case 'p':
	   		if(p_flag != 0){
				(void) usage();
			}
			p_flag = 1;
			p_arg = strtol(optarg, &endptr, 10);
			if((errno == ERANGE && (p_arg == LONG_MAX || p_arg == LONG_MIN)) 
				|| (errno != 0 && p_arg == 0)) {
				bail_out(EXIT_FAILURE, "parsing of power_of_two failed! (strtol)");
			}
			if(endptr == optarg){ bail_out(EXIT_FAILURE, "parsing of power_of_two failed! (strtol) No digits were found."); }
			/* strtol() parsed a number! */

			if(*endptr != '\0'){
				bail_out(EXIT_FAILURE, "Further characters after <power_of_two>: %s", endptr);
			}

			/* check for valid id range */
			if(p_arg < POWER_OF_TWO_LIMIT || p_arg	> POWER_OF_TWO_MAX){
				bail_out(EXIT_FAILURE, "Use a valid power_of_two range (%d-%d)",POWER_OF_TWO_LIMIT,POWER_OF_TWO_MAX);
			}

			*power_of_two = p_arg;
			break;
		case '?':
			usage();
			break;
		default:
			assert(0);
		}
	}

	if(optind < argc){
		(void) usage();
	}

	DEBUG("Parsing Arguments finished.\nPower of Two: %d\n",*power_of_two);
}

static void close_game( void )
{
	DEBUG("Clean Close of game %s\n",program_name);

    if (semrm(s1) < 0) {
        (void) bail_out(EXIT_FAILURE,"Error removing the semaphore 1 (semrm)");
    }
    if (semrm(s2) < 0) {
        (void) bail_out(EXIT_FAILURE,"Error removing the semaphore 2 (semrm)");
    }
    if (semrm(s3) < 0) {
        (void) bail_out(EXIT_FAILURE,"Error removing the semaphore 3 (semrm)");
    }
    if (semrm(s4) < 0) {
        (void) bail_out(EXIT_FAILURE,"Error removing the semaphore 4 (semrm)");
    }

    // Remove shm game
    if (shmctl(shm_id_game, IPC_RMID, NULL) < 0) {
        (void) bail_out(EXIT_FAILURE,"Error terminating shared memory of game (shmctl)");
    }

    exit(EXIT_SUCCESS);

}

/**
 * Program entry point
 * @brief Program entry point
 * @param argc The argument counter
 * @param argv The argument vector
 * @return EXIT_SUCCESS on success, EXIT_FAILURE in case of an error
 * @details global variables: program_name, server, game, s1, s2, s3, s4, sem_client, sem_client2
*/
int main(int argc, char ** argv) {
	unsigned int power_of_two;
	(void) parse_args(argc, argv, &power_of_two);
	(void) setup_signal_handler();
	(void) init_shared_memory();
	do{
		DEBUG("PARENT IS WAITING FOR CHILDS TO CONNECT\n");
		//fork program when the client unlocks the semaphore
		MY_P(sem_client,"sem_client");
		if(server->id == ID_UNSET){
			pid_t pid = fork();
			//from here the program is seperated into two programms

			switch (pid) {
			case -1:
				(void) bail_out(EXIT_FAILURE,"can't fork");
				break;
			case 0:
				/* start child */
				srand(time(NULL));
				server->number_clients = server->number_clients + 1;
				server->id = server->number_clients;
				(void) init_shared_game(server->id);
				
				MY_V(sem_client2,"sem_client2");

				DEBUG("Starting Loop\n");
				do {
					DEBUG("\tREADY\n");
					MY_P(s1,"s1");
					unsigned int cmd = game->command;
					unsigned int status = move_numbers_field(game->field,cmd,power_of_two);
					DEBUG("Got:\t%d\n", cmd);
					MY_V(s2,"s2");
					game->status = status;
					MY_V(s3,"s3");
					MY_P(s4,"s4");
					if(status == ST_DELETE 
						|| status == ST_WON 
						|| status == ST_LOST){
						(void) close_game();
					}

				} while (1);
				break;
			default:
				//parent process
				break;
			}
		} else{
			DEBUG("CLIENT %d CONNECTED\n",server->id);
			MY_V(sem_client2,"sem_client2");
		}
	} while(1);
}