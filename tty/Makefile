CFLAGS=-std=c99 -O3 -Wall -pedantic -fno-diagnostics-color -fstack-protector-all -fPIC -fPIE -pie -pipe

# for OpenBSD ports; Mac OS X with MacPorts will instead need
#   make rdcomm TCL=tcl
# or to create a link pkg-config can find
#   ln -s /opt/local/lib/pkgconfig/tcl.pc tcl86.pc 
TCL=tcl85

gunktty: gunktty.c
	$(CC) $(CFLAGS) `pkg-config --cflags --libs goptfoo` gunktty.c -o gunktty

nocolor: nocolor.c
	$(CC) $(CFLAGS) `pkg-config --cflags --libs libpcre` -lpthread -o nocolor nocolor.c

nocolor-test: nocolor.c Makefile
	$(CC) -DNOCOLOR_BUF_SIZE=32 -g $(CFLAGS) `pkg-config --cflags --libs libpcre` -lpthread -o nocolor-test nocolor.c

prte: prte.c

rdcomm: rdcomm.c
	$(CC) $(CFLAGS) `pkg-config --cflags --libs $(TCL) goptfoo` rdcomm.c -o rdcomm

torc: torc.c

ttytest: ttytest.c

ttywrite: ttywrite.c
	$(CC) $(CFLAGS) `pkg-config --cflags --libs goptfoo` ttywrite.c -o ttywrite

clean:
	git clean --force -x

depend:
	cpanm --installdeps ..
	cpanm --installdeps .
	pkg-config --exists libpcre goptfoo $(TCL)
	@expect -c "package require Tcl 8.5; package require fileutil 1.13.0"

test: nocolor-test
	@prove --nocolor

.PHONY: clean depend test
