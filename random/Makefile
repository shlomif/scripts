CFLAGS=-std=c99 -O3 -Wall -pedantic -fno-diagnostics-color -fstack-protector-all -fPIC -fPIE -pie -pipe

16: 16.c
coinflip: coinflip.c
fair-from-unfair: fair-from-unfair.c

modulo-bias: modulo-bias.c
	$(CC) $(CFLAGS) `pkg-config --cflags --libs goptfoo gsl` modulo-bias.c -o modulo-bias

oneof: oneof.c
randline: randline.c

randnum: randnum.c
	$(CC) $(CFLAGS) `pkg-config --cflags --libs goptfoo` randnum.c -o randnum

randrand: randrand.c
	$(CC) $(CFLAGS) `pkg-config --cflags --libs gsl` randrand.c -o randrand

roll: roll.c

test: coinflip oneof randline roll
	@prove --nocolor

clean:
	git clean --force -x

.PHONY: clean
