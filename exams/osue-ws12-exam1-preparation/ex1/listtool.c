#include <stdio.h>
#include <stdlib.h>             /* EXIT_FAILURE and EXIT_SUCCESS */
#include <assert.h>             /* assert() */
#include <string.h>
#include <limits.h>
#include <errno.h>
#include "list.h"

#define STREQ(a,b) (strcmp((a), (b)) == 0)

/* globals */
static const char *command = "<not yet set>";

/* prototypes for used functions */
static void usage(void);

/* implementation for challenge 2 */
static void insert_after(struct listelem *after, const char *const value)
{
	struct listelem *tmp = after->next;
	after->next = malloc(sizeof(struct listelem));
	after->next->next = tmp;
	after->next->val = strdup(value);
}

int main(int argc, char **argv)
{
    /* do not touch */
    struct listelem *head;
    struct listelem *current;

    command = argv[0];
    head = init_list("head");
    populate_list(head);
    current = head;
    /* end do not touch */

    int opt;
    int opt_s = -1, opt_a = -1;
    char *optstr;               /* used to point to <string> */

    /* START OF INTENTIONAL ERRORS */
    while ((opt = getopt(argc, argv, "sa:")) != -1) {
        switch (opt) {
        case 's':
            if (opt_s != -1) {
                (void) fprintf(stderr, "opt_s multiple times\n");
                (void) usage();         /* does not return */
            }
            opt_s = 1;
            break;
        case 'a':
            if (opt_a != -1) {
                (void) fprintf(stderr, "opt_a multiple times\n");
                (void) usage();        /* does not return */
            }
			char *endptr;
			long val;
			errno = 0;
			val = strtol(optarg,&endptr,10);
			if((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
				|| (errno != 0 && val == 0)) {
				(void) fprintf(stderr, "strtol\n");
				(void) usage();
			}

			if(endptr == optarg){
				(void) fprintf(stderr, "strtol\n");
				(void) usage();
			}

			if(*endptr != '\0'){
				(void) fprintf(stderr, "Further character after number: %s\n", endptr);	
			}
            opt_a = val;         /* strtol return checking not required */
            break;
		case '?':
			(void) fprintf(stderr, "unknown flag.\n");
			(void) usage();
		default:
			assert(0);
			break;
            /* maybe add some extra checks for unknown options */
        }

    }
	if(((opt_s == -1) && (opt_a == -1)) || ((opt_s > 0) && (opt_a > 0))){
		(void) fprintf(stderr, "use eighter the flag -a or the flag -s.\n");
		(void) usage();
	}
	
	if(optind >= argc){
		(void) fprintf(stderr, "you need an argument\n");
		(void) usage();
	}
	
	if(optind+1 < argc){
		(void) fprintf(stderr, "Too many arguments\n");
		(void) usage();
	}
	optstr = argv[optind];
    /* END OF INTENTIONAL ERRORS */

    /* CHALLENGE 0:
     * 1) find the intentional failures in the marked section
     * 2) make program synopsis conformant, especially implement the following checks: */

    /* make sure that the user has not given both (-a and -s) options */
    /* make sure that the user has given at least one (-a or -s) option */
    /* point optstr to <string> (do not forget to check optind) */
    /* END OF CHALLENGE 0 */

    /* CHALLENGE 1:
     * iterate over the whole list and find the given <string>
     * remember: the 'next' pointer of the last list entry is set to NULL.
     * for every element print:
     * 'yes,' if it is equal to <string>, else 'no,' */

    if (opt_s != -1) {
		while(current != NULL){
            if (STREQ(current->val,optstr )) {  
                printf("yes,");
            } else {
                printf("no,");
            }
			current = current->next;
        }
        printf("\n");           /* do not remove this line */
    }
    /* END OF CHALLENGE 1 */

    /* CHALLENGE 2:
     * insert the string <string> *after* the NUM-th element of the list
     * if NUM is larger than the number of list entries - 1, append the entry after the last element.
     * for debugging, the list can be printed with 'print_list(head)' */
    if (opt_a != -1) {
        /* iterate over the list and stop at the right entry */
        while(opt_a > 0 && current->next != NULL) {
			current = current->next;
			opt_a--;
        }
        insert_after(current, optstr);  /* assuming you stopped at current */
        check_list(head);       /* do not remove this line or your solution does not count */
    }
    /* END OF CHALLENGE 2 */

    /* After all challenges, free the list */
    destroy_list(head);         /* do not remove this line */
    return 0;
}

static void usage(void)
{
    fprintf(stderr, "SYNOPSIS: %s [-s | -a NUM] <string>\n", command);
    fprintf(stderr,
            "\t-s and -a are only allowed once\n"
            "\tone of them has to be given\n"
            "\t<string> is mandatory\n"
            "\t-s iterates over the list and searches the string <string> in the list\n"
            "\t-a inserts the string <string> *after* the NUM-th element of the list\n"
            "\tif NUM is larger than the number of list entries - 1, append the entry after the last element\n"
            "\tfor debugging, the whole list can be printed with 'print_list' (cf., list.h)\n");

    fprintf(stderr,
            "EXAMPLES: (assume the list consists of the entries 'osue', 'test', 'ss2012')\n");
    fprintf(stderr, "\t%s => print usage\n", command);
    fprintf(stderr, "\t%s -s -s foo => print usage\n", command);
    fprintf(stderr, "\t%s foo => print usage\n", command);
    fprintf(stderr, "\t%s -s foo => no,no,no,\n", command);
    fprintf(stderr, "\t%s -s ss2012 => no,no,yes,\n", command);
    fprintf(stderr,
            "\t%s -a 0 foo => list becomes: osue, foo, test, ss2012\n",
            command);
    fprintf(stderr,
            "\t%s -a 2 foo => list becomes: osue, test, ss2012, foo\n",
            command);
    fprintf(stderr,
            "\t%s -a 23 foo => list becomes: osue, test, ss2012, foo\n",
            command);

    exit(EXIT_FAILURE);
}
