CFLAGS=-std=c99 -O3 -Wall -pedantic -fno-diagnostics-color -fstack-protector-all -fPIC -fPIE -pie -pipe

tally: tally.c redblack-bst.h redblack-bst.o
	$(CC) $(CFLAGS) redblack-bst.o tally.c -o tally

redblack-bst.o: redblack-bst.c redblack-bst.h
	$(CC) $(CFLAGS) -c redblack-bst.c -o redblack-bst.o

test: tally
	@prove --nocolor

clean:
	git clean --force -x

.PHONY: clean test
