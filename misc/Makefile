#CFLAGS = -g
CC=gcc
CFLAGS += -O2 -std=c99 -Wall -pedantic -fstack-protector-all -fPIE -pie -pipe

bt: bt.c
copycat: copycat.c
muss-with-environ: muss-with-environ.c
setsid: setsid.c
threading: threading.c

depend:
	cpanm --installdeps ..

test: bt copycat
	@prove

clean:
	@-rm -fr bt copycat muss-with-environ setsid threading *.core *.dSYM 2>/dev/null
