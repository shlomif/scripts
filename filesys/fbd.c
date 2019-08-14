/* fbd - find files by (or around a) date */

#include <sys/types.h>
#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <getopt.h>
#include <fts.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

// https://github.com/thrig/goptfoo
#include <goptfoo.h>

enum { FBD_MTIME, FBD_ATIME, FBD_CTIME };

time_t Flag_Around;             /* -a */
int Flag_AroundF;
time_t Flag_Before;             /* -b */
int Flag_BeforeF;
int Flag_Field;                 /* -s */
int Flag_Null;                  /* -0 */
int Flag_Quiet;                 /* -q */
int Flag_Walk;                  /* -L or -P */

void emit_help(void);
time_t sbtime(struct stat *sb);
time_t parse_duration(char *durp);
long spec2secs(const int d, const char c);

int main(int argc, char *argv[])
{
    int ch, exit_status = EXIT_SUCCESS;
    struct stat sb;

    char **pathlist, **plp;
    FTS *filetree;
    FTSENT *filedat;
    int fts_options = FTS_COMFOLLOW;

    time_t epoch, min_epoch, max_epoch, target_epoch;

#ifdef __OpenBSD__
    if (pledge("rpath stdio", NULL) == -1)
        err(1, "pledge failed");
#endif

    while ((ch = getopt(argc, argv, "h?0LPa:b:e:f:s:qx")) != -1) {
        switch (ch) {
        case '0':
            Flag_Null = 1;
            break;
        case 'L':
            Flag_Walk = FTS_LOGICAL;
            break;
        case 'P':
            Flag_Walk = FTS_PHYSICAL;
            break;
        case 'a':
            Flag_Around = parse_duration(optarg);
            Flag_AroundF = 1;
            break;
        case 'b':
            Flag_Before = parse_duration(optarg);
            Flag_BeforeF = 1;
            break;
        case 's':
            switch (*optarg) {
            case 'a':
                Flag_Field = FBD_ATIME;
                break;
            case 'c':
                Flag_Field = FBD_CTIME;
                break;
            case 'm':
                Flag_Field = FBD_MTIME;
                break;
            default:
                emit_help();
            }
            break;
        case 'q':
            Flag_Quiet = 1;
            break;
        case 'x':
            fts_options |= FTS_XDEV;
            break;
        case 'h':
        case '?':
        default:
            emit_help();
            /* NOTREACHED */
        }
    }
    argc -= optind;
    argv += optind;

    /* these used to be options. however, they are not optional */
    if (strncmp(*argv, "epoch", 6)==0) {
        /* TODO see how this behaves on systems with 32-bit time_t */
        target_epoch = (time_t) argtoll("epoch", argv[1], LONG_MIN, LONG_MAX);
    } else if (strncmp(*argv, "file", 5)==0) {
        if (stat(argv[1], &sb) != 0)
            err(1, "could not stat '%s'", argv[1]);
        target_epoch = sbtime(&sb);
    } else {
        emit_help();
    }
    argc -= 2;
    argv += 2;

    if (Flag_BeforeF) {
        min_epoch = target_epoch - Flag_Before;
        /* -a "around" means "after" if -b supplied */
        max_epoch = target_epoch + (Flag_AroundF ? Flag_Around : 0);
    } else if (Flag_AroundF) {
        min_epoch = target_epoch - Flag_Around;
        max_epoch = target_epoch + Flag_Around;
    } else {
        min_epoch = max_epoch = target_epoch;
    }

    fts_options |= Flag_Walk == FTS_LOGICAL ? FTS_LOGICAL : FTS_PHYSICAL;

    if ((pathlist =
         calloc((size_t) (argc == 0 ? 2 : argc + 1), sizeof(char *))) == NULL)
        err(EX_OSERR, "calloc path list failed");
    plp = pathlist;
    if (argc == 0) {
        *plp++ = (char *) ".";
    } else {
        while (*argv)
            *plp++ = *argv++;
    }
    *plp = (char *) 0;

    if ((filetree = fts_open(pathlist, fts_options, NULL)) == NULL)
        err(EX_OSERR, "fts_open failed");
    while ((filedat = fts_read(filetree)) != NULL) {
        switch (filedat->fts_info) {
        case FTS_D:
        case FTS_F:
        case FTS_SL:
        case FTS_DEFAULT:
            epoch = sbtime(filedat->fts_statp);
            if (epoch >= min_epoch && epoch <= max_epoch) {
                if (Flag_Null) {
                    fputs(filedat->fts_path, stdout);
                    putchar('\0');
                } else {
                    puts(filedat->fts_path);
                }
            }
            break;
        case FTS_DP:
            break;
        case FTS_NS:
            exit_status = EX_NOPERM;
            if (!Flag_Quiet)
                warnx("failed to stat %s: %s", filedat->fts_path,
                      strerror(filedat->fts_errno));
            break;
        case FTS_DNR:
            exit_status = EX_NOPERM;
            if (!Flag_Quiet)
                warnx("failed to open %s directory: %s", filedat->fts_path,
                      strerror(filedat->fts_errno));
            break;
        case FTS_DC:
            /* `ln . foo` is one way (thanks to my users) that a
             * filesystem loop can be created. so it is handy to warn
             * about these... */
            exit_status = 13;
            if (!Flag_Quiet)
                warnx("filesystem cycle from '%s' to '%s'\n",
                      filedat->fts_accpath, filedat->fts_cycle->fts_accpath);
            break;
        case FTS_ERR:
            err(1, "fts_read failed??");
        default:
            ;                   /* do nothing but silence warnings */
        }
    }
    if (errno != 0)
        err(EX_OSERR, "fts_read() error");

    exit(exit_status);
}

