/**
 * @file 2048-client.c
 * @author David Pfahler (1126287) <e1126287@student.tuwien.ac.at>
 * @brief TODO
 * @date 26.04.2014
 */

#include "shared.h"
#include <stdbool.h>
#include <limits.h>
#include <assert.h>
#include <math.h>

/* === TYPE DEFINITIONS === */

struct opts {
    bool new_game;
    uint16_t id;
};

/* === GLOBALS === */

extern int shm_id_game;
extern int shm_id_clients;
extern int sem_client;
extern int sem_client2;
extern int s1;
extern int s2;
extern int s3;
extern int s4;
static struct shared_game* game;
static struct shared_server *server;


/* === PROTOTYPES === */

/**
 * @brief initialize shared memory
 * @param key the key of the shared memory
 * @details shm_id_clients
 */
static void init_shared_memory( void );

/**
 * @brief initialize shared memory
 * @param key the key of the shared memory
 * @details shm_id_game
 */
static void init_shared_game( key_t key );

/**
 * mandatory usage function
 * @brief This function prints the usage information (SYNOPSIS) onto stderr
 * @details uses global variable: program_name
 */
static void usage(void);

/**
 * @brief TODO
 */
static void grab_semaphors( key_t key );

/**
 * @brief Parse command line options
 * @param argc The argument counter
 * @param argv The argument vector
 * @param options Struct where parsed arguments are stored
 */
static void parse_args(int argc, char **argv, struct opts *options);

/**
 * @brief blocks until the user enters a command via the command line. Valid commands are listed in commands.h
 * @returns the code of the entered command
 */
static int read_next_command( void );

static void print_field(unsigned int field[FIELD_SIZE_Y][FIELD_SIZE_X]);

static void pause_game(void);

/* === IMPLEMENTATIONS === */


static void usage(void)
{
	bail_out(EXIT_FAILURE, "USAGE: 2048-client [-n | -i <id>]\n"
	"\t-n:\tStart a new game\n"
	"\t-i:\tConnect to existing game with the given id\n");
}

static void grab_semaphors( key_t key )
{
    s1 = semgrab(SEM_KEY + 5*key + 1);
    if (s1 < 0) {
        (void) bail_out(EXIT_FAILURE,"semgrab (1) failed");
    }
    s2 = semgrab(SEM_KEY + 5*key + 2);
    if (s2 < 0) {
        (void) bail_out(EXIT_FAILURE,"semgrab (2) failed");
    }
    s3 = semgrab(SEM_KEY + 5*key + 3);
    if (s3 < 0) {
        (void) bail_out(EXIT_FAILURE,"semgrab (3) failed");
    }
    s4 = semgrab(SEM_KEY + 5*key + 4);
    if (s4 < 0) {
        (void) bail_out(EXIT_FAILURE,"semgrab (4) failed");
    }
}

static void init_shared_game( key_t key )
{
	DEBUG("Connect to shared game with key %d\n",key);
    shm_id_game = shmget(SHM_KEY+key, sizeof(struct shared_game), PERMISSION);
    if (shm_id_game < 0) {
        (void) bail_out(EXIT_FAILURE,"Could not access the shared memory! Is there a online server?");
    }
    game = shmat(shm_id_game, NULL, 0);
    if (game == (struct shared_game *) -1) {
        (void) bail_out(EXIT_FAILURE,"shmat failed (game)");
    }

    (void) grab_semaphors(key);

    print_field(game->field);
}


static void init_shared_memory( void )
{
    shm_id_clients = shmget(SHM_KEY, sizeof(struct shared_server), PERMISSION);
    if (shm_id_clients < 0) {
        (void) bail_out(EXIT_FAILURE,"Could not access the shared memory! Is there a online server?");
    }
    server = shmat(shm_id_clients, NULL, 0);
    if (server == (struct shared_server*) -1) {
        (void) bail_out(EXIT_FAILURE,"shmat failed (server)");
    }

    sem_client =semgrab(SEM_KEY);
	if (sem_client < 0) {
		(void) bail_out(EXIT_FAILURE,"semgrab (sem_client) failed");
	}

	sem_client2 =semgrab(SEM_KEY+1);
	if (sem_client2 < 0) {
		(void) bail_out(EXIT_FAILURE,"semgrab (sem_client2) failed");
	}
}

