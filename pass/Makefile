bigpass: bigpass.c
	$(CC) $(CFLAGS) `pkg-config --cflags --libs libsodium` -o bigpass bigpass.c

mkpwhash: mkpwhash.c
	gcc -std=gnu99 `pkg-config --cflags --libs libsodium` -lcrypt -Werror -Wall -Wextra -Wundef -ftrapv -fstack-protector-all -pedantic -pipe -o mkpwhash mkpwhash.c

clean:
	@-rm mkpwhash 2>/dev/null
