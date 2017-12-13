CFLAGS=-std=c99 -O2 -Wall -pedantic -fno-diagnostics-color -fstack-protector-all -fPIC -fPIE -pie -pipe

# for OpenBSD ports; Mac OS X with MacPorts will instead need
#   make rdcomm TCL=tcl
# or to create a link pkg-config can find
#   ln -s /opt/local/lib/pkgconfig/tcl.pc tcl85.pc 
TCL=tcl85

nocolor: nocolor.c
	$(CC) $(CFLAGS) `pkg-config --cflags --libs libpcre` -lpthread -o nocolor nocolor.c

rdcomm: rdcomm.c
	$(CC) $(CFLAGS) `pkg-config --cflags --libs $(TCL) goptfoo` rdcomm.c -o rdcomm

torc: torc.c

ttytest: ttytest.c

ttywrite: ttywrite.c
	$(CC) $(CFLAGS) `pkg-config --cflags --libs goptfoo` ttywrite.c -o ttywrite

test: nocolor
	@prove --nocolor

depend:
	cpanm --installdeps ..
	cpanm --installdeps .
	pkg-config --exists goptfoo
	pkg-config --exists libpcre
	@expect -c "package require Tcl 8.5; package require fileutil 1.13.0"

clean:
	@-rm -f nocolor rdcomm torc ttytest ttywrite *.core *.o >/dev/null 2>&1

.PHONY: clean depend
