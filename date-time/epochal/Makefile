epochal: epochal.c

# requires App::Prove, Test::More, Test::Cmd perl modules installed
test: epochal
	@-rm epochal.core 2>/dev/null
	@prove t/*.t

clean:
	@-rm -f epochal *.o *.core
