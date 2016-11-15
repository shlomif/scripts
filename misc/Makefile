#CFLAGS = -g
CC=gcc
CFLAGS += -O2 -std=c99 -Wall -pedantic -fstack-protector-all -fPIE -pie -pipe

copycat: copycat.c
muss-with-environ: muss-with-environ.c
setsid: setsid.c
threading: threading.c

clean:
	@-rm -f copycat muss-with-environ setsid threading *.core 2>/dev/null