static void parse_args(int argc, char **argv, struct opts *options)
{
    char *endptr;
    int getopt_result;
    long id_arg = 0;
    int nflag = 0;
    int iflag = 0;

    options->new_game = true;
    options->id = ID_UNSET;

    if(argc > 0) {
    	program_name = argv[0];
    }
    
    while ((getopt_result = getopt(argc, argv, "ni:")) != -1) {
        switch (getopt_result) {
        case 'n':
        	if(nflag != 0 || iflag != 0){
        		(void) usage();
        	}
        	nflag = 1;
        	break;
        case 'i':
       		if(nflag != 0 || iflag != 0){
        		(void) usage();
        	}
        	iflag = 1;
        	id_arg = strtol(optarg, &endptr, 10);
    		if((errno == ERANGE && (id_arg == LONG_MAX || id_arg == LONG_MIN)) 
				|| (errno != 0 && id_arg == 0)) {
				bail_out(EXIT_FAILURE, "parsing of id failed! (strtol)");
			}
			if(endptr == optarg){ bail_out(EXIT_FAILURE, "parsing of id failed! (strtol) No digits were found."); }
			/* strtol() parsed a number! */

			if(*endptr != '\0'){
				bail_out(EXIT_FAILURE, "Further characters after <id>: %s", endptr);
			}

			/* check for valid id range */
			if(id_arg < ID_MIN || id_arg > ID_MAX){
				bail_out(EXIT_FAILURE, "Use a valid id range (%d-%d)",ID_MIN,ID_MAX);
			}

			options->id = id_arg;
			options->new_game = false;
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

    DEBUG("Parsing Arguments finished.\nNewgame: %d, User ID: %d\n",options->new_game,options->id);
}

static int read_next_command( void )
{
    char c = getchar();
    switch(c){
        case EOF:
            return EOF;
        case '\n':
            return '\n';
        case 'w': 
            return CMD_UP;
        case 'a': 
            return CMD_LEFT;
        case 's': 
            return CMD_DOWN;
        case 'd': 
            return CMD_RIGHT;
        case 'e': 
            return CMD_DISCONNECT;
        case 'x': 
            return CMD_DELETE;
        default:
            bail_out(EXIT_FAILURE,"The character %c (%i) is not a valid command\n",c,c);
            break;
    }
    assert(0);
}

static void print_field(unsigned int field[FIELD_SIZE_Y][FIELD_SIZE_X])
{
    char *line = "+----+----+----+----+";
    printf("%s\n",line);
    for(int y = 0; y < FIELD_SIZE_Y; ++y){
        for(int x = 0; x < FIELD_SIZE_X; ++x){
            if(field[y][x] == 0){
                printf("|%4s","");
            } else{
                printf("|%4u", 1<<field[y][x]);
            }
        }
        printf("|\n%s\n",line);
    }
}

static void pause_game(void)
{

    DEBUG("%s: Clean Pause Client\n",program_name);

    // Remove shm game
    if (shmdt(server) < 0) {
        (void) bail_out(EXIT_FAILURE,"Error detaching shared memory of server (shmdt)");
    }

    // Remove shm game
    if (shmdt(game) < 0) {
        (void) bail_out(EXIT_FAILURE,"Error detaching shared memory of game (shmdt)");
    }

    exit(EXIT_SUCCESS);
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
	struct opts options;

    (void) parse_args(argc, argv, &options);
    (void) setup_signal_handler();    
    (void) init_shared_memory();
    
    if(options.new_game == true){
        server->id = 0;
    } else{
    	server->id = options.id;
    }
	if (V(sem_client) < 0) {
		(void) bail_out(EXIT_FAILURE,"Semaphore V sem_client");
	}
	DEBUG("WAIT FOR SERVER TO INIT GAME\n");
    if (P(sem_client2) < 0) {
    	(void) bail_out(EXIT_FAILURE,"Semaphore P sem_client2");
 	}
 	init_shared_game(server->id);

    int cmd;
    while((cmd = read_next_command()) != EOF){
    	if(cmd == '\n') continue;
        if (ferror(stdin)) {
            (void) bail_out(EXIT_FAILURE,"fgetc");
        }
        if (P(s2) < 0) {
        	(void) bail_out(EXIT_FAILURE,"Semaphore P2");
     	}
            game->command = cmd;
            DEBUG("Wrote command '%d' to server\n",cmd);
        if (V(s1) < 0) {
    		(void) bail_out(EXIT_FAILURE,"Semaphore V1");
 		}	
        if (P(s3) < 0) {
        	(void) bail_out(EXIT_FAILURE,"Semaphore P3");
     	}
            unsigned int status = game->status;
            DEBUG("STATUS: %d\n",status);
        if (V(s4) < 0) {
    		(void) bail_out(EXIT_FAILURE,"Semaphore V4");
 		}
        switch(status){
            case ST_WON:
                printf("%s\tGAME WON!\n",program_name);
                (void) clean_close();
                break;
            case ST_LOST:
                printf("%s\tGAME OVER!\n",program_name);
                print_field(game->field);
                (void) clean_close();
                break;
            case ST_ON:
                print_field(game->field);
                break;
            case ST_DELETE:
                printf("%s\tGAME DELETED!\n",program_name);
                (void) clean_close();
                break;
            case ST_HALT:
                printf("%s\tGAME PAUSED!\n",program_name);
                (void) pause_game();
                break;
            default:
                printf("%s\tUNKNOWN GAME STATUS!\n",program_name);

                (void) clean_close();
                break;
    	}
    }

    (void) clean_close();
}