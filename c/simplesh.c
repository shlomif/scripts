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
 */

#include <sys/types.h>
#include <sys/wait.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAXLINE 640

int main(void)
{
    char buf[MAXLINE];
    pid_t pid;
    int status;

    setvbuf(stdout, (char *) 0, _IONBF, 0);

    printf("shs%% ");
    while (fgets(buf, MAXLINE, stdin) != NULL) {
        buf[strlen(buf) - 1] = 0;

        if ((pid = fork()) < 0)
            warn("%s", "fork error");

        else if (pid == 0) {    // child
            execlp(buf, buf, (char *) 0);
            warn("%s%s", "could not execlp ", buf);
            exit(127);
        }

        if ((pid = waitpid(pid, &status, 0)) < 0)       // parent
            warn("%s", "waitpid error");

        printf("shs%% ");
    }
    putchar('\n');

    exit(EXIT_SUCCESS);
}
