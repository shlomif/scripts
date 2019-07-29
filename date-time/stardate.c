/* stardate - show the stardate */

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>

#include <tcl.h>

int main(void)
{
    int ret;
    Tcl_Interp *Interp = NULL;
    if ((Interp = Tcl_CreateInterp()) == NULL)
        errx(EX_OSERR, "Tcl_CreateInterp failed");
    if (Tcl_Init(Interp) == TCL_ERROR)
        errx(EX_OSERR, "Tcl_Init failed");
    ret = Tcl_EvalEx(Interp, "clock format [clock seconds] -format %Q", 39,
                     TCL_EVAL_GLOBAL);
    if (ret != TCL_OK)
        errx(1, "TCL failed (%d): %s", ret, Tcl_GetStringResult(Interp));
    puts(Tcl_GetStringResult(Interp));
    exit(EXIT_SUCCESS);
}
