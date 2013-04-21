/*
 * Find a named file by literal or glob (first argument) in PATH by
 * default, or any arbitrary environment variable (second argument) that
 * consists of a colon-delimited list (e.g. MANPATH, and so forth). If the
 * second argument is a hypen, the list of paths to iterate over is read
 * from standard input.
 */

#ifdef __linux__
#include <linux/limits.h>
#else
#include <sys/syslimits.h>
#endif

#include <glob.h>
#include <string.h>
#include <unistd.h>

#include "findin.h"

/*
 * Note that the tilde and limit flags are non-standard extensions to
 * IEEE Std 1003.2 (``POSIX.2'').
 */
#ifdef __linux__
#define GL_FLAGS GLOB_ERR | GLOB_TILDE
#else
#define GL_FLAGS GLOB_ERR | GLOB_TILDE | GLOB_LIMIT
#endif

int opt_quiet = FALSE;
int opt_help = FALSE;

unsigned int hits = 0;

int main(int argc, char *argv[])
{
    char *prog_name;
    extern int optind;
    int ch;

    exit_status = EXIT_SUCCESS;
    prog_name = argv[0];

    while ((ch = getopt(argc, argv, "q?h")) != -1) {
        switch (ch) {
        case '?':
        case 'h':
            opt_help = TRUE;
            break;
        case 'q':
            opt_quiet = TRUE;
            break;
        default:
            break;
        }
    }
    argc -= optind;
    argv += optind;

    if (opt_help == TRUE || argc < 1 || argc > 2) {
        fprintf(stderr, "Usage: %s [-q] file-glob [envvar|-]\n", prog_name);
        exit_status = EX_USAGE;
    } else if (argc < 2) {
        iterate(argv[0], "PATH", ENV_PATH_DELIMITER,
                (int (*)(void *)) get_next_env);
    } else if (strncmp(argv[1], "-", 1) != 0) {
        iterate(argv[0], argv[1], ENV_PATH_DELIMITER,
                (int (*)(void *)) get_next_env);
    } else {
        iterate(argv[0], (char *) NULL, STDIN_PATH_DELIMITER,
                (int (*)(void *)) get_next_stdin);
    }

    if (hits == 0 && exit_status == EXIT_SUCCESS) {
        exit_status = R_NO_HITS;
    }
    exit(exit_status);
}

int get_next_stdin(char *not_used)
{
    return getchar();
}

void iterate(char *file_expr, char *env, int record_delim,
             int (*get_next) (void *))
{
    int c;
    char cur_dir[PATH_MAX];
    unsigned int ix_cur_dir = 0;

    while ((c = (*get_next) (env)) != EOF) {
        if (c == record_delim) {
            if (ix_cur_dir > 0) {
                if (cur_dir[ix_cur_dir - 1] == DIR_DELIMITER) {
                    cur_dir[ix_cur_dir - 1] = '\0';
                } else {
                    cur_dir[ix_cur_dir] = '\0';
                }
                check_dir(cur_dir, file_expr);
                ix_cur_dir = 0;
            }
        } else {
            cur_dir[ix_cur_dir++] = (char) c;
            if (ix_cur_dir >= PATH_MAX) {
                fprintf(stderr, "path exceeds %d characters\n",
                        (int) PATH_MAX);
                exit_status = EX_DATAERR;
                return;
            }
        }
    }
    if (ix_cur_dir > 0) {
        if (cur_dir[ix_cur_dir - 1] == DIR_DELIMITER) {
            cur_dir[ix_cur_dir - 1] = '\0';
        } else {
            cur_dir[ix_cur_dir] = '\0';
        }
        check_dir(cur_dir, file_expr);
    }
}

void check_dir(char *directory, char *file_expr)
{
    char file_path[PATH_MAX];
    char err_msg[PATH_MAX + 32];
    glob_t g;
    int i, ret;

    ret =
        snprintf(file_path, PATH_MAX, "%s%c%s", directory, DIR_DELIMITER,
                 file_expr);
    if (ret < 0) {
        fprintf(stderr, "unexpected return value from snprintf(3): %d\n",
                ret);
        exit_status = EX_OSERR;
        return;
    }
    ret = glob(file_path, GL_FLAGS, NULL, &g);
    if (ret != 0 && ret != GLOB_NOMATCH) {
        /*
         * Hard to distinguish actual errors from spurious warnings, so mask
         * all errors if quiet option set. 
         */
        if (opt_quiet != TRUE) {
            snprintf(err_msg, PATH_MAX + 32, "glob(3) error %d for %s", ret,
                     file_path);
            perror(err_msg);
            exit_status = EX_OSERR;
        }
        return;
    }

    for (i = 0; i < (int) g.gl_pathc; i++) {
        printf("%s\n", g.gl_pathv[i]);
        ++hits;
    }
    globfree(&g);
}
