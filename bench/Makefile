# (debug -g flag to help avoid compiler optimizing away memory foo being done)

insertornot: insertornot.c
	$(CC) -std=c99 $(CFLAGS) insertornot.c -o insertornot

lockalot: lockalot.c ../montecarlo/aaarghs.c
	$(CC) -std=c99 $(CFLAGS) -g lockalot.c ../montecarlo/aaarghs.c -o lockalot

malloc: malloc.c
	$(CC) -g -std=c99 $(CFLAGS) -g -lpthread malloc.c -o malloc

usemem: usemem.c
	$(CC) -g -std=c99 $(CFLAGS) -g -lpthread usemem.c ../montecarlo/aaarghs.c -o usemem

clean:
	@-rm -f insertornot lockalot malloc usemem
