CFLAGS=-std=c99 -O2 -Wall -pedantic -fno-diagnostics-color -fstack-protector-all -fPIC -fPIE -pie -pipe

# https://github.com/thrig/goptfoo

# for OpenBSD ports;
TCL=tcl85
# Mac OS X with MacPorts will instead need
#   make dnstw TCL=tcl
# or to create a link pkg-config can find
#   ln -s /opt/local/lib/pkgconfig/tcl.pc tcl85.pc 

dnstw: dnstw.c dnstw.h ipparse.o
	$(CC) $(CFLAGS) `pkg-config --cflags --libs goptfoo $(TCL)` dnstw.c ipparse.o -o dnstw

ipparse: dnstw.h ipparse.c
	$(CC) -c $(CFLAGS) `pkg-config --cflags --libs $(TCL)` ipparse.c -o ipparse.o

clean:
	git clean --force -x

depend:
	pkg-config --exists goptfoo $(TCL)
	@expect -c "package require Tcl 8.5"

test: dnstw
	@prove --nocolor

.PHONY: clean depend test
