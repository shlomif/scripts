#include <stdbool.h>
#include <string.h>

#include "udpfling.h"

int parse_opts(int argc, char *argv[])
{
    extern int Flag_AI_Family;
    extern int Flag_Count;
    extern long Flag_Delay;
    extern int Flag_Flood;
    extern char Flag_Port[MAX_PORTNAM_LEN];
    char *fpp = Flag_Port;

    int ch;
    char *ep;
    long lval;

    bool fourandsix = false;
    bool has_port = false;
    bool delayed_flood = false;

    Flag_AI_Family = AF_UNSPEC;
    Flag_Delay = DEFAULT_DELAY;

    while ((ch = getopt(argc, argv, "46c:d:fp:")) != -1) {
        switch (ch) {
        case '4':
            if (fourandsix)
                emit_usage();
            Flag_AI_Family = AF_INET;
            fourandsix = true;
            break;
        case '6':
            if (fourandsix)
                emit_usage();
            Flag_AI_Family = AF_INET6;
            fourandsix = true;
            break;
        case 'c':
            errno = 0;
            lval = strtol(optarg, &ep, 10);
            if (optarg[0] == '\0' || *ep != '\0')
                errx(EX_DATAERR, "invalid count");
            if (lval >= INT_MAX || lval < 0)
                errx(EX_DATAERR, "count out of range");
            Flag_Count = lval;
            break;
        case 'd':
            if (delayed_flood)
                emit_usage();
            errno = 0;
            lval = strtol(optarg, &ep, 10);
            if (optarg[0] == '\0' || *ep != '\0')
                errx(EX_DATAERR, "invalid delay");
            if (lval >= LONG_MAX || lval < 0)
                errx(EX_DATAERR, "delay out of range");
            Flag_Delay = lval;
            Flag_Flood = 0;
            delayed_flood = true;
            break;
        case 'f':
            if (delayed_flood)
                emit_usage();
            Flag_Flood = 1;
            delayed_flood = true;
            break;
        case 'p':
            if (snprintf(fpp, MAX_PORTNAM_LEN, "%s", optarg) >=
                MAX_PORTNAM_LEN)
                errx(EX_DATAERR, "port option is too long");
            has_port = true;
            break;
        case 'h':
        default:
            emit_usage();
            /* NOTREACHED */
        }
    }

    if (!has_port)
        emit_usage();

    return optind;
}
