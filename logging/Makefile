#CFLAGS = -g
CC=gcc
CFLAGS += -O2 -std=c99 -Wall -pedantic -fstack-protector-all -fPIE -pie -pipe

buggyservd: buggyservd.c
	$(CC) $(CFLAGS) `pkg-config --cflags --libs libevent` -o buggyservd buggyservd.c
cts: cts.c

clean:
	@-rm buggyservd cts *.core 2>/dev/null
