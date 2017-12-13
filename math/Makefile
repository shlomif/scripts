CFLAGS=-std=c99 -O2 -Wall -pedantic -fno-diagnostics-color -fstack-protector-all -fPIC -fPIE -pie -pipe

chii: chii.c
	$(CC) $(CFLAGS) `pkg-config --cflags --libs gsl goptfoo` chii.c -o chii

gaussiate: gaussiate.c
	$(CC) $(CFLAGS) `pkg-config --cflags --libs gsl goptfoo` gaussiate.c -o gaussiate

depend:
	pkg-config --exists goptfoo
	pkg-config --exists gsl

clean:
	@-rm -f chii gaussiate *.core *.o >/dev/null 2>&1

.PHONY: clean
