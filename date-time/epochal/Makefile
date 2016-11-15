#CFLAGS = -g
CC=gcc
CFLAGS += -O2 -std=c99 -Wall -pedantic -fstack-protector-all -fPIE -pie -pipe

epochal: epochal.c

depend:
	cpanm --installdeps .

test: epochal
	prove t/*.t

clean:
	@-rm -f epochal *.o *.core >/dev/null 2>&1
