gaussiate: ../montecarlo/aaarghs.c gaussiate.c
	$(CC) $(CFLAGS) `pkg-config --cflags --libs gsl` -o gaussiate ../montecarlo/aaarghs.c gaussiate.c

clean:
	@-rm -f gaussiate *.core >/dev/null 2>&1
