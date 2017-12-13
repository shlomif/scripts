CFLAGS=-std=c99 -O2 -Wall -pedantic -fno-diagnostics-color -fstack-protector-all -fPIC -fPIE -pie -pipe

epochal: epochal.c

depend:
	cpanm --installdeps ../..

test: epochal
	@prove --nocolor

clean:
	@-rm -f epochal *.core >/dev/null 2>&1

.PHONY: clean depend test
