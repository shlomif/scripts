/* dnstw - a nsupdate(1) wrapper for dynamic DNS changes */

#include <sys/wait.h>

#include <ctype.h>
#include <getopt.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <goptfoo.h>

#include "dnstw.h"

#define DNSTW_MAX_MODULE_LEN 65

/* FIXME these must be fully qualified paths for a site install */
char *Module_Dir = "modules";
char *Common_Lib = "modules/_common.tcl";
char *Final_Lib = "modules/_final.tcl";

int Flag_AcceptFQDN;            /* -F */
char *Flag_Server;              /* -S */
int Flag_TTL = 3600;            /* -T */
char *Flag_Domain;              /* -d */
int Flag_Preview;               /* -n */

int Option_MinTTL = 1;
int Option_MaxTTL = 31536000;

const char *Nsupdate_Cmd;

Tcl_Interp *Interp;

void call_nsupdate(const char *nsus);
void err_emit_help(void);
void setup_tcl(void);
void stack_dump(int code);

int main(int argc, char *argv[])
{
    char *module, *module_path;
    char *nsupdate_str = "";
    int ch, ret;
    size_t l, len;
    Tcl_Obj *argsPtr;

#ifdef __OpenBSD__
    if (pledge("dns exec getpw proc rpath stdio", NULL) == -1)
        err(EX_OSERR, "pledge failed");
#endif

#ifdef __linux__
    if (prctl(PR_SET_DUMPABLE, 0) == -1)
        err(EX_OSERR, "could not disable PR_SET_DUMPABLE");
#endif

    setlocale(LC_ALL, "C");

    setup_tcl();

    while ((ch = getopt(argc, argv, "FS:T:d:nh?")) != -1) {
        switch (ch) {
        case 'F':
            Flag_AcceptFQDN = 1;
            break;
        case 'S':
            Flag_Server = optarg;
            break;
        case 'T':
            Flag_TTL =
                (int) flagtoul(ch, optarg, (unsigned long) Option_MinTTL,
                               (unsigned long) Option_MaxTTL);
            break;
        case 'd':
            if ((Flag_Domain = strndup(optarg, 251)) == NULL)
                err(EX_OSERR, "strndup failed");
            break;
        case 'n':
            Flag_Preview = 1;
            break;
        case 'h':
        case '?':
        default:
            err_emit_help();
        }
    }
    argc -= optind;
    if (argc < 1)
        err_emit_help();
    argv += optind;

    module = *argv;
    len = strnlen(module, DNSTW_MAX_MODULE_LEN);
    if (len == DNSTW_MAX_MODULE_LEN)
        errx(EX_USAGE, "module name is too long");
    if (!isalnum(*module))
        errx(EX_USAGE, "module name contains an invalid character");
    for (l = 1; l < len; l++) {
        if (!isalnum(module[l]) && module[l] != '-')
            errx(EX_USAGE, "module name contains an invalid character");
    }
    if (asprintf(&module_path, "%s/%s.tcl", Module_Dir, module) == -1)
        err(EX_OSERR, "asprintf failed");

    argc--;
    argv++;
    argsPtr = Tcl_NewListObj(0, (Tcl_Obj **) NULL);
    while (*argv != NULL) {
        Tcl_ListObjAppendElement(Interp, argsPtr, Tcl_NewStringObj(*argv, -1));
        argv++;
    }
    Tcl_SetVar2Ex(Interp, "argv", NULL, argsPtr, 0);
    if (Flag_Domain == NULL || Flag_AcceptFQDN)
        Flag_Domain = "";
    if (Flag_Server == NULL) {
        Flag_Server = "";
    } else {
        if (asprintf(&nsupdate_str, "server %s\n", Flag_Server) == -1)
            err(EX_OSERR, "asprintf failed");
    }
    Tcl_SetVar2(Interp, "nsupdate", NULL, nsupdate_str, 0);

    if (Tcl_CreateObjCommand(Interp, "ipparse", IpParse, (ClientData) NULL,
                             (Tcl_CmdDeleteProc *) NULL) == NULL)
        errx(1, "Tcl_CreateObjCommand failed");

    if ((ret = Tcl_EvalFile(Interp, module_path)) != TCL_OK) {
        if (ret == TCL_ERROR)
            stack_dump(ret);
        errx(1, "module %s failed (%d)", module, ret);
    }

    if ((nsupdate_str = (char *) Tcl_GetVar(Interp, "nsupdate", 0)) == NULL)
        errx(1, "nsupdate is not set");
    if (*nsupdate_str == '\0')
        errx(1, "nothing to send to nsupdate");

    if ((ret = Tcl_EvalFile(Interp, Final_Lib)) != TCL_OK) {
        if (ret == TCL_ERROR)
            stack_dump(ret);
        errx(1, "final library failed (%d)", ret);
    }

    if (Flag_Preview) {
        printf("%s", nsupdate_str);
    } else {
        call_nsupdate(nsupdate_str);
    }
    exit(EXIT_SUCCESS);
}

