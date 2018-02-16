#include <sys/wait.h>

#include <err.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

/* this must be high enough such that a buggy justblocks that improperly
 * accumulates CPU time can be distinguished from the ususal overhead
 * but not so long that the user despairs */
#define CHILD_WAIT_SECONDS 16

void is_signal(int status, int sig);

int Current_Test = 1;

int main(void)
{
    int status;
    pid_t jbpid, kkpid;
    struct rusage usage;

    setvbuf(stdout, (char *) NULL, _IOLBF, (size_t) 0);

    printf("1..2\n");           /* test count for TAP plan */

    fprintf(stderr, "# tests will block for some time...\n");

    jbpid = fork();
    if (jbpid < 0) {
        err(EX_OSERR, "could not fork justblocks");

    } else if (jbpid == 0) {    /* child - justblocks */
        close(STDIN_FILENO);
        if (execl("./justblocks", "./justblocks", (char *) 0) == -1)
            err(EX_OSERR, "could not exec justblocks");
    }

    kkpid = fork();
    if (kkpid < 0) {
        err(EX_OSERR, "could not fork killer");

    } else if (kkpid == 0) {    /* child - killer */
        sleep(CHILD_WAIT_SECONDS);
        kill(jbpid, SIGUSR2);
        /* do not want killer to count towards any rusage so keep it open */
        while (1)
            sleep(99999);
        return 1;
    }

    wait4(jbpid, &status, 0, &usage);

    is_signal(status, SIGUSR2);

    if (usage.ru_utime.tv_sec + usage.ru_stime.tv_sec < CHILD_WAIT_SECONDS / 4) {
        printf("ok %u - rusage check\n", Current_Test);
    } else {
        printf("not ok %u - rusage check\n", Current_Test);
    }
    Current_Test++;

    kill(kkpid, SIGTERM);
    exit(EXIT_SUCCESS);
}

void is_signal(int status, int sig)
{
    if (WIFSIGNALED(status)) {
        if (WTERMSIG(status) == sig) {
            printf("ok %u - exit signal %d\n", Current_Test, sig);
        } else {
            fprintf(stderr, "# exit status %d\n", status);
            printf("not ok %u - exit signal %d", Current_Test, sig);
        }
    } else {
        printf("not ok %u - exit signal %d", Current_Test, sig);
    }
    Current_Test++;
}
