CFLAGS=-std=c99 -Wall -pedantic -fno-diagnostics-color -fstack-protector-all -fPIC -fPIE -pie -pipe

lockalot: lockalot.c
	$(CC) $(CFLAGS) `pkg-config --cflags --libs goptfoo` lockalot.c -o lockalot

malloc: malloc.c
	$(CC) $(CFLAGS) -g `pkg-config --cflags --libs goptfoo` -lpthread malloc.c -o malloc

repcharcount: repcharcount.c
	$(CC) -O2 $(CFLAGS) repcharcount.c -o repcharcount

usemem: usemem.c
	$(CC) $(CFLAGS) -g `pkg-config --cflags --libs goptfoo jkiss` -g usemem.c -o usemem

clean:
	git clean --force -x

depend:
	cpanm --installdeps ..
	pkg-config --exists goptfoo
	pkg-config --exists jkiss
	@expect -c "package require Tcl 8.5"

test: repcharcount
	@prove --nocolor

.PHONY: clean depend test
