CC?=gcc
CFLAGS?=-std=c99 -O2 -Wall -pedantic -fstack-protector-all -fPIE -pie -pipe

miditimer: miditimer.c
pedaltime: pedaltime.c
pianoteq: pianoteq.c

depend:
	cpanm --installdeps ..
	cpanm --installdeps .
	@expect -c "package require Tcl 8.5; package require fileutil 1.13.0"

clean:
	@-rm -f miditimer pedaltime 2>/dev/null
