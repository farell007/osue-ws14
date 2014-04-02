/**
 * @file client.c  
 * @author David Pfahler (1126287) <e1126287@student.tuwien.ac.at> 
 * @date Apr 01 2014
 * gcc -std=c99 -Wall -g -pedantic -DENDEBUG -D_BSD_SOURCE -D_XOPEN_SOURCE=500 -o server server.c
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <assert.h>
#include <stdbool.h>

/* === Constants === */

#define MAX_TRIES (35)
#define SLOTS (5)
#define COLORS (8)

#define READ_BYTES (2)
#define WRITE_BYTES (1)
#define BUFFER_BYTES (2)
#define SHIFT_WIDTH (3)
#define PARITY_ERR_BIT (6)
#define GAME_LOST_ERR_BIT (7)

#define EXIT_PARITY_ERROR (2)
#define EXIT_GAME_LOST (3)
#define EXIT_MULTIPLE_ERRORS (4)

 
/* === Macros === */

#ifdef ENDEBUG
#define DEBUG(...) do { fprintf(stderr, __VA_ARGS__); } while(0)
#else
#define DEBUG(...)
#endif

/* Length of an array */
#define COUNT_OF(x) (sizeof(x)/sizeof(x[0]))

/* === Global Variables === */

/* Name of the program */
static const char *progname = "client"; /* default name */

/* Usage Message */
static const char *usage = "SYNOPSIS\n\tclient <server-hostname> <server-port>\n\t\tEXAMPLE\n\tclient localhost 1280"; 

/* File descriptor for connection socket */
static int connfd = -1;

/* This variable is set to ensure cleanup is performed only once */
volatile sig_atomic_t terminating = 0;

/* === Type Definitions === */

struct opts {
    const char *server_port;
	const char *server_hostname;
};

enum color {beige = 0, darkblue, green, orange, red, black, violet, white};

/* === Prototypes === */

/**
 * @brief Parse command line options
 * @param argc The argument counter
 * @param argv The argument vector
 * @param options Struct where parsed arguments are stored
 */
static void parse_args(int argc, char **argv, struct opts *options);

/**
 * @brief Tries to create a TCP/IP connection to the server specified with the options
 * @param options contains the informations for connecting with the server (port and hostname)
 * @return the handler to the connected socket
 */
static int create_connection(struct opts *options);
/**
 * @brief terminate program on program error
 * @param eval exit code
 * @param fmt format string
 */
static void bail_out(int eval, const char *fmt, ...);

/**
 * @brief Signal handler
 * @param sig Signal number catched
 */
static void signal_handler(int sig);

/**
 * @brief free allocated resources
 */
static void free_resources(void);

/**
 * @brief generate the next tip
 * @return the two bytes for the server
 */
static uint16_t get_tipp(void);

/**
 * @brief sets a pin for the tipp at a position
 * @param pos the position of the pin (0-4)
 * @param c the color to set
 * @param tipp the pointer to the tipp to get changed
 */
static void set_pin(int pos, enum color c, uint16_t *tipp);
/**
 * @brief set the parity bit 
 * @param the 2 bytes that want to get sent to the server
 */
static void set_parity_bit(uint16_t *tipp);

/* === Implementations === */

static void bail_out(int eval, const char *fmt, ...)
{
    va_list ap;

    (void) fprintf(stderr, "%s: ", progname);
    if (fmt != NULL) {
        va_start(ap, fmt);
        (void) vfprintf(stderr, fmt, ap);
        va_end(ap);
    }
    if (errno != 0) {
        (void) fprintf(stderr, ": %s", strerror(errno));        
    }
    (void) fprintf(stderr, "\n");

    free_resources();
    exit(eval);
}

static void free_resources(void)
{
    sigset_t blocked_signals;
    (void) sigfillset(&blocked_signals);
    (void) sigprocmask(SIG_BLOCK, &blocked_signals, NULL);

    /* signals need to be blocked here to avoid race */
    if(terminating == 1) {
        return;
    }
    terminating = 1;

    /* clean up resources */
    DEBUG("Shutting down server\n");
    if(connfd >= 0) {
        (void) close(connfd);
    }
}

static void signal_handler(int sig)
{
    /* signals need to be blocked by sigaction */
    DEBUG("Caught Signal\n");
    free_resources();
    exit(EXIT_SUCCESS);
}

