#ifndef _DNSTW_H
#define _DNSTW_H 1

#include <err.h>
#include <string.h>
#include <sysexits.h>

#include <tcl.h>

int IpParse(ClientData clientData, Tcl_Interp * interp, int objc,
            Tcl_Obj * CONST objv[]);

#endif
