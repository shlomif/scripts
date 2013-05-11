obdurate:
	$(CC) $(CFLAGS) -o obdurate obdurate.c

sigint-off:
	$(CC) $(CFLAGS) -o sigint-off sigint-off.c

sigint-on:
	$(CC) $(CFLAGS) -o sigint-on sigint-on.c

clean:
	@-rm obdurate sigint-off sigint-on >/dev/null 2>&1
