CFLAGS=-std=c99 -O3 -Wall -pedantic -fno-diagnostics-color -fstack-protector-all -fPIC -fPIE -pie -pipe

epochal: epochal.c

clean:
	git clean --force -x

depend:
	cpanm --installdeps ../..

test: epochal
	@prove --nocolor

.PHONY: clean depend test
