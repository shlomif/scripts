#CFLAGS = -g
CC=gcc
CFLAGS += -O2 -std=c99 -Wall -pedantic -fstack-protector-all -fPIE -pie -pipe

fdsplit: fdsplit.c
fillerup: fillerup.c
findin: findin.c
getpof: getpof.c
llcount: llcount.c
permcheck: permcheck.c
stripeofeol: stripeofeol.c
unatomappend: unatomappend.c

depend:
	cpanm --installdeps ..
	@expect -c "package require Tcl 8.5; package require fileutil 1.14.11"

test: fdsplit findin getpof llcount
	@prove

clean:
	@-rm -f fdsplit fillerup findin getpof llcount permcheck stripeofeol unatomappend *.core 2>/dev/null
