localarchdir: localarchdir.o
	ld -m elf_x86_64_obsd -nopie localarchdir.o -o localarchdir

localarchdir.o: localarchdir.asm
	nasm -f elf64 localarchdir.asm -o localarchdir.o

depend:
	cpanm --installdeps .

test: localarchdir
	@prove --nocolor

clean:
	@-rm -f *.o *.core localarchdir 2>/dev/null

.PHONY: clean depend test
