CFLAGS += -std=c99
CFLAGS += `pkg-config --cflags --libs goptfoo jkiss`

# (debug -g flag to help avoid compiler optimizing away memory foo being done)
CFLAGS += -g

# for some of the progs
CFLAGS += -lpthread

lockalot: lockalot.c
malloc: malloc.c
usemem: usemem.c

clean:
	@-rm -f lockalot malloc usemem *.core 2>/dev/null
