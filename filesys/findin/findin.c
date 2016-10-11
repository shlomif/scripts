/* Find a named file by literal or glob (first argument) in PATH by
 * default, or any arbitrary environment variable (second argument) that
 * consists of a colon-delimited list (e.g. MANPATH or equivalent). If
 * the second argument is a hyphen, the list of paths to iterate over is
 * read from standard input.
 */

#include "findin.h"

/* Note that the tilde and limit flags are non-standard extensions to
 * IEEE Std 1003.2 (``POSIX.2'') */
/* Used to have GLOB_LIMIT here but even when setting g.gl_matchc to a
 * suitably high value a glob for /usr/share/man/man*\/ls* on Mac OS X
 * fails with GLOB_NOSPACE :/ */
#define GL_FLAGS GLOB_BRACE | GLOB_TILDE

bool Flag_Nulsep;               /* -0 */
bool Flag_Quiet;                /* -q */

int exit_status = EXIT_SUCCESS;
unsigned long File_Hits;

int main(int argc, char *argv[])
{
    int ch;

    while ((ch = getopt(argc, argv, "0q?h")) != -1) {
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

    if ((argc == 2 && strncmp(argv[1], "-", 1) == 0)
        || (argc == 1 && !isatty(STDIN_FILENO))) {
        iterate(argv[0], (const char *) NULL, STDIN_PATH_DELIMITER,
                (int (*)(const char *)) get_next_stdin);
    } else if (argc == 2) {
        iterate(argv[0], argv[1], ENV_PATH_DELIMITER,
                (int (*)(const char *)) get_next_env);
    } else {
        iterate(argv[0], "PATH", ENV_PATH_DELIMITER,
                (int (*)(const char *)) get_next_env);
    }

    if (File_Hits == 0 && exit_status == EXIT_SUCCESS)
        exit_status = 2;
    exit(exit_status);
}

void check_dir(char *directory, char *file_expr)
{
    char file_path[PATH_MAX];
    glob_t g;
    unsigned int i;
    int ret;

    g.gl_matchc = ARG_MAX;

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
        /* Hard to distinguish actual errors from spurious warnings, so mask
         * all errors if quiet option set. */
        if (!Flag_Quiet) {
            warnx("glob error %d for %s", ret, file_path);
            exit_status = EX_OSERR;
        }
        return;
    }

    for (i = 0; i < g.gl_pathc; i++) {
        printf("%s%c", g.gl_pathv[i], Flag_Nulsep ? '\0' : '\n');
        File_Hits++;
    }
    globfree(&g);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: findin [-0] [-q] file-glob [envvar|-]\n"
            "  note that MANPATH likely requires */foo* as a glob\n");
    exit(EX_USAGE);
}

int get_next_stdin(const char *not_used)
{
    return getchar();
}

void iterate(char *file_expr, const char *env, int record_delim,
             int (*get_next) (const char *))
{
    int c;
    char cur_dir[PATH_MAX];
    unsigned int ix_cur_dir = 0;

    if (Flag_Nulsep)
        record_delim = '\0';

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
