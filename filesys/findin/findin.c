/*
 * Find a named file by literal or glob (first argument) in PATH by default, or
 * any arbitrary environment variable (second argument) that consists of a
 * colon-delimited list (e.g. MANPATH or equivalent). If the second argument is
 * a hyphen, the list of paths to iterate over is read from standard input.
 */

/* ugh, linux */
#ifdef __linux__
#include <linux/limits.h>
#else
#include <sys/syslimits.h>
#endif

#include <glob.h>
#include <string.h>
#include <unistd.h>

#include "findin.h"

/* Note that the tilde and limit flags are non-standard extensions to IEEE Std
 * 1003.2 (``POSIX.2''). */
#ifdef __linux__
#define GL_FLAGS GLOB_ERR | GLOB_TILDE
#else
#define GL_FLAGS GLOB_ERR | GLOB_TILDE | GLOB_LIMIT
#endif

bool Flag_Quiet;                /* -q */

unsigned int File_Hits;

void emit_help(void);

int main(int argc, char *argv[])
{
    int ch;

    while ((ch = getopt(argc, argv, "q?h")) != -1) {
        switch (ch) {
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

    exit_status = EXIT_SUCCESS;

    if (argc < 2) {
        iterate(argv[0], "PATH", ENV_PATH_DELIMITER,
                (int (*)(char const *)) get_next_env);
    } else if (strncmp(argv[1], "-", 1) != 0) {
        iterate(argv[0], argv[1], ENV_PATH_DELIMITER,
                (int (*)(char const *)) get_next_env);
    } else {
        iterate(argv[0], (char *) NULL, STDIN_PATH_DELIMITER,
                (int (*)(char const *)) get_next_stdin);
    }

    if (File_Hits == 0 && exit_status == EXIT_SUCCESS) {
        exit_status = R_NO_HITS;
    }
    exit(exit_status);
}

int get_next_stdin(char const *not_used)      // ignore warn as f(x) ptr need
{
    return getchar();
}

void iterate(char const *file_expr, char const *env, int const record_delim,
             int (*get_next) (char const *))
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
            cur_dir[ix_cur_dir++] = (char)c;
            if (ix_cur_dir >= PATH_MAX) {
                fprintf(stderr, "path exceeds %d characters\n", PATH_MAX);
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

void check_dir(char const *directory, char const *file_expr)
{
    char file_path[PATH_MAX];
    char err_msg[PATH_MAX + 32];
    glob_t g;
    unsigned int i;
    int ret;

    ret =
        snprintf(file_path, PATH_MAX, "%s%c%s", directory, DIR_DELIMITER,
                 file_expr);
    if (ret < 0) {
        fprintf(stderr, "unexpected return value from snprintf(3): %d\n", ret);
        exit_status = EX_OSERR;
        return;
    }
    ret = glob(file_path, GL_FLAGS, NULL, &g);
    if (ret != 0 && ret != GLOB_NOMATCH) {
        /*
         * Hard to distinguish actual errors from spurious warnings, so mask
         * all errors if quiet option set. 
         */
        if (!Flag_Quiet) {
            snprintf(err_msg, PATH_MAX + 32, "glob(3) error %d for %s", ret,
                     file_path);
            perror(err_msg);
            exit_status = EX_OSERR;
        }
        return;
    }

    for (i = 0; i < g.gl_pathc; i++) {
        printf("%s\n", g.gl_pathv[i]);
        File_Hits++;
    }
    globfree(&g);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: findin [-q] file-glob [envvar|-]\n"
            "  note that MANPATH likely requires */foo* as a glob\n");
    exit(EX_USAGE);
}
