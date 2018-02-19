localarchdir: localarchdir.o
	ld -m elf_x86_64_obsd -nopie localarchdir.o -o localarchdir

localarchdir.o: localarchdir.asm
	nasm -f elf64 localarchdir.asm -o localarchdir.o

clean:
	git clean --force -x

depend:
	cpanm --installdeps .

test: localarchdir
	@prove --nocolor

.PHONY: clean depend test
