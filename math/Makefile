#CFLAGS = -g
CC=gcc
CFLAGS += -O2 -std=c99 -Wall -pedantic -fstack-protector-all -fPIE -pie -pipe

CFLAGS += `pkg-config --cflags --libs gsl goptfoo`

chii: chii.c

gaussiate: gaussiate.c

clean:
	@-rm -f gaussiate *.core >/dev/null 2>&1
