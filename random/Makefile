CC?=gcc
CFLAGS?=-std=c99 -O2 -Wall -pedantic -fno-diagnostics-color -fstack-protector-all -fPIE -pie -pipe

CFLAGS += `pkg-config --cflags --libs goptfoo`

coinflip: coinflip.c

fair-from-unfair: fair-from-unfair.c

modulo-bias: modulo-bias.c

randline: randline.c

randnum: randnum.c

randrand: randrand.c

clean:
	@-rm -f coinflip fair-from-unfair modulo-bias randline randnum randrand *.core 2>/dev/null
