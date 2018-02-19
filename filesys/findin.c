/* findin - finds files by glob in PATH or the given colon-delimited
 * environment variable, or instead with a directory list read from from
 * standard input */

#include <err.h>
#include <errno.h>
#include <getopt.h>
#include <glob.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#define STDIN_DELIMITER '\n'
#define STDOUT_DELIMITER '\n'

/* NOTE tilde is a non-standard extension to IEEE Std 1003.2
 * (``POSIX.2''). used to have GLOB_LIMIT as well, though that is
 * problematical on Mac OS X */
#define GLOB_FLAGS GLOB_BRACE | GLOB_TILDE

void check_dir(char *directory, size_t dir_len, const char *expr,
               char output_delim);
void emit_help(void);
void parse_env(const char *envname, const char *expr, char output_delim);
void parse_stdin(const char *expr, char input_delim, char output_delim);

bool Flag_Nulsep;               /* -0 */
bool Flag_Quiet;                /* -q */

int exit_status = EXIT_SUCCESS;
bool File_Hits;
extern int errno;

int main(int argc, char *argv[])
{
    char *env_or_stdin;
    int ch;

    while ((ch = getopt(argc, argv, "0qh?")) != -1) {
        switch (ch) {
        case '0':
            Flag_Nulsep = true;
            break;
        case 'q':
            Flag_Quiet = true;
            break;
        case '?':
        case 'h':
        default:
            emit_help();
            /* NOTREACHED */
        }
    }
    argc -= optind;
    argv += optind;

    if (argc < 1 || argc > 2)
        emit_help();

    env_or_stdin = argc == 2 ? argv[1] : "PATH";

    if (strncmp(env_or_stdin, "-", 2) == 0)
        parse_stdin(argv[0], Flag_Nulsep ? '\0' : STDIN_DELIMITER,
                    Flag_Nulsep ? '\0' : STDOUT_DELIMITER);
    else
        parse_env(env_or_stdin, argv[0], Flag_Nulsep ? '\0' : STDOUT_DELIMITER);

    if (!File_Hits && exit_status == EXIT_SUCCESS)
        exit_status = 2;
    exit(exit_status);
}

void check_dir(char *directory, size_t dir_len, const char *expr,
               char output_delim)
{
    glob_t g;
    int ret;
    size_t tail = dir_len - 1;

    /* NOTE dir_len may be out of sync with directory afterwards, but is
     * not presently relevant hence */
    while (tail > 0 && directory[tail] == '/')
        directory[tail--] = '\0';

    if (chdir(directory) != 0) {
        if (errno != ENOENT && !Flag_Quiet) {
            warn("could not chdir '%s'", directory);
            exit_status = EX_OSERR;
        }
        return;
    }

    if ((ret = glob(expr, GLOB_FLAGS, NULL, &g)) != 0) {
        if (ret != GLOB_NOMATCH && !Flag_Quiet) {
            warn("glob error (%d) for %s/%s", ret, directory, expr);
            exit_status = EX_OSERR;
        }
    } else {
        for (unsigned int i = 0; i < g.gl_pathc; i++) {
            printf("%s/%s%c", directory, g.gl_pathv[i], output_delim);
            File_Hits = true;
        }
    }

    globfree(&g);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: findin [-0] [-q] file-glob [ENVVAR|-]\n"
            "  note that MANPATH likely requires man*/foo* as a glob\n");
    exit(EX_USAGE);
}

void parse_env(const char *envname, const char *expr, char output_delim)
{
    char *directory, *envp, *string, *tofree;
    size_t dir_len;

    if ((envp = getenv(envname)) == NULL)
        errx(EX_USAGE, "no such environment variable '%s'", envname);
    if ((string = strdup(envp)) == NULL)
        err(EX_OSERR, "could not strdup environment");
    tofree = string;

    while ((directory = strsep(&string, ":")) != NULL) {
        dir_len = strlen(directory);
        if (dir_len > 0)
            check_dir(directory, dir_len, expr, output_delim);
    }
    free(tofree);
}

void parse_stdin(const char *expr, char input_delim, char output_delim)
{
    char *line = NULL;
    size_t linebuflen;
    ssize_t numchars;

    while ((numchars = getdelim(&line, &linebuflen, input_delim, stdin)) != -1) {
        if (numchars > 1) {
            /* NOTE line includes the delimiter, except when the caller
             * fails to append the delimiter to the ultimate line */
            if (line[numchars - 1] == input_delim)
                line[--numchars] = '\0';
            check_dir(line, numchars, expr, output_delim);
        }
    }
    if (ferror(stdin)) {
        warn("error reading from standard input");
        exit_status = EX_OSERR;
    }
}
