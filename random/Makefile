CFLAGS=-std=c99 -O3 -Wall -pedantic -fno-diagnostics-color -fstack-protector-all -fPIC -fPIE -pie -pipe

16: 16.c

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
	git clean --force -x

.PHONY: clean
