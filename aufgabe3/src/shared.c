/**
 * @file shared.c
 * @author David Pfahler (1126287) <e1126287@student.tuwien.ac.at>
 * @brief TODO
 * @date 26.04.2014
 */

#include "shared.h"

/**
 * @brief This variable is set to ensure cleanup is performed only once
 */
volatile sig_atomic_t terminating = 0;

/**
 * @brief id for the shared memory
 */
int shm_id = -1;
int s1 = -1;
int s2 = -1;
int s3 = -1;
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
    
    (void) shmctl(shm_id, IPC_RMID, NULL);

    (void) semrm(s1);
    (void) semrm(s2);
    (void) semrm(s3);
    (void) semrm(s4);
}

void setup_signal_handler (void)
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

    if (semrm(s1) < 0) {
        (void) bail_out(EXIT_FAILURE,"semrm 1");
    }
    if (semrm(s2) < 0) {
        (void) bail_out(EXIT_FAILURE,"semrm 2");
    }
    if (semrm(s3) < 0) {
        (void) bail_out(EXIT_FAILURE,"semrm 3");
    }
    if (semrm(s4) < 0) {
        (void) bail_out(EXIT_FAILURE,"semrm 4");
    }
    
    // Remove shm
    if (shmctl(shm_id, IPC_RMID, NULL) < 0) {
        (void) bail_out(EXIT_FAILURE,"terminating shm (shmctl)");
    }

    exit(EXIT_SUCCESS);
}