CFLAGS=-std=c99 -O3 -Wall -pedantic -fno-diagnostics-color -fstack-protector-all -fPIC -fPIE -pie -pipe

.SUFFIXES: .m4 .c
.m4.c:
	m4 -P ${.IMPSRC} > ${.TARGET}

todo: todo.c
todo.c: todo.m4

# cpanm assumes App::cpanminus is installed (and possibly local::lib too)
# only OpenBSD m4(1) has really been tested (portability is poor on m4)
depend:
	expect -c 'package require Tcl 8.5; package require fileutil 1.13.0'
	echo 'm4_m4exit(0)' | m4 -P
	perl -e 'use 5.16.0'
	cpanm --installdeps .

test: todo
	@prove --nocolor

clean:
	git clean --force -x

.PHONY: clean depend test
