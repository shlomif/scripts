/*
 * Checks and reports on access permissions via fts(3) and access(2). Only unix
 * permissions are considered; exotic ACLs or file system encryption or other
 * curiosities (apparmor, selinux) are not checked (unless access(2) somehow
 * knows about those).
 *
 * See also parsepath(1) for a different (and older) take on this task; this
 * utility was motivated by the need to investigate a fairly large directory
 * tree for permissions problems for an arbitrary user, where the file
 * permissions policy was unspecified, so there was a need to see what files
 * were set wrong and why, without the bother of having to screen out all the
 * files with no permissions problems. A better approach in most cases is to
 * specify and enforce some permissions policy; however, this tool might help
 * in figuring out what is broken while trying to devise such a policy.
 *
 * access(2) while handy probably needs replacement with custom code that only
 * checks the given of individual FTSENT items, e.g. r+x for directories and
 * whatever -Rwx is required for other files, as this will avoid repeated
 * checks that access(2) is stated to make on parent directories (that is, to
 * only check / once, /foo once, and then the individual file permissions on
 * /foo/{bar,baz,...} once). Though, as a start-up task would then need to
 * check the parent directories of any starting paths, e.g. / and /foo for a
 * /foo/bar argument, which access(2) does check (often many times?).
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <fts.h>
#include <grp.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

const char *Program_Name;

gid_t Flag_Group_ID;            // -g groupname|gid
uid_t Flag_User_ID;             // -u username|uid
bool Flag_Verbose;              // -v

void emit_help(void);
void showstat(struct stat *sb);

int main(int argc, char *argv[])
{
    FTS *filetree;
    FTSENT *filedat;
    int access_mode = R_OK;
    int ch;
    int fts_options = FTS_LOGICAL;
    int ret = EXIT_SUCCESS;     // optimistic
    struct group *gr;
    struct passwd *pw;

    Program_Name = *argv;

    while ((ch = getopt(argc, argv, "g:h?Ru:vwxX")) != -1) {
        switch (ch) {
        case 'g':
            if (sscanf(optarg, "%u", &Flag_Group_ID) != 1) {
                if ((gr = getgrnam(optarg)) != NULL) {
                    Flag_Group_ID = gr->gr_gid;
                } else {
                    errx(EX_USAGE, "could not parse -g '%s' option", optarg);
                }
            }
            break;

        case 'R':
            access_mode ^= R_OK;
            break;

        case 'u':
            if (sscanf(optarg, "%u", &Flag_User_ID) != 1) {
                if ((pw = getpwnam(optarg)) != NULL) {
                    Flag_User_ID = pw->pw_uid;
                    // implicit default-group-of-user if not already set
                    if (Flag_Group_ID == 0)
                        Flag_Group_ID = pw->pw_gid;
                } else {
                    errx(EX_USAGE, "could not parse -u '%s' option", optarg);
                }
            }
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

    if (argc == 0)
        emit_help();
    if (access_mode == 0) {
        warnx("-R without -w or -x makes little sense?");
        emit_help();
    }

    if (Flag_User_ID != 0 && Flag_User_ID != getuid())
        if (setreuid(Flag_User_ID, (uid_t) - 1) == -1)
            err(EX_OSERR, "could not setreuid() to %u", Flag_User_ID);
    if (Flag_Group_ID != 0 && Flag_Group_ID != getgid())
        if (setregid(Flag_Group_ID, (uid_t) - 1) == -1)
            err(EX_OSERR, "could not setregid() to %u", Flag_Group_ID);

    if ((filetree = fts_open(argv, fts_options, NULL)) == NULL)
        err(EX_OSERR, "could not fts_open()");

    while ((filedat = fts_read(filetree)) != NULL) {
        switch (filedat->fts_info) {
        case FTS_DC:
            if (Flag_Verbose) {
                ret = 1;
                printf("filesystem cycle from '%s' to '%s'\n",
                       filedat->fts_accpath, filedat->fts_cycle->fts_accpath);
            }
            break;

        case FTS_SLNONE:
            if (Flag_Verbose) {
                ret = 1;
                printf("broken symlink '%s'\n", filedat->fts_accpath);
            }
            break;

        case FTS_DNR:
        case FTS_ERR:
        case FTS_NS:
            ret = 1;
            printf("error reading '%s' ", filedat->fts_accpath);
            if (filedat->fts_statp)
                showstat(filedat->fts_statp);
            putchar('\n');
            break;

        case FTS_DEFAULT:
        case FTS_F:
        case FTS_SL:
            if (access(filedat->fts_accpath, access_mode) == -1) {
                ret = 1;
                printf("no access '%s' ", filedat->fts_accpath);
                if (filedat->fts_statp)
                    showstat(filedat->fts_statp);
                putchar('\n');
            }
            break;
        }
    }
    if (errno != 0)
        err(EX_OSERR, "fts_read() error");

    exit(ret);
}

void showstat(struct stat *sb)
{
    printf("mode %04o owner %u:%u", sb->st_mode & 07777, sb->st_uid,
           sb->st_gid);
}

void emit_help(void)
{
    const char *shortname;
    if ((shortname = strrchr(Program_Name, '/')) != NULL)
        shortname++;
    else
        shortname = Program_Name;

    fprintf(stderr,
            "Usage: %s [-X] [-u user] [-g group] [-v] [-Rwx] file [file1 ..]\n",
            shortname);

    exit(EX_USAGE);
}
