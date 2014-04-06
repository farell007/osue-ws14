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
#include <time.h>

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

#define POSSIBILITIES (32768)
 
/* === Macros === */

#ifdef ENDEBUG
#define DEBUG(...) do { fprintf(stderr, __VA_ARGS__); } while(0)
#else
#define DEBUG(...)
#endif

/* Length of an array */
#define COUNT_OF(x) (sizeof(x)/sizeof(x[0]))

/* === Type Definitions === */

struct opts {
    const char *server_port;
	const char *server_hostname;
};

typedef struct node{
	uint16_t val;
	struct node * next;
} node_t;

enum color {beige = 0, darkblue, green, orange, red, black, violet, white};

/* === Global Variables === */

/* Name of the program */
static const char *progname = "client"; /* default name */

/* Usage Message */
static const char *usage = "SYNOPSIS\n\tclient <server-hostname> <server-port>\n\t\tEXAMPLE\n\tclient localhost 1280"; 

/* File descriptor for connection socket */
static int connfd = -1;

/* linked list of possibilities */
static node_t *possibilities = NULL;

/* array for responses */
static uint8_t responses[POSSIBILITIES];

/* number of elements in possibilities*/
static uint16_t s_possibilities = POSSIBILITIES;

/* This variable is set to ensure cleanup is performed only once */
volatile sig_atomic_t terminating = 0;

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
 * @brief compute all possibilities of solutions for mastermind
 */
static void compute_possibilities( void );

/**
 * @brief Compute answer to request
 * @param req Client's guess
 * @param resp Buffer that will be sent to the client
 * @param secret The last used secret
 * @return Number of correct matches on success; -1 in case of a parity error
 */
static int compute_answer(uint16_t req, uint8_t *resp, uint8_t *secret);

/**
 * @brief generate the next tip
 * @param resp the last response from the server. if 0 make a random guess
 * @param last_guess the last guess that was sent to the server
 * @return the two bytes for the server
 */
static uint16_t get_tipp( uint8_t resp, uint16_t last_guess);

/**
 * @brief set the parity bit 
 * @param the 2 bytes that want to get sent to the server
 * @return the tipp with the set parity bit
 */
static uint16_t set_parity_bit(uint16_t tipp);

/**
 * @brief removes all possibilities that receive a worse result than the last resp.
 * @param resp the last response from the server from the "last_guess"
 * @param last_guess the last guess that was sent to the server 
 */
static void remove_worse_answers(uint8_t resp, uint16_t last_guess);

/**
 * @brief get a random possibility from the remaining possibilities
 * @return a possible solution
 */
static uint16_t get_random_answer( void ); 
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

    (void) free_resources();
    exit(eval);
}

void free_list (node_t *head){
	node_t *tmp;

	while(head != NULL){
		tmp = head;
		head = head->next;
		(void) free(tmp);
	}
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
	(void) free_list(possibilities);
	
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

	DEBUG("Parsing Arguments finished.\nPort: %s, Host: %s\n",options->server_port,options->server_hostname);
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
	
	DEBUG("socket created.\n");

	if(connect(sockfd, ai_sel->ai_addr, ai_sel->ai_addrlen) <0)
	{
		(void) close(sockfd);
		(void) freeaddrinfo(ai);
		(void) bail_out(EXIT_FAILURE, "Connection failed.");
		
	}
	DEBUG("Connected to the Server.\n");
	(void) freeaddrinfo(ai);

	return sockfd;
}

static void compute_possibilities(){
	
	possibilities = malloc(sizeof(node_t));
	node_t * curr = possibilities;
	uint16_t x = 0;
	while(x < s_possibilities){
		curr->val = x++;
		if(x < s_possibilities){
			curr->next = malloc(sizeof(node_t));
			curr = curr->next;
			responses[x]=0;
		} else{
			curr->next = NULL;
		}
	}
}

void print_list(node_t *head){
	node_t * current = head;

	while(current != NULL){
		printf("%d\n",current->val);
		current = current->next;
	}
}

static int compute_answer(uint16_t req, uint8_t *resp, uint8_t *secret)
{
	int colors_left[COLORS];
	int guess[COLORS];
	int red, white;
	int j;
	/* extract the guess */
	for (j = 0; j < SLOTS; ++j) {
		int tmp = req & 0x7;
		guess[j] = tmp;
		req >>= SHIFT_WIDTH;
	}
	/* marking red and white */
	(void) memset(&colors_left[0], 0, sizeof(colors_left));
	red = white = 0;
	for (j = 0; j < SLOTS; ++j) {
		/* mark red */
		if (guess[j] == secret[j]) {
			red++;
		} else {
			colors_left[secret[j]]++;
		}
	}
	for (j = 0; j < SLOTS; ++j) {
	/* not marked red */
		if (guess[j] != secret[j]) {
			if (colors_left[guess[j]] > 0) {
				white++;
				colors_left[guess[j]]--;
			}
		}
	}
	/* build response buffer */
	resp[0] = red;
	resp[0] |= (white << SHIFT_WIDTH);
	return red;
}

