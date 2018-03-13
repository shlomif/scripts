CFLAGS=-std=c99 -O2 -Wall -pedantic -fno-diagnostics-color -fstack-protector-all -fPIC -fPIE -pie -pipe

endian: endian.c
getsockname: getsockname.c
ip6tomac: ip6tomac.c
ipvalid: ipvalid.c

macnorm: macnorm.c
	$(CC) $(CFLAGS) `pkg-config --cflags --libs goptfoo` macnorm.c -o macnorm

portpester: portpester.c

macutil.o: macutil.c macutil.h
	$(CC) -c $(CFLAGS) macutil.c -o macutil.o
randmac: randmac.c macutil.o macutil.h
	$(CC) $(CFLAGS) `pkg-config --cflags --libs goptfoo` macutil.o randmac.c -o randmac

v4addr: v4addr.c

v4in6addr: v4in6addr.c
	$(CC) $(CFLAGS) `pkg-config --cflags --libs goptfoo` v4in6addr.c -o v4in6addr

v6addr: v6addr.c

validate-rsnapshot: validate-rsnapshot.c
	$(CC) $(CFLAGS) `pkg-config --cflags --libs libpcre` validate-rsnapshot.c -o validate-rsnapshot

clean:
	git clean --force -x

depend:
	cpanm --installdeps ..
	cpanm --installdeps .
	pkg-config --exists goptfoo
	@expect -c "package require Tcl 8.5;"

test: macnorm v4addr v6addr
	@prove --nocolor

.PHONY: clean depend test
