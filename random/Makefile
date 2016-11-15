#CFLAGS = -g
CC=gcc
CFLAGS += -O2 -std=c99 -Wall -pedantic -fstack-protector-all -fPIE -pie -pipe

CFLAGS += `pkg-config --cflags --libs goptfoo`

coinflip: coinflip.c

fair-from-unfair: fair-from-unfair.c

modulo-bias: modulo-bias.c

randnum: randnum.c

randrand: randrand.c

clean:
	@-rm -f coinflip fair-from-unfair modulo-bias randnum randrand *.core 2>/dev/null
