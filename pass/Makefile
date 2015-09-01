mkpwhash: mkpwhash.c
	gcc -g -std=gnu99 -lsodium -lcrypt -Werror -Wall -Wextra -Wundef -ftrapv -fstack-protector-all -pedantic -pipe -o mkpwhash mkpwhash.c

clean:
	@-rm mkpwhash 2>/dev/null