uint16_t minimax_answer(){
	uint16_t best_worst_case = s_possibilities + 100;
	uint16_t tipp = 0;
	node_t *current = possibilities;
	while(current != NULL){
		uint8_t remaining[SLOTS];
		node_t *buff = possibilities;
		uint8_t secret[SLOTS];
		for(int j = 0; j < SLOTS; ++j){
			secret[j] = (current->val >> (SHIFT_WIDTH * j)) & 7;	
		}

		while(buff != NULL){
			uint8_t resp = 0;
			(void) compute_answer(buff->val,&resp,secret);
			int score = (((resp & 7) << SHIFT_WIDTH) + ((resp>>SHIFT_WIDTH)&7));
			remaining[score]++;
			buff = buff->next;
		}
		int worst_case = 0;
		for(int i = 0; i < COUNT_OF(remaining); ++i){
			if(remaining[i]>worst_case) {
				worst_case = remaining[i];
			}
		}
		if(worst_case < best_worst_case){
			best_worst_case = worst_case;
			tipp = current->val;
		}
		current = current->next;	
	}	
	return tipp;
}

	static uint16_t get_tipp( uint8_t resp, uint16_t last_guess){
		uint16_t tipp = 0x00;
		if(s_possibilities != POSSIBILITIES){
			(void) remove_worse_answers(resp, last_guess);
		}
		//tipp = minimax_answer();
		tipp = get_random_answer();
		return set_parity_bit(tipp);
	}

	/**
	 * @brief sets a pin for the tipp at a position
	 * @param pos the position of the pin (0-4)
	 * @param c the color to set
	 * @param tipp the pointer to the tipp to get changed
	 */
	void set_pin(int pos, enum color c, uint16_t *tipp){	
		*tipp &= ~(7 << (pos * SHIFT_WIDTH));
		*tipp |= c << (pos * SHIFT_WIDTH);
	}

	static uint16_t set_parity_bit(uint16_t tipp){
		uint8_t parity = 0;
		uint16_t buffer = tipp;
		int j;
		for (j = 0; j < SLOTS; ++j){
			int tmp = buffer & 0x7;
			parity ^= tmp ^ (tmp >> 1) ^ (tmp >> 2);
			buffer >>= SHIFT_WIDTH;
		}
		parity &= 0x1;
		tipp |= parity << 15;  
		return tipp;
	}

	static void remove_worse_answers(uint8_t last_resp,uint16_t last_guess){
		//uint8_t last_correct = (((last_resp & 7) << SHIFT_WIDTH)|((last_resp>>SHIFT_WIDTH)&7));
		node_t *current = possibilities;
		node_t *last = NULL;
		last_resp &= 63; //delete the server status bits
		//delete all possible values that have a smaller value than the last guess
		while(current != NULL){
			uint8_t resp;
			uint8_t secret[SLOTS];
			for(int j = 0; j < SLOTS; ++j){
				secret[j] = (current->val >> (SHIFT_WIDTH * j)) & 7;	
			}
			(void) compute_answer(last_guess,&resp,secret);
			//uint8_t correct_guesses = (((resp & 7) << SHIFT_WIDTH)|((resp>>SHIFT_WIDTH)&7));
			if(resp != last_resp){
				if(last != NULL){
					last->next = current->next;
					(void) free(current);
					current = last->next;
				}else{ //the head has to get deleted
					possibilities = current->next;
					(void) free(current);
					current = possibilities;
				}
				s_possibilities--;
			} else{
				last = current;
				current = current->next;
			}
		}	
		DEBUG("remaining possibilities: %d\n",s_possibilities);
	}

static uint16_t get_random_answer(){
	
	uint16_t r = rand() % s_possibilities;
	DEBUG("r = %d\n",r);
	/* //if you want to make a good first guess
	if(s_possibilities == POSSIBILITIES){
		r = 18568; //bbgor
	}else{
		r = rand() % s_possibilities;
	}
	*/
	uint16_t ret = 0;
	if(s_possibilities == 1){
		ret = possibilities->val;
		(void) free(possibilities);
		possibilities = NULL;	
	}else if(r == 0){
		ret = possibilities->val;
		node_t *tmp = possibilities;
		possibilities = possibilities->next;
		(void) free(tmp);
	}else {
		node_t *current = possibilities;
		node_t *last = NULL;
		while(r-- > 0){
			last = current;
			current = current->next;
		}
		ret = current->val;
		last->next = current->next;
		(void) free(current);
	}
	s_possibilities--;
	return ret;
}

/**
 * Program entry point.
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */
int main(int argc, char **argv) {
	struct opts options;
	sigset_t blocked_signals;
	uint16_t tipp = 0;
	uint8_t read_buffer = 0;
	bool running = true;
	int round = 0;

	parse_args(argc,argv,&options);

	/* setup signal handlers */

	if(sigfillset(&blocked_signals) < 0){
		bail_out(EXIT_FAILURE, "sigfillset");
	} else{
		const int signals[] = {SIGINT, SIGQUIT, SIGTERM };
		struct sigaction s;
		s.sa_handler = signal_handler;
		(void) memcpy(&s.sa_mask, &blocked_signals, sizeof(s.sa_mask));
		s.sa_flags = SA_RESTART;
		for(int i = 0; i < COUNT_OF(signals); i++){
			if (sigaction(signals[i], &s, NULL) < 0){
				bail_out(EXIT_FAILURE, "sigaction");
			}
		}
	}

	compute_possibilities();
	connfd = create_connection(&options);

	(void) srand(time(NULL));	
	/* GAME LOOP */
	do{
		round++;
		tipp = get_tipp(read_buffer, tipp);
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
			printf("%d\n",round);
			(void) free_resources();
			return 0;
		}
		
	}
	while(running == 1);

	assert(0);
	return EXIT_FAILURE;
}
