CC?=gcc
CFLAGS?=-std=c99 -O2 -Wall -pedantic -fno-diagnostics-color -fstack-protector-all -fPIE -pie -pipe

# NOTE on linux need to add -lbsd
# NOTE RHEL6 bsdlib borked, try compile on something else:
#   /usr/include/bsd/stdlib.h:52: error: expected ‘)’ before ‘*’ token
corrupt: corrupt.c
	$(CC) $(CFLAGS) `pkg-config --cflags --libs goptfoo` -o corrupt corrupt.c
fdsplit: fdsplit.c
fillerup: fillerup.c
findin: findin.c
getpof: getpof.c
llcount: llcount.c
permcheck: permcheck.c
stripeofeol: stripeofeol.c
twiddle: twiddle.c
	$(CC) $(CFLAGS) `pkg-config --cflags --libs goptfoo` -o twiddle twiddle.c
unatomappend: unatomappend.c

# needs libbsd libbsd-devel (RPM) or equivalent packages installed on linux
makeloimage: makeloimage.c
	$(CC) $(CFLAGS) -lbsd -o makeloimage makeloimage.c

depend:
	cpanm --installdeps ..
	@expect -c "package require Tcl 8.5; package require fileutil 1.13.0"

test: fdsplit findin getpof llcount twiddle
	@prove --nocolor

clean:
	@-rm -f corrupt fdsplit fillerup findin getpof llcount permcheck stripeofeol twiddle unatomappend *.core 2>/dev/null
