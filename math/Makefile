CFLAGS=-std=c99 -O3 -Wall -pedantic -fno-diagnostics-color -fstack-protector-all -fPIC -fPIE -pie -pipe

chii: chii.c
	$(CC) $(CFLAGS) `pkg-config --cflags --libs gsl goptfoo` chii.c -o chii

gaussiate: gaussiate.c
	$(CC) $(CFLAGS) `pkg-config --cflags --libs gsl goptfoo` gaussiate.c -o gaussiate

clean:
	git clean --force -x

depend:
	pkg-config --exists goptfoo
	pkg-config --exists gsl

.PHONY: clean depend
