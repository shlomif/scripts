CFLAGS=-std=c99 -Wall -pedantic -fno-diagnostics-color -fstack-protector-all -fPIC -fPIE -pie -pipe

lockalot: lockalot.c
	$(CC) $(CFLAGS) `pkg-config --cflags --libs goptfoo` lockalot.c -o lockalot

malloc: malloc.c
	$(CC) -g $(CFLAGS) `pkg-config --cflags --libs goptfoo` -g malloc.c -o malloc

repcharcount: repcharcount.c
	$(CC) -O2 $(CFLAGS) repcharcount.c -o repcharcount

usemem: usemem.c
	$(CC) $(CFLAGS) -g `pkg-config --cflags --libs goptfoo jkiss` -g usemem.c -o usemem

depend:
	cpanm --installdeps ..
	pkg-config --exists goptfoo
	pkg-config --exists jkiss
	@expect -c "package require Tcl 8.5"

test: repcharcount
	@prove --nocolor

clean:
	@-rm -f lockalot malloc repcharcount usemem *.core *.o 2>/dev/null

.PHONY: clean depend test
