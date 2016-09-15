CFLAGS += -std=c99
CFLAGS += `pkg-config --cflags --libs gsl goptfoo`

chii: chii.c

gaussiate: gaussiate.c

clean:
	@-rm -f gaussiate *.core >/dev/null 2>&1
