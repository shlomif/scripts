/*
 * Checks and reports on access permissions via fts(3). Only unix permissions
 * are considered; exotic ACLs or file system encryption or other curiosities
 * (apparmor, selinux) are not checked.
 *
 * See also parsepath(1) for a different (and older) take on this task; this
 * utility was motivated by the need to investigate a fairly large directory
 * tree for permissions problems for an arbitrary user, where the file
 * permissions policy was unspecified, so there was a need to determine which
 * files were set wrong. A better approach in most cases is to specify and
 * enforce some permissions policy; however, this tool might help in figuring
 * out what is broken while trying to devise such a policy. (And really as an
 * excuse to code something up using fts(3).)
 *
 * access(2) while attractive is not used as it according to the docs by
 * default checks the parent directories for each file it is called on; when
 * recursing with fts(3), these should only be checked once. Hence the custom
 * permission checking code (and corresponding risk of stupid bugs therein).
 *
 *   CFLAGS=-std=c99 make permcheck
 *
 * Should build the code on something modernish (Mac OS X 10.9, OpenBSD 5.5,
 * and Slackware 3.10.17 thus far in my limited testing).
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <fts.h>
#include <grp.h>
#include <libgen.h>
#include <limits.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

/* Catch-all exit code for "something awry during fts(3) search" (or during the
 * pre-fts(3) parent directory permissions tests). */
#define EX_PERMS_ERR 1

const char *Program_Name;

gid_t Flag_Group_ID;            // -g groupname|gid
bool Flag_No_Lookups;           // -n
bool Flag_Prune_Dirs;           // -p
uid_t Flag_User_ID;             // -u username|uid
bool Flag_Verbose;              // -v

gid_t *User_Groups;
unsigned int User_Group_Count;
#define MAX_GROUP_COUNT 640

enum { FAIL_USER, FAIL_GROUP, FAIL_OTHER };     // for ugo indication

void emit_help(void);
/* The 'amode' is something like R_OK|X_OK (for directories) that is tested
 * against each of the user, group, and other fields as necessary. */
unsigned int file_access(struct stat *sb, unsigned int amode, int *ugo);
void report_fail(struct stat *sb, char *filepath, unsigned int amode, int ugo);

