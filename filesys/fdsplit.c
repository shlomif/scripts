/* fdsplit - parts of a filename, in particular the extension ("gz" of
 * "foo.tar.gz") and root ("foo.tar" to complement the previous) */

#include <sys/types.h>

#include <err.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

typedef char *(*fn_parser) (char *prefix, const char *filepath);

void emit_help(void);
char *parse_ext(char *unused, const char *filename);
char *parse_root(char *prefix, const char *filename);

char Flag_Delimiter = '.';
bool Flag_NullSep;

int main(int argc, char *argv[])
{
    char *filename, *filepart, *lastslash, *prefix;
    fn_parser select_part;
    int ch;

    while ((ch = getopt(argc, argv, "0d:h?")) != -1) {
        switch (ch) {
        case '0':
            Flag_NullSep = true;
            break;
        case 'd':
            Flag_Delimiter = *optarg;
            /* avoid special case for ext if someone gets clever and
             * specifies -d $'\000' or the like */
            if (Flag_Delimiter == '\0')
                errx(EX_DATAERR, "delimiter may not be NUL character");
            break;
        case 'h':
        case '?':
        default:
            emit_help();
            /* NOTREACHED */
        }
    }
    argc -= optind;
    if (argc != 2)
        emit_help();
    argv += optind;

    if (strcmp(*argv, "root") == 0) {
        select_part = parse_root;
    } else if (strcmp(*argv, "ext") == 0) {
        select_part = parse_ext;
    } else {
        warnx("unknown argument '%s'", *argv);
        emit_help();
    }
    argv++;

    /* but first! must only work on the last directory component, if
     * any, as otherwise an ext parse of "/etc/cron.d/blah" would be
     * surprising, at the cost of making the results for "/etc/cron.d/"
     * somewhat surprising. ZSH does the same; consider
     *
     *   `x=/etc/cron.d/; echo ext $x:e; echo root $x:r`
     */
    if ((lastslash = strrchr(*argv, '/')) == NULL) {
        prefix = NULL;
        filename = *argv;
    } else {
        if ((prefix = strndup(*argv, lastslash - *argv + 1)) == NULL)
            err(EX_OSERR, "could not strndup");
        filename = ++lastslash;
    }

    if ((filepart = select_part(prefix, filename)) != NULL) {
        printf("%s%c", filepart, Flag_NullSep ? '\0' : '\n');
        free(filepart);
        filepart = NULL;
        if (prefix != NULL)
            free(prefix);
    }

    exit(EXIT_SUCCESS);
}

void emit_help(void)
{
    fprintf(stderr, "Usage: fdsplit [-0] [-d delim] root|ext filename\n");
    exit(EX_USAGE);
}

char *parse_ext(char *unused, const char *filename)
{
    char *ext, *lastdot;
    if ((lastdot = strrchr(filename, Flag_Delimiter)) == NULL)
        return NULL;
    /* special case for "blah." */
    if (strlen(++lastdot) == 0)
        return NULL;
    if ((ext = strdup(lastdot)) == NULL)
        err(EX_OSERR, "could not strdup");
    return ext;
}

char *parse_root(char *prefix, const char *filename)
{
    char *fullpath, *lastdot, *root;
    size_t plen;
    size_t len = strlen(filename);
    if (len == 0)
        return prefix;
    if ((root = strdup(filename)) == NULL)
        err(EX_OSERR, "could not strdup");
    if ((lastdot = strrchr(root, Flag_Delimiter)) == NULL)
        goto FULLPATH;
    *lastdot = '\0';
    /* special case for ".blah" */
    if ((len = strlen(root)) == 0)
        return prefix;
  FULLPATH:
    if (prefix) {
        plen = strlen(prefix);
        len = strlen(root);
        if ((fullpath = malloc(plen + len + 1)) == NULL)
            err(EX_OSERR, "could not malloc space for path");
        strcpy(fullpath, prefix);
        strcpy(fullpath + plen, root);
        return fullpath;
    } else {
        return root;
    }
}
