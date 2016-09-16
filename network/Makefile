CFLAGS += -std=c99
CFLAGS += `pkg-config --cflags --libs goptfoo`

endian: endian.c

getsockname: getsockname.c

ip6tomac: ip6tomac.c

ipvalid: ipvalid.c

macutil: macutil.c

portpester: portpester.c

randmac: randmac.c macutil.c macutil.h
	$(CC) $(CFLAGS) randmac.c macutil.c -o randmac

v6addr: v6addr.c

v4in6addr: v4in6addr.c


clean:
	@-rm -f endian getsockname ip6tomac ipvalid macutil portpester randmac v4addr v4in6addr v6addr *.core 2>/dev/null
