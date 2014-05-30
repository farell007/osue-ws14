/**
 * @file shared.c
 * @author David Pfahler (1126287) <e1126287@student.tuwien.ac.at>
 * @brief The shared files between the client and the server of a 2048 game
 * @date 26.04.2014
 */

#include "shared.h"

/**
 * @brief This variable is set to ensure cleanup is performed only once
 */
volatile sig_atomic_t terminating = 0;

/**
 * @brief id for the shared memory of one game
 */
int shm_id_game = -1;

/**
 * @brief id for the shared memory of the server
 */
int shm_id_clients = -1;

/**
 * @brief semaphore for the client connection to the parent server
 */
int sem_client = -1;

/**
 * @brief semaphore for the client connection to the parent server
 */
int sem_client2 = -1;

/**
 * @brief semaphore for the client connection to the game server that is responsible for one client
 */
int s1 = -1;

/**
 * @brief semaphore for the client connection to the game server that is responsible for one client
 */
int s2 = -1;

/**
 * @brief semaphore for the client connection to the game server that is responsible for one client
 */
int s3 = -1;

/**
 * @brief semaphore for the client connection to the game server that is responsible for one client
 */
int s4 = -1;

/* === STATIC FUNCTIONS === */


/* === IMPLEMENTATIONS === */

void signal_handler(int sig) 
{
     /* signals need to be blocked by sigaction */
    DEBUG("Caught Signal: %d\n",sig);
    free_resources();
    if (sig >= 0) {
        exit(EXIT_FAILURE);
    }
}

void bail_out(int eval, const char *fmt, ...)
{
    va_list ap;
    pid_t pid;
    int status;

    (void) fprintf(stderr, "%s: ", program_name);
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
   
    DEBUG("Wait for the childs to close\n");

    pid = wait(&status);
    
    if(WEXITSTATUS(status) != EXIT_SUCCESS){
        (void) fprintf(stderr, "%s: ", program_name);
        (void) fprintf(stderr,"child with pid %d returned exit code %d.\n", pid, WEXITSTATUS(status));
        exit(EXIT_FAILURE);
    }
    

    DEBUG("SERVER CLOSED\n");
    exit(eval);
}

void free_resources (void)
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
    DEBUG("Shutting down %s\n",program_name);
    
    (void) semrm(sem_client);
    (void) semrm(sem_client2);
    (void) semrm(s1);
    (void) semrm(s2);
    (void) semrm(s3);
    (void) semrm(s4);

    (void) shmctl(shm_id_game, IPC_RMID, NULL);
    (void) shmctl(shm_id_clients, IPC_RMID, NULL);
}

void setup_signal_handler(void)
{
    sigset_t blocked_signals;

    if(sigfillset(&blocked_signals) < 0) {
        bail_out(EXIT_FAILURE, "sigfillset");
    } else {
        const int signals[] = { SIGINT, SIGQUIT, SIGTERM };
        struct sigaction s;
        s.sa_handler = signal_handler;
        (void) memcpy(&s.sa_mask, &blocked_signals, sizeof(s.sa_mask));
        s.sa_flags   = SA_RESTART;
        for(int i = 0; i < COUNT_OF(signals); i++) {
            if (sigaction(signals[i], &s, NULL) < 0) {
                bail_out(EXIT_FAILURE, "sigaction");
            }
        }
    }
}

void clean_close(void)
{

    DEBUG("Clean Close %s\n",program_name);

    if (semrm(sem_client) < 0) {
        (void) bail_out(EXIT_FAILURE,"Error removing the semaphore sem_client (semrm)");
    }
    if (semrm(sem_client2) < 0) {
        (void) bail_out(EXIT_FAILURE,"Error removing the semaphore sem_client2 (semrm)");
    }
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

    // Remove shm game
    if (shmctl(shm_id_clients, IPC_RMID, NULL) < 0) {
        (void) bail_out(EXIT_FAILURE,"Error terminating shared memory of clients (shmctl)");
    }

    exit(EXIT_SUCCESS);
}