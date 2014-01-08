findin: findin.o getenv.o
	$(CC) $(CFLAGS) -o findin findin.o getenv.o
findin.o: findin.c findin.h
	$(CC) $(CFLAGS) -c findin.c
getenv.o: getenv.c findin.h
	$(CC) $(CFLAGS) -c getenv.c

clean:
	@-rm -f findin *.o *.core
# for Mac OS X debug info cleanup
	@-rm -rf *.dSYM
