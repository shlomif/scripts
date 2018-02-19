CFLAGS=-std=c99 -O2 -Wall -pedantic -fno-diagnostics-color -fstack-protector-all -fPIC -fPIE -pie -pipe

# https://github.com/thrig/goptfoo

# for OpenBSD ports;
TCL=tcl85
# Mac OS X with MacPorts will instead need
#   make dnstw TCL=tcl
# or to create a link pkg-config can find
#   ln -s /opt/local/lib/pkgconfig/tcl.pc tcl85.pc 

dnstw: dnstw.c
	$(CC) $(CFLAGS) `pkg-config --cflags --libs goptfoo $(TCL)` dnstw.c -o dnstw

clean:
	git clean --force -x

depend:
	pkg-config --exists goptfoo
	@expect -c "package require Tcl 8.5"

.PHONY: clean depend
