CC?=gcc
CFLAGS?=-std=c99 -O2 -Wall -pedantic -fno-diagnostics-color -fstack-protector-all -fPIE -pie -pipe

CFLAGS += `pkg-config --cflags --libs gsl goptfoo`

chii: chii.c

gaussiate: gaussiate.c

clean:
	@-rm -f gaussiate *.core >/dev/null 2>&1
