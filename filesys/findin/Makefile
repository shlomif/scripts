#CFLAGS = -g
CC=gcc
CFLAGS += -O2 -std=c99 -Wall -pedantic -fstack-protector-all -fPIE -pie -pipe

findin: findin.o getenv.o
	$(CC) $(CFLAGS) -o findin findin.o getenv.o
findin.o: findin.c findin.h
	$(CC) $(CFLAGS) -c findin.c
getenv.o: getenv.c findin.h
	$(CC) $(CFLAGS) -c getenv.c

clean:
	@-rm -f findin *.o *.core
