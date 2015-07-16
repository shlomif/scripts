/*
 * This is (code for) a very simple shell. Try issuing commands in it
 * (once compiled and run) such as "ls" or "pwd". Then, figure out
 * how to make "cd" work in it (and also how to implement "exit", and
 * how to implement arguments to programs, but those are unrelated to
 * "cd" support).
 *
 * Source for majority of code taken from chapter 1 of:
 * http://www.amazon.com/o/ASIN/0321525949
 * (Well, the previous edition, anyways.)
 *
 * Ideas for implementing 'cd' at end of this file.
 */
#include <sys/types.h>
#include <sys/wait.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>
#define MAXLINE 640
int main(void)
{
    char buf[MAXLINE];
    pid_t pid;
    int status;

    setvbuf(stdout, NULL, _IONBF, (size_t) 0);

    printf("shs%% ");
    while (fgets(buf, MAXLINE, stdin) != NULL) {
        buf[strlen(buf) - 1] = '\0';

        pid = vfork();
        if (pid == 0) {         /* child */
            execlp(buf, buf, NULL);
            warn("could not execlp '%s'", buf);
            _exit(EX_OSERR);
        } else if (pid > 0) {   /* parent */
            if ((pid = waitpid(pid, &status, 0)) < 0)
                warn("waitpid error");
        } else {
            warn("vfork error");
        }

        printf("shs%% ");
    }
    putchar('\n');

    exit(EXIT_SUCCESS);
}

/*
 * FCBVYRE FRPGVBA! Bar vqrn sbe n pq vzcyrzragngvba jbhyq or gb cnefr
 * ohs ivn fgegbx(3), purpx jurgure gung vf abg AHYY naq vf "pq", naq vs
 * fb vafgrnq bs qbvat gur sbex ovg pnyy puqve(2) jvgu jungrire vf
 * yrsgbire sebz gur fgegbx pnyy (ubcrshyyl fbzrguvat, cebonoyl fubhyq
 * purpx gung gbb, naq guvax nobhg ubj gb vzcyrzrag 'pq' irefhf 'pq
 * fbzrqve'). Vs abg 'pq' gura gel rkrpyc nf hfhny.
 */
