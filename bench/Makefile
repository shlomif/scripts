CFLAGS += `pkg-config --cflags --libs goptfoo`

# (debug -g flag to help avoid compiler optimizing away memory foo being done)
CFLAGS += -g

# for some of the progs
CFLAGS += -lpthread

insertornot: insertornot.c

lockalot: lockalot.c

malloc: malloc.c

usemem: usemem.c

clean:
	@-rm -f insertornot lockalot malloc usemem
