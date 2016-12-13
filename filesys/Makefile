#CFLAGS = -g
CC=gcc
CFLAGS += -O2 -std=c99 -Wall -pedantic -fstack-protector-all -fPIE -pie -pipe

fdsplit: fdsplit.c
fillerup: fillerup.c
getpof: getpof.c
llcount: llcount.c
permcheck: permcheck.c
stripeofeol: stripeofeol.c
unatomappend: unatomappend.c

depend:
	cpanm --installdeps ..

test: fdsplit
	@prove

clean:
	@-rm -f fdsplit fillerup getpof llcount permcheck stripeofeol unatomappend *.core 2>/dev/null
