bigpass: bigpass.c
	$(CC) $(CFLAGS) `pkg-config --cflags --libs libsodium` -o bigpass bigpass.c

mkpwhash: mkpwhash.c salt.o
	gcc -std=gnu99 `pkg-config --cflags --libs libsodium` -lcrypt -Werror -Wall -Wextra -Wundef -ftrapv -fstack-protector-all -pedantic -pipe -o mkpwhash mkpwhash.c salt.o

salt.o: salt.c
	$(CC) -std=c99 $(CFLAGS) -c `pkg-config --cflags --libs libsodium` -o salt.o salt.c

testsalt: testsalt.c salt.o
	$(CC) -std=c99 $(CFLAGS) `pkg-config --cflags --libs libsodium` -Werror -Wall -Wextra -Wundef -ftrapv -fstack-protector-all -pedantic -pipe -o testsalt testsalt.c salt.o

clean:
	@-rm bigpass mkpwhash testsalt 2>/dev/null

realclean: clean
	@-rm salt.o 2>/dev/null
