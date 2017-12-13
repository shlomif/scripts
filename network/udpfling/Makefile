CFLAGS=-std=c99 -O2 -Wall -pedantic -fno-diagnostics-color -fstack-protector-all -fPIC -fPIE -pie -pipe

all: showip udp-sender udp-sink

showip: showip.c

getopts.o: getopts.c
	$(CC) -c $(CFLAGS) `pkg-config --cflags goptfoo` getopts.c -o getopts.o

udp-sender.o: udp-sender.c udpfling.h
	$(CC) -c $(CFLAGS) udp-sender.c -o udp-sender.o

udp-sender: udp-sender.o getopts.o
	$(CC) $(CFLAGS) `pkg-config --cflags --libs goptfoo` udp-sender.o getopts.o -o udp-sender

udp-sink.o: udp-sink.c udpfling.h
	$(CC) -c $(CFLAGS) udp-sink.c -o udp-sink

udp-sink: udp-sink.o getopts.o
	$(CC) $(CFLAGS) `pkg-config --cflags --libs goptfoo` udp-sink.o getopts.o -o udp-sink

clean:
	@-rm -f showip udp-sender udp-sink *.o *.core 2>/dev/null

.PHONY: all clean
