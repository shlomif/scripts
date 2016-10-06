CFLAGS+=-std=c99
CFLAGS+=`pkg-config --cflags --libs goptfoo`

# for OpenBSD ports; Mac OS X with MacPorts may instead need
#   make rdcomm TCL=tcl
TCL=tcl85

rdcomm: rdcomm.c
	$(CC) $(CFLAGS) `pkg-config --cflags --libs $(TCL)` -o rdcomm rdcomm.c
torc: torc.c
ttywrite: ttywrite.c
ttytest: ttytest.c

clean:
	@-rm -f rdcomm ttywrite *.core >/dev/null 2>&1
