CFLAGS += `pkg-config --cflags --libs goptfoo`

all: showip udp-sender udp-sink

showip: showip.c

getopts.o: getopts.c
	$(CC) $(CFLAGS) -c getopts.c -o getopts.o

udp-sender: udp-sender.c getopts.o udpfling.h
	$(CC) $(CFLAGS) -o udp-sender getopts.o udp-sender.c

udp-sink: udp-sink.c getopts.o udpfling.h
	$(CC) $(CFLAGS) -o udp-sink getopts.o udp-sink.c

clean:
	@-rm -f showip udp-sender udp-sink *.o *.core 2>/dev/null
