#CFLAGS=-g
CC=gcc
CFLAGS+=-O2 -std=c99 -Wall -pedantic -fstack-protector-all -fPIE -pie -pipe

nodupenv: nodupenv.c redblack-bst.h redblack-bst.o
	$(CC) $(CFLAGS) redblack-bst.o nodupenv.c -o nodupenv

redblack-bst.o: redblack-bst.c redblack-bst.h
	$(CC) $(CFLAGS) -c redblack-bst.c -o redblack-bst.o 

clean:
	git clean --force -x
