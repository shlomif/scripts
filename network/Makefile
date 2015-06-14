CFLAGS += `pkg-config --cflags --libs goptfoo`

portpester: portpester.c

randmac: randmac.c macutil.c macutil.h
	$(CC) $(CFLAGS) randmac.c macutil.c -o randmac

v4in6addr: v4in6addr.c

v6addr: v6addr.c

clean:
	@-rm -f portpester randmac
