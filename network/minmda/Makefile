CC?=gcc
CFLAGS?=-std=c99 -O2 -Wall -pedantic -fno-diagnostics-color -fstack-protector-all -fPIE -pie -pipe

minmda: minmda.c minmda.h files.o Makefile
	$(CC) $(CFLAGS) `pkg-config --cflags --libs libsodium` files.o minmda.c -o minmda

minmda-corrupts: minmda.c minmda.h files.o Makefile
	$(CC) $(CFLAGS) -DMINMDA_ALWAYS_CORRUPTS=1 `pkg-config --cflags --libs libsodium` files.o minmda.c -o minmda-corrupts

files.o: files.c
	$(CC) $(CFLAGS) -c files.c -o files.o

# this does not attempt the install as that will vary between platform
depend:
	@pkg-config --cflags --libs libsodium

test-depend:
	cpanm --installdeps .

test: minmda minmda-corrupts
	@prove --nocolor
	@-rm minmda-corrupts 2>/dev/null

clean:
	git clean --force -x
