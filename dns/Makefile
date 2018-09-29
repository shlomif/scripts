CFLAGS=-std=c99 -O3 -Wall -pedantic -fno-diagnostics-color -fstack-protector-all -fPIC -fPIE -pie -pipe

dns-server-version: dns-server-version.c

depend:
	cpanm --installdeps .

clean:
	git clean --force -x

.PHONY: clean depend
