/**
 * @file 2048-server.c
 * @author David Pfahler (1126287) <e1126287@student.tuwien.ac.at>
 * @brief TODO
 * @date 26.04.2014
 */

#include "shared.h"
#include <limits.h>
#include <assert.h>

extern int shm_id;
extern int s1;
extern int s2;
extern int s3;
extern int s4;
static struct shared_struct *shared_mem;

/* === IMPLEMENTATIONS === */

/**
 * @brief TODO
 */
static void init_semaphors( void )
{
	s1 = seminit(SEM_KEY+0, PERMISSION, 0);
	if (s1 < 0) {
		(void) bail_out(EXIT_FAILURE,"seminit (1) failed");
	}
	s2 = seminit(SEM_KEY+1, PERMISSION, 1);
	if (s2 < 0) {
		(void) bail_out(EXIT_FAILURE,"seminit (2) failed");
	}
	s3 = seminit(SEM_KEY+2, PERMISSION, 0);
	if (s3 < 0) {
		(void) bail_out(EXIT_FAILURE,"seminit (2) failed");
	}
	s4 = seminit(SEM_KEY+3, PERMISSION, 2);
	if (s4 < 0) {
		(void) bail_out(EXIT_FAILURE,"seminit (2) failed");
	}
}

/**
 * @brief initialize shared memory
 * @details shm_id
 */
static void init_shared_memory( void )
{
	shm_id = shmget(SHM_KEY, sizeof(struct shared_struct), IPC_CREAT | PERMISSION);
	if (shm_id < 0) {
		(void) bail_out(EXIT_FAILURE,"shmget failed");
	}
	shared_mem = shmat(shm_id, NULL, 0);
	if (shared_mem == (struct shared_struct *) -1) {
		(void) bail_out(EXIT_FAILURE,"shmat failed");
	}

	memset( shared_mem->field, '\0', sizeof(shared_mem->field[0][0])*FIELD_SIZE_X*FIELD_SIZE_Y); 

	shared_mem->status = ST_ON;
	shared_mem->command = CMD_UNSET;

	(void) init_semaphors();
}


/**
 * mandatory usage function
 * @brief This function prints the usage information (SYNOPSIS) onto stderr
 * @details uses global variable: program_name
 */
static void usage(void){
	bail_out(EXIT_FAILURE, "USAGE: 2048-server [-p power_of_two]\n"
	"\t-p:\tPlay until 2^power_of_two is reached (default: 11)\n");
}

/**
 * @brief Parse command line options
 * @param argc The argument counter
 * @param argv The argument vector
 * @param options Struct where parsed arguments are stored
 */
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

static unsigned int move_numbers_field(unsigned int field[FIELD_SIZE_Y][FIELD_SIZE_X], unsigned int command, unsigned int power_of_two)
{
    for(int y = 0; y < FIELD_SIZE_Y; ++y){
        for(int x = 0; x < FIELD_SIZE_X; ++x){
            if(field[y][x] == power_of_two){
            	return ST_WON;
            }
        }
    }

    return new_number_field(shared_mem->field);
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
	unsigned int power_of_two;
	(void) parse_args(argc, argv, &power_of_two);
	
	(void) setup_signal_handler();
	(void) init_shared_memory();

	DEBUG("Starting Loop\n");
	do {
		ERROR_P(s1);
			unsigned int cmd = shared_mem->command;
			unsigned int status;
			switch(cmd){
				 case CMD_LEFT:			
				 case CMD_RIGHT:		
				 case CMD_UP:			
				 case CMD_DOWN:			
				 	status = move_numbers_field(shared_mem->field,cmd,power_of_two);
					break;
				 case CMD_DELETE:
				 	status = ST_DELETE;	
					break;
				 case CMD_DISCONNECT:	
				 	status = ST_HALT;	
					break;
			}
			DEBUG("Got:\t%d\n", shared_mem->command);
		ERROR_V(s2);
		ERROR_P(s4);
			shared_mem->status = status;
			DEBUG("Writing to client %d\n",shared_mem->status);
		ERROR_V(s3);
	} while (1);
	
	(void) clean_close();
}