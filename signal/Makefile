#CFLAGS = -g
CC=gcc
CFLAGS += -O2 -std=c99 -Wall -pedantic -fstack-protector-all -fPIE -pie -pipe

blocksig: blocksig.c

jitterbits: jitterbits.c

obdurate: obdurate.c

sigint-off: sigint-off.c

sigint-on: sigint-on.c

slay-self: slay-self.c

clean:
	@-rm -f blocksig jitterbits sigint-off sigint-on sigratetest slay-self *.core 2>/dev/null
