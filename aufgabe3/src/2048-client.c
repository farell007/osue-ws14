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
#include <time.h>

/* === TYPE DEFINITIONS === */

struct opts {
    bool new_game;
    uint16_t id;
};

extern int shm_id;
extern int s1;
extern int s2;
extern int s3;
extern int s4;
static struct shared_struct* shared_mem;

/* === STATIC FUNCTIONS === */

/**
 * mandatory usage function
 * @brief This function prints the usage information (SYNOPSIS) onto stderr
 * @details uses global variable: program_name
 */
static void usage(void){
	bail_out(EXIT_FAILURE, "USAGE: 2048-client [-n | -i <id>]\n"
	"\t-n:\tStart a new game\n"
	"\t-i:\tConnect to existing game with the given id\n");
}

/**
 * @brief TODO
 */
static void grab_semaphors( void )
{
    s1 = semgrab(SEM_KEY);
    if (s1 < 0) {
        (void) bail_out(EXIT_FAILURE,"semgrab (1) failed");
    }
    s2 = semgrab(SEM_KEY+1);
    if (s2 < 0) {
        (void) bail_out(EXIT_FAILURE,"semgrab (2) failed");
    }
    s3 = semgrab(SEM_KEY+2);
    if (s3 < 0) {
        (void) bail_out(EXIT_FAILURE,"semgrab (3) failed");
    }
    s4 = semgrab(SEM_KEY+3);
    if (s4 < 0) {
        (void) bail_out(EXIT_FAILURE,"semgrab (4) failed");
    }
}

/**
 * @brief initialize shared memory
 * @details shm_id
 */
static void init_shared_memory( void )
{
    shm_id = shmget(SHM_KEY, sizeof(struct shared_struct), PERMISSION);
    if (shm_id < 0) {
        (void) bail_out(EXIT_FAILURE,"Could not access the shared memory! Is there a online server?");
    }
    shared_mem = shmat(shm_id, NULL, 0);
    if (shared_mem == (struct shared_struct *) -1) {
        (void) bail_out(EXIT_FAILURE,"shmat failed");
    }

    (void) grab_semaphors();
}

/**
 * @brief Parse command line options
 * @param argc The argument counter
 * @param argv The argument vector
 * @param options Struct where parsed arguments are stored
 */
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

/**
 * @brief blocks until the user enters a command via the command line. Valid commands are listed in commands.h
 * @returns the code of the entered command
 */
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
            printf("|%4u", field[y][x]);
        }
        printf("|\n%s\n",line);
    }
}

/* === MAIN FUNCTION === */

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
    srand(time(NULL));

    (void) parse_args(argc, argv, &options);
    (void) setup_signal_handler();    

    if(options.new_game == true){
        (void) init_shared_memory();
    } else{

    }

    int cmd;
    while((cmd = read_next_command()) != EOF){
        if(cmd != '\n'){ 
        //to disable the return character it is pretty complicated and not platform independent
        if (ferror(stdin)) {
            (void) bail_out(EXIT_FAILURE,"fgetc");
        }
        ERROR_P(s2);
            shared_mem->command = cmd;
            DEBUG("Wrote command '%d' to server\n",cmd);
        ERROR_V(s1);
        ERROR_P(s3);
            unsigned int status = shared_mem->status;
            DEBUG("STATUS: %d\n",status);
            switch(status){
                case ST_WON:
                    printf("GAME WON!\n");
                    (void) clean_close();
                    break;
                case ST_LOST:
                    printf("GAME OVER!\n");
                    (void) clean_close();
                    break;
                case ST_ON:
                    print_field(shared_mem->field);
                    break;
                case ST_DELETE:
                    printf("GAME DELETED!\n");
                    (void) clean_close();
                    break;
                case ST_HALT:
                    printf("GAME PAUSED!\n");
                    (void) clean_close();
                    break;
                default:
                    printf("UNKNOWN GAME STATUS!\n");
                    (void) clean_close();
                    break;
            }
        ERROR_V(s4);
        }
    }

    (void) clean_close();
}