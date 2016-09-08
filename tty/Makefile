CFLAGS+=-std=c99
CFLAGS+=`pkg-config --cflags --libs goptfoo`

rdcomm: rdcomm.c
ttywrite: ttywrite.c
ttytest: ttytest.c

clean:
	@-rm -f rdcomm ttywrite *.core >/dev/null 2>&1
