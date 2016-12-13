#CFLAGS = -g
CC=gcc
CFLAGS += -O2 -std=c99 -Wall -pedantic -fstack-protector-all -fPIE -pie -pipe

c-delta-time: c-delta-time.c
date2epoch: date2epoch.c
myat: myat.c
mycal: mycal.c
now: now.c
timeout: timeout.c

depend:
	cpanm --installdeps ..

test: date2epoch
	@prove

clean:
	@-rm c-delta-time date2epoch myat mycal now timeout 2>/dev/null
