#ifndef _MINMDA_H
#define _MINMDA_H 1

#if defined(linux) || defined(__linux) || defined(__linux__)
/* some grepping shows this is where linux hides PATH_MAX ? */
#include <linux/limits.h>
#endif

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include <err.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

void gen_filenames(const char *dir, const char *hostid, char **tmp, char **new);
void make_paths(const char *dir);

void tempexit(int sig);

/* if any of these defines change also be sure to review the unit tests */

#define MBUF_MAX 8192

/* sendmail/deliver.c source indicates that this exit code is likely to
 * cause the MTA to temporarily fail (4.x.x) the message; exactly what
 * happens will depend on what is calling minmda, its config, etc */
#define MEXIT_STATUS EX_TEMPFAIL

#endif
