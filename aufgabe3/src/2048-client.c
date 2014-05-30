/**
 * @file 2048-client.c
 * @author David Pfahler (1126287) <e1126287@student.tuwien.ac.at>
 * @brief This is the class for the client of a 2048 game. It connects to a 2048-server and displays the game field and handles the commands to the server
 * @date 26.04.2014
 */

#include "shared.h"
#include <stdbool.h>
#include <limits.h>
#include <assert.h>
#include <math.h>

/* === TYPE DEFINITIONS === */

/* === GLOBALS === */

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
static struct shared_game* game;
/**
 * @brief the pointer to the shared memory of a server
 */
static struct shared_server *server;

/**
 * @brief the id of the game
 */
static uint16_t id;


/* === PROTOTYPES === */

/**
 * @brief initialize shared memory of server
 * @details shm_id_clients, server
 */
static void init_shared_memory( void );

/**
 * @brief initialize shared memory of the game and initializes the semaphores
 * @param key the key of the shared memory
 * @details shm_id_game, game, id
 */
static void init_shared_game( key_t key );

/**
 * mandatory usage function
 * @brief This function prints the usage information (SYNOPSIS) onto stderr
 * @details uses global variable: program_name
 */
static void usage(void);

/**
 * @brief Gets the semaphors that should got initialized by the server
 * @param key the id of the game
 * @details s1,s2,s3,s4
 */
static void grab_semaphors( key_t key );

/**
 * @brief Parse command line options
 * @param argc The argument counter
 * @param argv The argument vector
 * @details global variable id gets set
 */
static void parse_args(int argc, char **argv);

/**
 * @brief blocks until the user enters a command via the command line. Valid commands are listed in commands.h
 * @returns the code of the entered command
 */
static int read_next_command( void );

/**
 * @brief prints the field of the game with a message of the game status
 * @param field the field to be printed
 * @param msg the message of the game status
 */
static void print_field(struct tile field[FIELD_SIZE_Y][FIELD_SIZE_X], const char* msg);

/**
 * @brief prints a help message for the commands of the client
 */
static void print_help(void);

/**
 * @brief pauses the game and closes the client
 * @details detaches the shared memory of the game and the server
 */
static void pause_game(void);

/**
 * @brief deletes the game and closes the client
 * @details detaches the shared memory of the game and the server
 */
static void clean_client_close(void);

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
	id = key;
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

	print_field(game->field,"NEW GAME");
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

static void parse_args(int argc, char **argv)
{
	char *endptr;
	int getopt_result;
	long id_arg = 0;
	int nflag = 0;
	int iflag = 0;

	id = ID_UNSET;

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

			id = id_arg;
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

	DEBUG("Parsing Arguments finished.User ID: %d\n",id);
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
		case 'h':
			print_help();
			return '\n';
		default:
			bail_out(EXIT_FAILURE,"The character %c (%i) is not a valid command\n",c,c);
			break;
	}
	assert(0);
}

static void print_field(struct tile field[FIELD_SIZE_Y][FIELD_SIZE_X], const char* msg)
{
	const int size = FIELD_SIZE_Y*5 + 5;
	char startline[size+1];
	memset(startline,'#',size+1);
	startline[size] = '\0';

	char line[FIELD_SIZE_Y*5+1];
	memset(line,'-',FIELD_SIZE_Y*5+1);
	for(int i = 0; i < FIELD_SIZE_Y; ++i){
		line[i*5] = '+';
	}
	line[FIELD_SIZE_Y*5] = '\0';
	printf("%s\n",startline);
	printf("#%-*s#\n",size-2," Tiles");
	printf("# ID=%06d %*s#\n",id,size-13," ");
	printf("#%*s#\n",size-2," ");
	printf("# %-*s#\n",size-3,msg);
	printf("#%*s#\n",size-2," ");

	printf("# %s+ #\n# ",line);
	for(int y = 0; y < FIELD_SIZE_Y; ++y){
		for(int x = 0; x < FIELD_SIZE_X; ++x){
			if(field[y][x].val == 0){
				printf("|%4s","");
			} else{
				printf("|%4u", 1<<field[y][x].val);
			}
		}
		printf("| #\n# %s+ #\n# ",line);
	}
	printf("%*s#\n",size-3," ");
	printf("#%-*s#\n",size-2," Display help with 'h'");
	printf("%s\n",startline);
}

static void print_help(void)
{
	const int size = FIELD_SIZE_Y*5 + 5;
	char startline[size+1];
	memset(startline,'#',size+1);
	startline[size] = '\0';

	printf("%s\n",startline);
	printf("#%-*s#\n",size-2," W = Up");
	printf("#%-*s#\n",size-2," A = Left");
	printf("#%-*s#\n",size-2," S = Down");
	printf("#%-*s#\n",size-2," D = Right");
	printf("#%-*s#\n",size-2," E = Pause game");
	printf("#%-*s#\n",size-2," X = Delete game");
	printf("#%-*s#\n",size-2," H = This Help");
	printf("%s\n",startline);
	printf("Enter Command: >");
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

	MY_V(s4,"s4");

	exit(EXIT_SUCCESS);
}

static void clean_client_close(void)
{

	DEBUG("Clean Client Close %s\n",program_name);

	MY_V(s4,"s4");

	exit(EXIT_SUCCESS);
}

/**
 * Program entry point
 * @brief Program entry point
 * @param argc The argument counter
 * @param argv The argument vector
 * @return EXIT_SUCCESS on success, EXIT_FAILURE in case of an error
 * @details global variables: program_name, id, server, game, s1, s2, s3, s4, sem_client, sem_client2
*/
int main(int argc, char ** argv) {
	(void) parse_args(argc, argv);
	(void) setup_signal_handler();	
	(void) init_shared_memory();
	
	server->id = id;
	MY_V(sem_client,"sem_client");
	DEBUG("WAIT FOR SERVER TO INIT GAME\n");
	MY_P(sem_client2,"sem_client2");
 	init_shared_game(server->id);
 	printf("Enter Command: >");
	int cmd;
	while((cmd = read_next_command()) != EOF){
		if(cmd == '\n') continue;
		if (ferror(stdin)) {
			(void) bail_out(EXIT_FAILURE,"fgetc");
		}
		MY_P(s2,"s2");
			game->command = cmd;
			DEBUG("Wrote command '%d' to server\n",cmd);	
		MY_V(s1,"s1");
		MY_P(s3,"s3");
			unsigned int status = game->status;
			DEBUG("STATUS: %d\n",status);
			switch(status){
			case ST_WON:
				print_field(game->field,"GAME WON");
				(void) clean_client_close();
				break;
			case ST_LOST:
				print_field(game->field,"GAME OVER");
				(void) clean_client_close();
				break;
			case ST_ON:
				print_field(game->field,"TILES MOVED");
				break;
			case ST_DELETE:
				print_field(game->field,"GAME DELETED");
				(void) clean_client_close();
				break;
			case ST_HALT:
				print_field(game->field,"GAME PAUSED");
				(void) pause_game();
				break;
			case ST_NOSUCHGAME:
				print_field(game->field,"NO MOVE AVAILABLE");
				break;
			default:
				print_field(game->field,"UNKNOWN GAME STATUS");
				(void) clean_client_close();
				break;
		}
		printf("Enter Command: >");
		MY_V(s4,"s4");
	}

	(void) clean_client_close(); 
}