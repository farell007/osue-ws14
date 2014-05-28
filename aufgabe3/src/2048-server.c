/**
 * @file 2048-server.c
 * @author David Pfahler (1126287) <e1126287@student.tuwien.ac.at>
 * @brief TODO
 * @date 26.04.2014
 */

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
static struct shared_game *game;
static struct shared_server *server;

/* === PROTOTYPES */

/**
 * @brief TODO
 */
static void init_semaphors( key_t key );

/**
 * @brief initialize shared memory
 * @details shm_id
 */
static void init_shared_memory( void );

/**
 * @brief initialize shared memory
 * @param key the key of the shared memory
 * @details shm_id
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
 * @param options Struct where parsed arguments are stored
 */
static void parse_args(int argc, char **argv, unsigned int *power_of_two);

static unsigned int new_number_field(unsigned int field[FIELD_SIZE_Y][FIELD_SIZE_X]);

static void move(unsigned int *next,unsigned int *curr);

static unsigned int move_numbers_field(
	unsigned int field[FIELD_SIZE_Y][FIELD_SIZE_X], 
	unsigned int command, 
	unsigned int power_of_two);


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
	s4 = seminit(SEM_KEY + 5*key + 4, PERMISSION, 1);
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

	server->id = 0;
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

	memset( game->field, '\0', sizeof(game->field[0][0])*FIELD_SIZE_X*FIELD_SIZE_Y); 

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

static unsigned int new_number_field(unsigned int field[FIELD_SIZE_Y][FIELD_SIZE_X])
{
    int number_zero_fields = 0;
    for(int y = 0; y < FIELD_SIZE_Y; ++y){
        for(int x = 0; x < FIELD_SIZE_X; ++x){
            if(field[y][x] == 0){
                ++number_zero_fields;
            }
        }
    }
    if(number_zero_fields == 0){
    	//Error
    	return ST_LOST;
    	DEBUG("NO FIELDS LEFT!\n");
    } else{
	    int r = rand() % number_zero_fields + 1;
	    int power = 2;
	    if((rand() % 4) < 2){
	        power = 1;
	    }
	    for(int y = 0; y < FIELD_SIZE_Y; ++y){
	        for(int x = 0; x < FIELD_SIZE_X; ++x){
	            if(field[y][x] == 0){
	                if(--r==0){
	                    field[y][x] = power;
	                }
	            }
	        }
	    }
	}

	return ST_ON;
}

static void move(unsigned int *next,unsigned int *curr)
{
	if(*next == 0){
		*next = *curr;
		*curr = 0; 
	} 
	if(*curr == *next && *next != 0){
		*curr = 0;
		*next = *next + 1; 
	}
}

static unsigned int move_numbers_field(
	unsigned int field[FIELD_SIZE_Y][FIELD_SIZE_X], 
	unsigned int command, 
	unsigned int power_of_two)
{
	unsigned int *next, *curr;
	switch(command){
		case CMD_LEFT:	
			DEBUG("MOVE LEFT!\n");
			for(int y = 0; y < FIELD_SIZE_Y; ++y){
				int x = FIELD_SIZE_X -1;
				next = &field[y][x];
				while(--x >= 0){
					curr = next;
					next = &field[y][x];
					move(next,curr);
				}
		    }
			break;		
		case CMD_RIGHT:
			DEBUG("MOVE RIGHT!\n");
			for(int y = 0; y < FIELD_SIZE_Y; ++y){
				int x = 0;
				next = &field[y][x];
				while(++x < FIELD_SIZE_X){
					curr = next;
					next = &field[y][x];
					move(next,curr);
				}
		    }
			break;	
		case CMD_UP:	
			DEBUG("MOVE UP!\n");
			for(int x = 0; x < FIELD_SIZE_X; ++x){
				int y = FIELD_SIZE_Y-1;
				next = &field[y][x];
				while(--y >= 0){
					curr = next;
					next = &field[y][x];
					move(next,curr);
				}
		    }
			break;		
		case CMD_DOWN:
			DEBUG("MOVE DOWN!\n");
			for(int x = 0; x < FIELD_SIZE_X; ++x){
				int y = 0;
				next = &field[y][x];
				while(++y < FIELD_SIZE_Y){
					curr = next;
					next = &field[y][x];
					move(next,curr);
				}
		    }
			break;
		case CMD_DELETE:
		 	return ST_DELETE;
		case CMD_DISCONNECT:	
		 	return ST_HALT;	
	}

    for(int y = 0; y < FIELD_SIZE_Y; ++y){
        for(int x = 0; x < FIELD_SIZE_X; ++x){
            if(field[y][x] == power_of_two){
            	return ST_WON;
            }
        }
    }

    return new_number_field(game->field);
}

/**
 * Program entry point
 * @brief TODO
 * @param argc The argument counter
 * @param argv The argument vector
 * @return EXIT_SUCCESS on success, EXIT_FAILURE in case of an error
 * @details global variables: program_name, TODO
*/
int main(int argc, char ** argv) {
	unsigned int power_of_two;
	(void) parse_args(argc, argv, &power_of_two);
	(void) setup_signal_handler();
	(void) init_shared_memory();
	do{
		DEBUG("PARENT IS WAITING FOR NEW CHILDS TO CONNECT\n");
		//fork program when the client unlocks the semaphore
		if (P(sem_client) < 0) {
			(void) bail_out(EXIT_FAILURE,"Semaphore P sem_client");
		}
		pid_t pid = fork();
		//from here the program is seperated into two programms

		switch (pid) {
		case -1:
			(void) bail_out(EXIT_FAILURE,"can't fork");
			break;
		case 0:
			/* start child */
			srand(time(NULL));
			if(server->id == 0){
				server->number_clients = server->number_clients + 1;
				server->id = server->number_clients;
				(void) init_shared_game(server->id);
				new_number_field(game->field);
			} else{
				(void) init_shared_game(server->id);
			}
			
			if (V(sem_client2) < 0) {
		    	(void) bail_out(EXIT_FAILURE,"Semaphore V sem_client2");
		 	}

			DEBUG("Starting Loop\n");
			do {
				if (P(s1) < 0) {
		         (void) bail_out(EXIT_FAILURE,"Semaphore P1");
		     	}
					unsigned int cmd = game->command;
					unsigned int status = move_numbers_field(game->field,cmd,power_of_two);
					DEBUG("Got:\t%d\n", cmd);
				if (V(s2) < 0) {
		    		(void) bail_out(EXIT_FAILURE,"Semaphore V1");
		 		}
				if (P(s4) < 0) {
		         (void) bail_out(EXIT_FAILURE,"Semaphore P4");
		     	}
					game->status = status;
					DEBUG("Writing to client %d: %d\n",server->id,game->status);
				if (V(s3) < 0) {
		    		(void) bail_out(EXIT_FAILURE,"Semaphore V3");
		 		}
			} while (1);
			
			(void) clean_close();
			break;
		default:
			break;
		}
	} while(1);
	(void) clean_close();
}