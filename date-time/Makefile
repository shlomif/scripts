CC?=gcc
CFLAGS?=-std=c99 -O2 -Wall -pedantic -fno-diagnostics-color -fstack-protector-all -fPIE -pie -pipe

c-delta-time: c-delta-time.c
date2epoch: date2epoch.c
myat: myat.c
mycal: mycal.c
now: now.c
timeout: timeout.c

depend:
	cpanm --installdeps ..

test: date2epoch now timeout
	@prove --nocolor

clean:
	@-rm c-delta-time date2epoch myat mycal now timeout 2>/dev/null
