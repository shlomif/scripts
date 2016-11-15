#CFLAGS = -g
CC=gcc
CFLAGS += -O2 -std=c99 -Wall -pedantic -fstack-protector-all -fPIE -pie -pipe

CFLAGS += `pkg-config --cflags --libs goptfoo`

fillerup: fillerup.c

getpof: getpof.c

#large-file-support: large-file-support.c

llcount: llcount.c

nomfd: nomfd.c

permcheck: permcheck.c

stripeofeol: stripeofeol.c

unatomappend: unatomappend.c

clean:
	@-rm -f fillerup getpof large-file-support llcount nomfd permcheck stripeofeol unatomappend *.core 2>/dev/null
