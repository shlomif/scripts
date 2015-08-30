/***********************************************************************
 *
 * Option parsing for udpfling utils.
 *
 */

// https://github.com/thrig/goptfoo
#include <goptfoo.h>

#include "udpfling.h"

int parse_opts(int argc, char *argv[])
{
    int ch;

    bool fourandsix = false;
    bool delayed_flood = false;

    Flag_AI_Family = AF_UNSPEC;
    Flag_Count = DEFAULT_STATS_INTERVAL;
    Flag_Delay = DEFAULT_DELAY;
    Flag_Padding = sizeof(uint32_t);

    while ((ch = getopt(argc, argv, "46C:c:d:flNP:p:")) != -1) {
        switch (ch) {

        case '4':
            if (fourandsix) {
                warnx("need just one of -4 or -6");
                emit_usage();
            }
            Flag_AI_Family = AF_INET;
            fourandsix = true;
            break;
        case '6':
            if (fourandsix) {
                warnx("need just one of -4 or -6");
                emit_usage();
            }
            Flag_AI_Family = AF_INET6;
            fourandsix = true;
            break;

        case 'C':
            Flag_Max_Send = flagtoul(ch, optarg, 0UL, (unsigned long) INT_MAX);
            break;

        case 'c':
            Flag_Count = flagtoul(ch, optarg, 0UL, (unsigned long) INT_MAX);
            break;

        case 'd':
            if (delayed_flood) {
                warnx("cannot both delay and flood packets");
                emit_usage();
            }
            Flag_Delay =
                (unsigned int) flagtoul(ch, optarg, 0UL,
                                        (unsigned long) INT_MAX);
            Flag_Flood = 0;
            delayed_flood = true;
            break;

        case 'f':
            if (delayed_flood) {
                warnx("cannot both delay and flood packets");
                emit_usage();
            }
            Flag_Flood = true;
            delayed_flood = true;
            break;

        case 'l':
            Flag_Line_Buf = true;
            break;

        case 'N':
            Flag_Nanoseconds = true;
            break;

        case 'P':
            // NOTE greatly restrict max size of packet by default
            Flag_Padding = (size_t) flagtoul(ch, optarg, 0UL, 8192UL);
            // ... but do need a minimum size for the counter in the packet
            if (Flag_Padding < sizeof(uint32_t))
                Flag_Padding = sizeof(uint32_t);
            break;

        case 'p':
            Flag_Port = optarg;
            break;

        case 'h':
        default:
            emit_usage();
            /* NOTREACHED */
        }
    }

    if (!Flag_Port) {
        warnx("-p port option is mandatory");
        emit_usage();
    }

    return optind;
}
