all: showip udp-sender udp-sink

showip: showip.c
	$(CC) $(CFLAGS) -o showip showip.c

udp-sender: udp-sender.c getopts.c udpfling.h
	$(CC) $(CFLAGS) -o udp-sender getopts.c udp-sender.c

udp-sink: udp-sink.c getopts.c udpfling.h
	$(CC) $(CFLAGS) -o udp-sink getopts.c udp-sink.c

clean:
	@-rm -f showip udp-sender udp-sink 2>/dev/null
