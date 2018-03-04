/* parses an IPv4 or IPv6 address in string form via inet_pton(3) and
 * sets the ip, reverse DNS, and DNS type for use by subsequent TCL code
 * (this avoids the need to fork out to v4addr or v6addr) */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "dnstw.h"

#define INET_REVLEN 30
#define INET6_REVLEN 74

int IpParse(ClientData clientData, Tcl_Interp * interp, int objc,
            Tcl_Obj * CONST objv[])
{
    const char *srcaddr;
    char *rp, dstaddr[INET6_ADDRSTRLEN], revarpa[INET6_REVLEN];
    int i, ret;
    struct in_addr v4addr;
    struct in6_addr v6addr;

    Tcl_Obj *ipObj[3];          /* ipaddr 0, reverse 1, type 2 */

    if (objc != 5) {
        Tcl_WrongNumArgs(interp, 1, objv, "string ipaddr reverse type");
        return TCL_ERROR;
    }

    srcaddr = Tcl_GetString(objv[1]);

    if ((ret = inet_pton(AF_INET, srcaddr, &v4addr)) == -1)
        err(EX_OSERR, "inet_pton AF_INET failed");
    if (ret == 1) {
        if (!inet_ntop(AF_INET, &v4addr, dstaddr, INET_ADDRSTRLEN))
            err(EX_OSERR, "inet_ntop AF_INET failed");

        v4addr.s_addr = htonl(v4addr.s_addr);
        snprintf((char *) &revarpa, INET_REVLEN, "%u.%u.%u.%u.in-addr.arpa.",
                 v4addr.s_addr & 0xff,
                 v4addr.s_addr >> 8 & 0xff,
                 v4addr.s_addr >> 16 & 0xff, v4addr.s_addr >> 24 & 0xff);

        ipObj[2] = Tcl_NewStringObj("A", -1);

    } else {                    /* maybe IPv6? */
        if ((ret = inet_pton(AF_INET6, srcaddr, &v6addr)) == -1)
            err(EX_OSERR, "inet_pton AF_INET6 failed");
        if (ret != 1)
            errx(EX_DATAERR, "unable to parse an IP address");

        if (!inet_ntop(AF_INET6, &v6addr, dstaddr, INET6_ADDRSTRLEN))
            err(EX_OSERR, "inet_ntop AF_INET6 failed");

        rp = revarpa;
        for (i = 15; i >= 0; i--) {
            snprintf(rp, 5, "%1x.%1x.", v6addr.s6_addr[i] & 15,
                     v6addr.s6_addr[i] >> 4 & 15);
            rp += 4;
        }
        strncpy(rp, "ip6.arpa.", 9);

        ipObj[2] = Tcl_NewStringObj("AAAA", -1);
    }

    ipObj[0] = Tcl_NewStringObj((const char *) &dstaddr, -1);
    ipObj[1] = Tcl_NewStringObj((const char *) &revarpa, -1);

    for (i = 0; i < 3; i++) {
        if (Tcl_ObjSetVar2
            (interp, objv[2 + i], NULL, ipObj[i], TCL_LEAVE_ERR_MSG) == NULL)
            err(1, "could not set '%s'", Tcl_GetString(objv[2 + i]));
    }

    Tcl_SetObjResult(interp, Tcl_NewBooleanObj(1));
    return TCL_OK;
}