int main(int argc, char *argv[])
{
    char *username = NULL;
    char *epu, *epg;
    FTS *filetree;
    FTSENT *filedat;
    unsigned int access_mode = R_OK;
    int ch;
    long ugid;
    unsigned int failmodes;
    int failugo;
    int fts_options = FTS_LOGICAL;

    int ret = EXIT_SUCCESS;     // optimistic

    struct group *gr;
    struct passwd *pw;
    struct stat statbuf;

    Program_Name = *argv;

    /* Without -u or -g, permissions default to user running this code. */
    Flag_User_ID = getuid();
    Flag_Group_ID = getgid();

    while ((ch = getopt(argc, argv, "g:h?npRu:vwxX")) != -1) {
        switch (ch) {
        case 'g':
            ugid = strtol(optarg, &epg, 10);
            if (*epg != '\0') {
                if ((gr = getgrnam(optarg)) != NULL) {
                    Flag_Group_ID = gr->gr_gid;
                } else {
                    errx(EX_USAGE,
                         "could not parse -g '%s' option (no such group or gid?)",
                         optarg);
                }
                break;
            }
            if (ugid < 0 || ugid > INT_MAX)
                errx(EX_USAGE, "group id via -g is out of range");
            Flag_Group_ID = (int) ugid;
            break;

        case 'n':
            Flag_No_Lookups = true;
            break;

        case 'p':
            Flag_Prune_Dirs = true;
            break;

        case 'R':
            access_mode ^= R_OK;
            break;

        case 'u':
            ugid = strtol(optarg, &epu, 10);
            if (*epu != '\0') {
                if ((pw = getpwnam(optarg)) != NULL) {
                    username = pw->pw_name;
                    Flag_User_ID = pw->pw_uid;
                    // implicit default-group-of-user if not already set
                    if (Flag_Group_ID == 0)
                        Flag_Group_ID = pw->pw_gid;
		    break;
                } else {
                    errx(EX_USAGE, "could not parse -u '%s' option", optarg);
                }
	    }
            if (ugid < 0 || ugid > INT_MAX)
                errx(EX_USAGE, "user id via -u is out of range");
            Flag_User_ID = (int) ugid;
            break;

        case 'v':
            Flag_Verbose = true;
            break;

        case 'w':
            access_mode |= W_OK;
            break;

        case 'x':
            access_mode |= X_OK;
            break;

        case 'X':
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

    if (argc == 0) {
        warnx("specify files or directories to check");
        emit_help();
    }
    if (access_mode == 0) {
        warnx("-R without -w or -x makes little sense?");
        emit_help();
    }

    /* The username will have been filled in if -u specified, but always need
     * it for the getgrouplist(3) work that in turn is necessary for the
     * permissions checks. Well, unless the user is root, in which case the
     * groups do not apply. */
    if (Flag_User_ID != 0) {
        if (username == NULL) {
            if ((pw = getpwuid(Flag_User_ID)) != NULL) {
                username = pw->pw_name;
            } else {
                errx(EX_NOUSER, "could not lookup username for uid %lu",
                     (unsigned long) Flag_User_ID);
            }
        }

        User_Group_Count = 16;
        if ((User_Groups =
             calloc((size_t) User_Group_Count, sizeof(gid_t))) == NULL)
            err(EX_OSERR, "could not calloc() group ID list");

        /* Portability note - getgrouplist(3) on Slackware 3.10.17 uses 'gid_t'
         * while Mac OS X 10.9.5 and OpenBSD 5.5 use 'int' for various group ID
         * fields. :( */
        while (getgrouplist
               (username, Flag_Group_ID, User_Groups,
                &User_Group_Count) == -1) {
            User_Group_Count <<= 1;
            if (User_Group_Count > MAX_GROUP_COUNT)
                errx(1, "group ID list too large");
            if (realloc(User_Groups, (size_t) User_Group_Count) == NULL)
                err(EX_OSERR, "could not realloc group ID list");
        }

        /* Unlikely awry but gotta check / for sanity ... */
        if (stat("/", &statbuf) == -1)
            err(EX_IOERR, "could not stat '/' ??");

        if ((failmodes = file_access(&statbuf, R_OK | X_OK, &failugo)) != 0) {
            report_fail(&statbuf, (char *) "/", failmodes, failugo);
            if (Flag_Prune_Dirs)
                exit(ret);
        }
    }

    /* User-supplied paths for fts(3) must be made sane (and exist), and parent
     * directories checked for permissions problems, as running fts(3) over
     * /home/jdoe is a waste of time if /home is the source of the problem. */
    char **argp, **pathlist, **plp;
    if ((pathlist = calloc((size_t) argc + 1, sizeof(char *))) == NULL)
        err(EX_OSERR, "could not calloc() path list");
    plp = pathlist;
    argp = argv;
    while (*argp) {
        char *dirtoken, *parentdir, *realp, *realerp;

        if ((realp = realpath(*argp, NULL)) == NULL)
            err(EX_IOERR, "realpath() failed on '%s'", *argp);

        /* fts(3) should check the starting inode, so we only need to cover
         * directories above that. Alas, Linux screws around with the realp, so
         * must make a copy and dirname() that. */
        if (asprintf(&realerp, "%s", realp) == -1)
            err(EX_OSERR, "could not asprintf() real path");
        if ((parentdir = dirname(realerp)) == NULL)
            err(EX_IOERR, "dirname() failed on '%s' from '%s'", realp, *argp);

        parentdir++;            // skip presumed leading root / dir

        bool skipdir = false;
        char *pathportion = (char *) "";
        while ((dirtoken = strsep(&parentdir, "/")) != NULL) {
            if (asprintf(&pathportion, "%s/%s", pathportion, dirtoken) == -1)
                err(EX_OSERR, "could not asprintf() path portion");
            if (stat(pathportion, &statbuf) == -1)
                err(EX_IOERR, "could not stat '%s'", pathportion);

            if ((failmodes = file_access(&statbuf, R_OK | X_OK, &failugo)) != 0) {
                ret = EX_PERMS_ERR;
                report_fail(&statbuf, pathportion, failmodes, failugo);

                if (Flag_Prune_Dirs) {
                    skipdir = true;
                    break;
                }
            }
        }

        if (!skipdir)
            *plp++ = realp;

        argp++;
    }
    *plp = (char *) 0;

    if ((filetree = fts_open(pathlist, fts_options, NULL)) == NULL)
        err(EX_OSERR, "could not fts_open()");

    while ((filedat = fts_read(filetree)) != NULL) {
        switch (filedat->fts_info) {
        case FTS_DC:
            /* A symlink loop lists the symlink twice; ln(1) and link(2) fail
             * to create a hard link on Mac OS X. TODO test on other OS. What
             * is it with OS disallowing hard linked directories? I mean, what
             * could possibly go wrong?? */
            if (Flag_Verbose) {
                printf("filesystem cycle from '%s' to '%s'\n",
                       filedat->fts_accpath, filedat->fts_cycle->fts_accpath);
            }
            break;

        case FTS_SLNONE:
            if (Flag_Verbose) {
                printf("broken symlink '%s'\n", filedat->fts_accpath);
            }
            break;

        case FTS_NS:
            ret = EX_PERMS_ERR;
            printf("could not stat '%s'\n", filedat->fts_accpath);
            break;

        case FTS_DNR:
        case FTS_ERR:
            ret = EX_PERMS_ERR;
            if ((failmodes =
                 file_access(filedat->fts_statp, access_mode, &failugo)) != 0) {
                report_fail(filedat->fts_statp, filedat->fts_accpath, failmodes,
                            failugo);
            }
            /* Report should provide more detail than "Permission denied",
             * though do show the errno if it is not "Permission denied". */
            if (filedat->fts_errno != EACCES) {
                fprintf(stderr, "error reading '%s': %s\n",
                        filedat->fts_accpath, strerror(filedat->fts_errno));
            }
            break;

        case FTS_D:
            if ((failmodes =
                 file_access(filedat->fts_statp, R_OK | X_OK, &failugo)) != 0) {
                ret = EX_PERMS_ERR;
                report_fail(filedat->fts_statp, filedat->fts_accpath, failmodes,
                            failugo);

                if (Flag_Prune_Dirs)
                    fts_set(filetree, filedat, FTS_SKIP);
            }
            break;

        case FTS_DEFAULT:
        case FTS_F:
        case FTS_SL:
            if ((failmodes =
                 file_access(filedat->fts_statp, access_mode, &failugo)) != 0) {
                ret = EX_PERMS_ERR;
                report_fail(filedat->fts_statp, filedat->fts_accpath, failmodes,
                            failugo);
            }
            break;

        default:
            /* Silence a warning; everything relevant should be handled by the
             * above. If not, see fts(3). */
            ;
        }
    }
    if (errno != 0)
        err(EX_OSERR, "fts_read() error");

    exit(ret);
}

void emit_help(void)
{
    const char *shortname;
    if ((shortname = strrchr(Program_Name, '/')) != NULL)
        shortname++;
    else
        shortname = Program_Name;

    fprintf(stderr,
            "Usage: %s "
            "[-g group] [-n] [-p] [-Rwx] [-u user] [-v] [-X] "
            "file [file1 ..]\n", shortname);

    exit(EX_USAGE);
}

/* Checks the permissions on the given stat, returns 0 if access is allowed, or
 * some positive value on failure (the bits that failed the permissions test);
 * amode is something like R_OK|X_OK that is applied to the user, group, and
 * other, in turn. */
unsigned int file_access(struct stat *sb, unsigned int amode, int *ugo)
{
    unsigned int tmode;
    if (Flag_User_ID == 0)
        return 0;

    /* Unix, once gets match on user or group, stops looking at subsequent
     * (Stevens, APUE, ch 4, section 5 (p.81 1st edition; p.94 2nd edition)).
     * This can be confirmed by owning a directory but giving it 0007 modes;
     * the owner should fail read attempts. */
    tmode = amode << 6;
    if (sb->st_uid == Flag_User_ID) {
        if ((sb->st_mode & tmode) == tmode) {
            return 0;
        } else {
            *ugo = FAIL_USER;
            return amode;
        }
    }

    tmode = amode << 3;
    for (unsigned int i = 0; i < User_Group_Count; i++) {
        if (sb->st_gid == (gid_t) User_Groups[i]) {
            if ((sb->st_mode & tmode) == tmode) {
                return 0;
            } else {
                *ugo = FAIL_GROUP;
                return amode;
            }
        }
    }

    if ((sb->st_mode & amode) == amode) {
        return 0;
    } else {
        *ugo = FAIL_OTHER;
        return amode;
    }
}

void report_fail(struct stat *sb, char *filepath, unsigned int amode, int ugo)
{
    char filetype = 'd';
    char modestr[4] = "   ";
    char *mp;
    char ustr = 'o';

    if (!S_ISDIR(sb->st_mode))
        filetype = 'f';

    if (ugo == FAIL_USER) {
        ustr = 'u';
    } else if (ugo == FAIL_GROUP) {
        ustr = 'g';
    }

    mp = modestr;
    if ((amode & R_OK) == R_OK)
        *mp++ = 'r';
    if ((amode & W_OK) == W_OK)
        *mp++ = 'w';
    if ((amode & X_OK) == X_OK)
        *mp++ = 'x';

    if (Flag_No_Lookups) {
        printf("! %c+%s fails: %c %04o %d:%d %s\n", ustr, modestr, filetype,
               sb->st_mode & 07777, sb->st_uid, sb->st_gid, filepath);
    } else {
        char *groupname, *username;
        struct group *gr;
        struct passwd *pw;

        if ((gr = getgrgid(sb->st_gid)) != NULL) {
            groupname = gr->gr_name;
        } else {
            if (asprintf(&groupname, "%lu", (unsigned long) sb->st_gid) == -1)
                err(EX_OSERR, "could not asprintf() gid %d", sb->st_gid);
        }
        if ((pw = getpwuid(sb->st_uid)) != NULL) {
            username = pw->pw_name;
        } else {
            if (asprintf(&username, "%lu", (unsigned long) sb->st_uid) == -1)
                err(EX_OSERR, "could not asprintf() uid %d", sb->st_uid);
        }

        printf("! %c+%s fails: %c %04o %s:%s %s\n", ustr, modestr, filetype,
               sb->st_mode & 07777, username, groupname, filepath);
    }
}