void call_nsupdate(const char *nsus)
{
    char **nsargs;
    int fd[2], i, status;
    pid_t pid;

    Tcl_Obj *argObj, *nsargsPtr;
    int objcount, ret;

    if (pipe(fd) < 0)
        err(EX_IOERR, "pipe failed");
    if ((pid = fork()) < 0)
        err(EX_OSERR, "fork failed");

    if (pid == 0) {             /* child */
        close(fd[1]);
        if (fd[0] != STDIN_FILENO) {
            if (dup2(fd[0], STDIN_FILENO) != STDIN_FILENO)
                err(EX_IOERR, "dup2 failed");
            close(fd[0]);
        }
        if ((nsargsPtr =
             Tcl_GetVar2Ex(Interp, "nsupdate_args", NULL,
                           TCL_LEAVE_ERR_MSG)) == NULL)
            err(1, "could not get nsupdate_args");
        if ((ret = Tcl_ListObjLength(Interp, nsargsPtr, &objcount)) != TCL_OK)
            err(1, "could not get elements of nsupdate_args");
        nsargs = calloc(objcount + 2, sizeof(char *));
        nsargs[0] = (char *) Nsupdate_Cmd;
        for (i = 0; i < objcount; i++) {
            Tcl_ListObjIndex(Interp, nsargsPtr, i, &argObj);
            nsargs[i + 1] = Tcl_GetStringFromObj(argObj, NULL);
        }
        nsargs[objcount + 1] = (char *) NULL;
        execv(Nsupdate_Cmd, nsargs);
        err(EX_OSERR, "execv failed");
    }

    close(fd[0]);               /* parent */
    dprintf(fd[1], "%s", nsus);
    close(fd[1]);
    if (waitpid(pid, &status, 0) < 0)
        err(EX_OSERR, "waitpid failed");
    if (status != 0)
        errx(1, "nsupdate failed (%d)", status);
}

void err_emit_help(void)
{
    fprintf(stderr,
            "Usage: dnstw [-F | -d domain] [-n] [-S server] [-T TTL] module [args ..]\n");
    exit(EX_USAGE);
}

void setup_tcl(void)
{
    int ret;
    if ((Interp = Tcl_CreateInterp()) == NULL)
        errx(EX_OSERR, "Tcl_CreateInterp failed");
    if (Tcl_Init(Interp) == TCL_ERROR)
        errx(EX_OSERR, "Tcl_Init failed");

    Tcl_LinkVar(Interp, "module_dir", (void *) &Module_Dir,
                TCL_LINK_STRING | TCL_LINK_READ_ONLY);

    Tcl_LinkVar(Interp, "accept_fqdn", (void *) &Flag_AcceptFQDN,
                TCL_LINK_BOOLEAN | TCL_LINK_READ_ONLY);
    Tcl_LinkVar(Interp, "server", (void *) &Flag_Server, TCL_LINK_STRING);
    Tcl_LinkVar(Interp, "TTL", (void *) &Flag_TTL, TCL_LINK_INT);
    Tcl_LinkVar(Interp, "domain", (void *) &Flag_Domain, TCL_LINK_STRING);

    Tcl_LinkVar(Interp, "TTL_MIN", (void *) &Option_MinTTL, TCL_LINK_INT);
    Tcl_LinkVar(Interp, "TTL_MAX", (void *) &Option_MaxTTL, TCL_LINK_INT);

    if ((ret = Tcl_EvalFile(Interp, Common_Lib)) != TCL_OK) {
        if (ret == TCL_ERROR)
            stack_dump(ret);
        errx(1, "common library failed (%d)", ret);
    }

    if ((Nsupdate_Cmd = Tcl_GetVar(Interp, "nsupdate_cmd", 0)) == NULL)
        errx(1, "nsupdate_cmd not set");
}

void stack_dump(int code)
{
    Tcl_Obj *options = Tcl_GetReturnOptions(Interp, code);
    Tcl_Obj *key = Tcl_NewStringObj("-errorinfo", -1);
    Tcl_Obj *stackTrace;
    Tcl_IncrRefCount(key);
    Tcl_DictObjGet(NULL, options, key, &stackTrace);
    fprintf(stderr, "%s\n", Tcl_GetStringFromObj(stackTrace, NULL));
    Tcl_DecrRefCount(key);
}
