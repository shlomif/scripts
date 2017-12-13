CFLAGS=-std=c99 -O2 -Wall -pedantic -fno-diagnostics-color -fstack-protector-all -fPIC -fPIE -pie -pipe

bt: bt.c
copycat: copycat.c
muss-with-environ: muss-with-environ.c
setsid: setsid.c
sgrax: sgrax.c
threading: threading.c

depend:
	cpanm --installdeps ..
	cpanm --installdeps .
	@expect -c "package require Tcl 8.5; package require fileutil 1.13.0"

waitornot: waitornot.c

test: bt copycat sgrax waitornot
	@prove --nocolor

clean:
	@-rm -fr bt copycat muss-with-environ setsid sgrax threading waitornot *.core *.dSYM 2>/dev/null

.PHONY: clean depend test
