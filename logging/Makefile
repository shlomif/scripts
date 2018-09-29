CFLAGS=-std=c99 -O3 -Wall -pedantic -fno-diagnostics-color -fstack-protector-all -fPIC -fPIE -pie -pipe

# The _core bit is to hopefully get -levent_core as Centos 7 and OpenBSD
# lack a libevent_core.pc file to bring that in (and the libevent book
# indicates that -levent might go away in the future).
#
# For sd_notify() add `-DSYSTEMD -lsystemd` and ensure OS devel foo
buggyservd: buggyservd.c
	$(CC) $(CFLAGS) `pkg-config --cflags --libs libevent`_core -o buggyservd buggyservd.c

cts: cts.c
logsoc: logsoc.c

clean:
	git clean --force -x

depend:
	cpanm --installdeps ..
	expect -c "package require Tcl 8.5"

.PHONY: clean depend
