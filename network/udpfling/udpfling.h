#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <err.h>
#include <errno.h>
#include <limits.h>
#include <netdb.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>

#define DEFAULT_BACKOFF 100000	/* base microseconds for backoff code */
#define MAX_BACKOFF 99999999

#define DEFAULT_DELAY 5		/* milliseconds for delay */
#define MS_IN_SEC 1000		/* milliseconds */
#define USEC_IN_MS 1000000
#define USEC_IN_SEC 10000000
#define NSEC_IN_SEC 1000000000

#define DEFAULT_STATS_INTERVAL 10000	/* how often to print stats */

void catch_intr(int sig);
void emit_usage(void);
int parse_opts(int argc, char *argv[]);

bool Flag_Flood;		/* -f to flood send packets (sender) */
bool Flag_Line_Buf;		/* -l to set line buffering (sink) */
bool Flag_Nanoseconds;		/* -N to use nanoseconds for delay */
int Flag_AI_Family;		/* For -4 or -6 (default UNSPEC) */
unsigned int Flag_Count;	/* -c stats interval */
unsigned int Flag_Delay;	/* -d delay in milliseconds */
unsigned int Flag_Max_Send;	/* -C client max send */
size_t Flag_Padding;		/* -P packet padding size */
char *Flag_Port;		/* -p port name/number to use */
