#include "udpfling.h"

int parse_opts(int argc, char *argv[])
{
    int ch;
    char *ep;
    long lval;

    bool fourandsix = false;
    bool has_port = false;
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
	    errno = 0;
	    lval = strtol(optarg, &ep, 10);
	    if (optarg[0] == '\0' || *ep != '\0')
		errx(EX_DATAERR, "invalid -C count");
	    if (lval >= INT_MAX || lval < 0)
		errx(EX_DATAERR, "-C count out of range");
	    Flag_Max_Send = (unsigned long) lval;
	    break;

	case 'c':
	    errno = 0;
	    lval = strtol(optarg, &ep, 10);
	    if (optarg[0] == '\0' || *ep != '\0')
		errx(EX_DATAERR, "invalid -c count");
	    if (lval >= INT_MAX || lval < 0)
		errx(EX_DATAERR, "-c count out of range");
	    Flag_Count = (unsigned long) lval;
	    break;

	case 'd':
	    if (delayed_flood) {
		warnx("cannot both delay and flood packets");
		emit_usage();
	    }
	    errno = 0;
	    lval = strtol(optarg, &ep, 10);
	    if (optarg[0] == '\0' || *ep != '\0')
		errx(EX_DATAERR, "invalid delay");
	    if (lval >= INT_MAX || lval < 0)
		errx(EX_DATAERR, "delay -d out of range");
	    Flag_Delay = (unsigned long) lval;
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
	    errno = 0;
	    lval = strtol(optarg, &ep, 10);
	    if (optarg[0] == '\0' || *ep != '\0')
		errx(EX_DATAERR, "invalid delay");
	    /* NOTE greatly restrict max size of packet by default */
	    if (lval > 8192 || lval < 0)
		errx(EX_DATAERR, "padding size out of range");
	    /* but do need a minimum size for the counter in the packet  */
	    Flag_Padding =
		(size_t) lval <
		sizeof(uint32_t) ? sizeof(uint32_t) : (size_t) lval;
	    break;

	case 'p':
	    if (asprintf(&Flag_Port, "%s", optarg) == -1)
		err(EX_DATAERR, "could not copy -p port option");
	    has_port = true;
	    break;

	case 'h':
	default:
	    emit_usage();
	    /* NOTREACHED */
	}
    }

    if (!has_port) {
	warnx("-p port option is mandatory");
	emit_usage();
    }
    return optind;
}
