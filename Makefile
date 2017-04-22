#CFLAGS = -g
CC=gcc
CFLAGS += -O2 -std=c99 -Wall -pedantic -fstack-protector-all -fPIE -pie -pipe

depend:
	cpanm --installdeps .
	@expect -c "package require Tcl 8.5; package require fileutil 1.13.0"
	@echo also install gsl dev, goptfoo, etc.

sitelen-sin: sitelen-sin.c

test:
	@prove

clean:
	@-rm -f sitelen-sin 2>/dev/null
