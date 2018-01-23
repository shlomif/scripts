CFLAGS=-std=c99 -O2 -Wall -pedantic -fno-diagnostics-color -fstack-protector-all -fPIC -fPIE -pie -pipe

coinflip: coinflip.c

fair-from-unfair: fair-from-unfair.c

modulo-bias: modulo-bias.c

oneof: oneof.c

randline: randline.c

randnum: randnum.c
	$(CC) $(CFLAGS) `pkg-config --cflags --libs goptfoo` randnum.c -o randnum

randrand: randrand.c

test: coinflip oneof randline
	@prove --nocolor

clean:
	@-rm -f coinflip fair-from-unfair modulo-bias oneof randline randnum randrand *.core *.o 2>/dev/null

.PHONY: clean
