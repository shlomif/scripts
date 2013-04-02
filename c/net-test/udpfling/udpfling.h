#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <err.h>
#include <errno.h>
#include <limits.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>

#define DEFAULT_DELAY 5         /* default milliseconds for delay */
#define MS_IN_SEC 1000          /* milliseconds */
#define USEC_IN_SEC 1000000     /* microseconds */

/* /etc/services on OpenBSD 5.2 and Mac OS X 10.8 shows maximum service
 * name length of 15 -- ooops, Debian 6 has one of 16. */
#define MAX_PORTNAM_LEN 32

void emit_usage(void);
int parse_opts(int argc, char *argv[]);

int Flag_AI_Family;             /* For -4 or -6 (default UNSPEC) */
int Flag_Count;                 /* -c packet count|stats interval */
long Flag_Delay;                /* -d delay in milliseconds */
int Flag_Flood;                 /* -f to flood send packets (sender) */
int Flag_Line_Buf;              /* -l to set line buffering (sink) */
char Flag_Port[MAX_PORTNAM_LEN];        /* -p port name/number to use */
