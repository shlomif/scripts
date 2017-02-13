#CFLAGS = -g
#ASMDEBUG = -g -F dwarf
CC=gcc
CFLAGS += -O2 -std=c99 -Wall -pedantic -fstack-protector-all -fPIE -pie -pipe

bt: bt.c
copycat: copycat.c
muss-with-environ: muss-with-environ.c
setsid: setsid.c
threading: threading.c

depend:
	cpanm --installdeps ..
	cpanm --installdeps .
	@expect -c "package require Tcl 8.5; package require fileutil 1.14.11"

register2hex32: register2hex32.o
	ld -m elf_i386 -o register2hex32 register2hex32.o

register2hex32.o: register2hex32.asm
	nasm -f elf32 $(ASMDEBUG) register2hex32.asm

test: bt copycat
	@prove

clean:
	@-rm -fr bt copycat muss-with-environ setsid threading *.core *.dSYM 2>/dev/null
