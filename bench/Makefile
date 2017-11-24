CC?=gcc
CFLAGS?=-std=c99 -Wall -pedantic -fno-diagnostics-color -fstack-protector-all -fPIE -pie -pipe

lockalot: lockalot.c
	$(CC) $(CFLAGS) `pkg-config --cflags --libs goptfoo` -o malloc malloc.c

malloc: malloc.c
	$(CC) -g $(CFLAGS) `pkg-config --cflags --libs goptfoo` -g -lpthread -o malloc malloc.c

repcharcount: repcharcount.c
	$(CC) -O2 $(CFLAGS) repcharcount.c -o repcharcount

usemem: usemem.c
	$(CC) -g $(CFLAGS) `pkg-config --cflags --libs goptfoo jkiss` -g -lpthread -o usemem usemem.c

depend:
	cpanm --installdeps ..
	@expect -c "package require Tcl 8.5"

test: repcharcount
	@prove

clean:
	@-rm -f lockalot malloc repcharcount usemem *.core 2>/dev/null