static void parse_args(int argc, char **argv, struct opts *options)
{
	char *host_arg;
	char *port_arg;
	char *endptr;
	long int port;

	if(argc > 0) {
		progname = argv[0];
	} 
	if(argc < 3){
		bail_out(EXIT_FAILURE, "Too less arguments!\n%s", usage);
	}
	host_arg = argv[1];
	port_arg = argv[2];
	
	errno = 0;
	port = strtol(port_arg, &endptr, 10);

	if((errno == ERANGE && (port == LONG_MAX || port == LONG_MIN)) 
		|| (errno != 0 && port == 0)) {
		bail_out(EXIT_FAILURE, "parsing of port failed! (strtol)");
	}
	if(endptr == port_arg){ bail_out(EXIT_FAILURE, "parsing of port failed! No digits were found."); }

	/* strtol() parsed a number! */

	if(*endptr != '\0'){
		bail_out(EXIT_FAILURE, "Further characters after <server-port>: %s", endptr);
	}

	/* check for valid port range */
	if(port < 1 || port	> 65535){
		bail_out(EXIT_FAILURE, "Use a valid TCP/IP port range (1-65535)");
	}
	
	options->server_hostname = host_arg; 	
    options->server_port = port_arg;	

	DEBUG("Parsing Arguments finished! Port: %s, Host: %s\n",options->server_port,options->server_hostname);
}

static int create_connection(struct opts *options)
{
	int sockfd = -1; 
	struct addrinfo* ai,* ai_sel = NULL;
	struct addrinfo hints;
	int err;

	hints.ai_flags = 0;
	hints.ai_family = AF_INET; /* IPv4 only, IPv6: AF_INET6  */
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_addrlen = 0;
	hints.ai_addr = NULL;
	hints.ai_canonname = NULL;
	hints.ai_next = NULL;

	if((err = getaddrinfo(options->server_hostname,options->server_port, &hints, &ai)) < 0)
	{
		(void) bail_out(EXIT_FAILURE, "ERROR: %s", gai_strerror(err));
	}

	if(ai == NULL){
		(void) bail_out(EXIT_FAILURE,"Could not resolve host %s.\n", options->server_hostname);
	}
	
	ai_sel = ai;
	if((sockfd = socket(ai_sel->ai_family, ai_sel->ai_socktype, ai->ai_protocol)) < 0)
	{
		(void) bail_out(EXIT_FAILURE, "Socket creation failed\n");
	}
	
	DEBUG("socket created!\n");

	if(connect(sockfd, ai_sel->ai_addr, ai_sel->ai_addrlen) <0)
	{
		(void) close(sockfd);
		freeaddrinfo(ai);
		(void) bail_out(EXIT_FAILURE, "Connection failed.");
		
	}
	DEBUG("Connected to the Server!");
	freeaddrinfo(ai);

	return sockfd;
}


static uint16_t get_tipp(void){
	uint16_t tipp = 0x0000;
	
	(void) set_pin(0,red,&tipp);
	(void) set_pin(1,red,&tipp);
	(void) set_pin(2,red,&tipp);
	(void) set_pin(3,red,&tipp);
	(void) set_pin(4,red,&tipp);
	
	(void) set_parity_bit(&tipp);

	return tipp; 
}

static void set_pin(int pos, enum color c, uint16_t *tipp){	
	*tipp |= c << (pos * SHIFT_WIDTH);
}

static void set_parity_bit(uint16_t *tipp){
	uint8_t parity = 0;
	uint16_t buffer = *tipp;
	for ( int j = 0; j < SLOTS; ++j){
		int tmp = buffer & 0x7;
		parity ^= tmp ^ (tmp >> 1) ^ (tmp >> 2);
		buffer >>= SHIFT_WIDTH;
	}
}

/**
 * Program entry point.
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */
int main(int argc, char **argv) {
	struct opts options;
	int running = 1;
	int round = 0;
	uint16_t tipp;
	uint8_t read_buffer;

	parse_args(argc,argv,&options);
	connfd = create_connection(&options);

	/* GAME LOOP */
	do{
		round++;
		tipp = get_tipp();
		(void) send(connfd,&tipp,2,0);
		DEBUG("Sent 0x%x\n",tipp);

		(void) recv(connfd, &read_buffer, 1, 0);
		DEBUG("Got byte 0x%x\n",read_buffer);
		
		switch(read_buffer >> PARITY_ERR_BIT) {
			case 1:
				(void) bail_out(EXIT_PARITY_ERROR,"Parity error");
				break;
			case 2: 
				(void) bail_out(EXIT_GAME_LOST, "Game lost");
				break;
			case 3: 
				(void) bail_out(EXIT_MULTIPLE_ERRORS, "Parity error and game lost");
				break;
			default:
				break;
		}

		if((read_buffer & 7) == 5) //all answers (first 3 bits) are true
		{
			printf("Rounds: %d\n",round);
			return 0;
		}
		
	}
	while(running == 1);

	assert(0);
	return EXIT_FAILURE;
}
