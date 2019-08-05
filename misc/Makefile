CFLAGS=-std=c99 -O3 -Wall -pedantic -fno-diagnostics-color -fstack-protector-all -fPIC -fPIE -pie -pipe

bt: bt.c
copycat: copycat.c
justblocks: justblocks.c
muss-with-environ: muss-with-environ.c

pcre-precompile: pcre-precompile.c Makefile
	$(CC) $(CFLAGS) `pkg-config --cflags --libs libpcre` pcre-precompile.c -o pcre-precompile

procrust: procrust.c Makefile
	$(CC) $(CFLAGS) `pkg-config --cflags --libs goptfoo` procrust.c -o procrust

setsid: setsid.c
sgrax: sgrax.c
solitary: solitary.c

threading: threading.c
	$(CC) $(CFLAGS) -lpthread threading.c -o threading

waitornot: waitornot.c

clean:
	git clean --force -x

depend:
	cpanm --installdeps ..
	cpanm --installdeps .
	@expect -c "package require Tcl 8.5; package require fileutil 1.13.0"

test: bt copycat justblocks procrust sgrax waitornot unwrap
	make -C t
	@prove --nocolor

.PHONY: clean depend test
