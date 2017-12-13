CFLAGS=-std=c99 -O2 -Wall -pedantic -fno-diagnostics-color -fstack-protector-all -fPIC -fPIE -pie -pipe

corrupt: corrupt.c
	$(CC) $(CFLAGS) `pkg-config --cflags --libs goptfoo` corrupt.c -o corrupt

fdsplit: fdsplit.c
fillerup: fillerup.c
findin: findin.c
findup: findup.c
getpof: getpof.c
llcount: llcount.c

nomfd: nomfd.c
	$(CC) $(CFLAGS) `pkg-config --cflags --libs goptfoo` nomfd.c -o nomfd

permcheck: permcheck.c

seek: seek.c
	$(CC) $(CFLAGS) `pkg-config --cflags --libs goptfoo` seek.c -o seek

stripeofeol: stripeofeol.c

twiddle: twiddle.c
	$(CC) $(CFLAGS) `pkg-config --cflags --libs goptfoo` twiddle.c -o twiddle

unatomappend: unatomappend.c

# needs libbsd libbsd-devel (RPM) or equivalent packages installed on linux
makeloimage: makeloimage.c
	$(CC) $(CFLAGS) -lbsd -o makeloimage makeloimage.c

depend:
	cpanm --installdeps ..
	pkg-config --exists goptfoo
	@expect -c "package require Tcl 8.5; package require fileutil 1.13.0"

test: fdsplit findin getpof llcount seek twiddle
	@prove --nocolor

clean:
	@-rm -f corrupt fdsplit fillerup findin findup getpof llcount nomfd permcheck seek stripeofeol twiddle unatomappend *.core *.o 2>/dev/null

.PHONY: clean depend test
