# (debug -g flag to help avoid compiler optimizing away memory foo being done)
CFLAGS = -g
CFLAGS += -std=c99 -Wall -pedantic -fstack-protector-all -fPIE -pie -pipe

CFLAGS += `pkg-config --cflags --libs goptfoo jkiss`

# for some of the progs
CFLAGS += -lpthread

lockalot: lockalot.c
malloc: malloc.c
usemem: usemem.c

depend:
	cpanm --installdeps ..
	@expect -c "package require Tcl 8.5"

test:
	@prove

clean:
	@-rm -f lockalot malloc usemem *.core 2>/dev/null
