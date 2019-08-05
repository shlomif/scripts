# this assumes App::cpanminus is installed (and possibly local::lib too)
depend:
	perl -e 'use 5.16.0'
	cpanm --installdeps .
	expect -c "package require Tcl 8.5; package require fileutil 1.13.0"

.SUFFIXES: .m4 .c
.m4.c:
	m4 -P ${.IMPSRC} > ${.TARGET}

todo: todo.c
todo.c: todo.m4

clean:
	git clean --force -x

.PHONY: clean depend
