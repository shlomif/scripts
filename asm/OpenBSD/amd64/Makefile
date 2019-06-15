# TODO figure out what -m flag clang ld wants
localarchdir: localarchdir.o
	ld.bfd -m elf_x86_64_obsd -nopie localarchdir.o -o localarchdir

localarchdir.o: localarchdir.asm
	nasm -f elf64 localarchdir.asm -o localarchdir.o

clean:
	git clean --force -x

depend:
	cpanm --installdeps .

test: localarchdir
	@prove --nocolor

.PHONY: clean depend test
