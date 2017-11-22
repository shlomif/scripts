CC?=gcc
CFLAGS?=-std=c99 -O2 -Wall -pedantic -fstack-protector-all -fPIE -pie -pipe

# The _core bit is to hopefully get -levent_core as Centos 7 and OpenBSD
# lack a libevent_core.pc file to bring that in (and the libevent book
# indicates that -levent might go away in the future).
#
# For sd_notify() add `-DSYSTEMD -lsystemd` and ensure OS devel foo
buggyservd: buggyservd.c
	$(CC) $(CFLAGS) `pkg-config --cflags --libs libevent`_core -o buggyservd buggyservd.c

cts: cts.c

clean:
	@-rm buggyservd cts *.core 2>/dev/null
