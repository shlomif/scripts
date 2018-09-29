CFLAGS=-std=c99 -O3 -Wall -pedantic -fno-diagnostics-color -fstack-protector-all -fPIC -fPIE -pie -pipe

miditimer: miditimer.c
pedaltime: pedaltime.c
pianoteq: pianoteq.c

clean:
	git clean --force -x

depend:
	cpanm --installdeps ..
	cpanm --installdeps .
	@expect -c "package require Tcl 8.5; package require fileutil 1.13.0"

.PHONY: clean depend
