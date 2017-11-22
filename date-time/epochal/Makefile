CC?=gcc
CFLAGS?=-std=c99 -O2 -Wall -pedantic -fstack-protector-all -fPIE -pie -pipe

epochal: epochal.c

depend:
	cpanm --installdeps ../..

test: epochal
	@prove

clean:
	@-rm -f epochal *.core >/dev/null 2>&1
