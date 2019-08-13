divert(-1)
todo - I used to do fancier things for the todo list

   alias todo='VISUAL=ed todo'

define(`TODO_FILE', `"'esyscmd(`printf "%s" "$HOME"')`/todo"')dnl
divert(0)dnl
#include <sys/file.h>
#include <sys/wait.h>

#include <err.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

enum { NOPE = -1, CHILD };

int main(void)
{
    char *editor;
    int fd, status;
    pid_t pid;
#ifdef __OpenBSD__
    if (pledge("cpath exec flock proc rpath stdio", NULL) == -1)
        err(1, "pledge failed");
#endif
    if ((fd = open(TODO_FILE, O_CREAT | O_RDONLY, 0660)) == -1)
        err(EX_IOERR, "open failed '%s'", TODO_FILE);
    if (flock(fd, LOCK_EX) == -1)
        err(EX_IOERR, "flock failed");
    pid = fork();
    if (pid == NOPE)
        err(EX_OSERR, "fork failed");
    if (pid == CHILD) {
        editor = getenv("VISUAL");
        if (editor == NULL || *editor == '\0') {
            editor = getenv("EDITOR");
            if (editor == NULL || *editor == '\0')
                editor = "ed";
        }
        execlp(editor, editor, TODO_FILE, (char *) 0);
        err(1, "exec '%s' failed", editor);
    } else {
        wait(&status);
    }
    flock(fd, LOCK_UN);
    close(fd);
    exit(EXIT_SUCCESS);
}