void emit_help(void)
{
    fputs("Usage: fbd [opts] epoch-or-file e|f [file ..]\n", stderr);
    exit(EX_USAGE);
}

/* parse a duration, could either be "42" or "1m3s" */
time_t parse_duration(char *durp)
{
    char duration_spec;
    int advance, ret;
    unsigned int duration = 0;
    time_t dur = 0;

    // limit here of 9 so cannot overflow unsigned int
    ret = sscanf(durp, "%9u%n", &duration, &advance);
    if (ret != 1)
        errx(EX_DATAERR,
             "duration must be positive integer or short form (e.g. 62 or 1m2s)");
    durp += advance;

    ret = sscanf(durp, "%1c%n", &duration_spec, &advance);
    if (ret == 0 || ret == EOF) {
        /* no following character, so just a raw value */
        dur = duration;

    } else if (ret == 1) {
        /* human short form, perhaps */
        durp += advance;
        dur += spec2secs(duration, duration_spec);

        while ((ret =
                sscanf(durp, "%9u%1c%n", &duration, &duration_spec,
                       &advance)) == 2) {
            durp += advance;
            dur += spec2secs(duration, duration_spec);
        }

    } else {
        err(EX_SOFTWARE, "unknown return '%d' while parsing duration??", ret);
    }

    /* An attacker could make the timeout stupidly large, but not negative */
    if (dur < 1)
        err(EX_DATAERR, "duration must work out to a positive integer");

    return dur;
}

inline time_t sbtime(struct stat * sb)
{
    switch (Flag_Field) {
    case FBD_MTIME:
        return sb->st_mtim.tv_sec;
    case FBD_CTIME:
        return sb->st_ctim.tv_sec;
        break;
    case FBD_ATIME:
        return sb->st_atim.tv_sec;
        break;
    default:
        err(1, "unknown field (%d) ??", Flag_Field);
    }
}

/* converts 3, "m" or whatever into an appropriate number of seconds */
long spec2secs(const int d, const char c)
{
    long new_duration = 0;

    switch (c) {
    case 's':
        new_duration = d;
        break;
    case 'm':
        new_duration = d * 60;
        break;
    case 'h':
        new_duration = d * 3600;
        break;
    case 'd':
        new_duration = d * 86400;
        break;
    case 'w':
        new_duration = d * 604800;
        break;
    default:
        err(EX_DATAERR,
            "unknown duration specification '%c', must be one of 'smhdw'", c);
    }

    return new_duration;
}
