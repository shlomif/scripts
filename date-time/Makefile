CFLAGS=-std=c99 -O3 -Wall -pedantic -fno-diagnostics-color -fstack-protector-all -fPIC -fPIE -pie -pipe

c-delta-time: c-delta-time.c
date2epoch: date2epoch.c
myat: myat.c
mycal: mycal.c
now: now.c
timeout: timeout.c

clean:
	git clean --force -x

depend:
	cpanm --installdeps ..

test: date2epoch now timeout
	@prove --nocolor

.PHONY: clean depend test
