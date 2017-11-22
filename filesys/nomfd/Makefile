CC?=gcc
CFLAGS?=-std=c99 -O2 -Wall -pedantic -fstack-protector-all -fPIE -pie -pipe

CFLAGS += `pkg-config --cflags --libs goptfoo`

nomfd: nomfd.c

clean:
	@-rm -f nomfd *.core 2>/dev/null
