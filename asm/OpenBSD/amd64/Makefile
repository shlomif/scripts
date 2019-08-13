.SUFFIXES: .m4 .asm
.m4.asm:
	m4 ${.IMPSRC} > ${.TARGET}

# TODO figure out what -m flag clang ld wants (this is new in OpenBSD
# 6.5 or so)
localarchdir: localarchdir.o
	ld.bfd -m elf_x86_64_obsd -nopie localarchdir.o -o localarchdir

localarchdir.o: localarchdir.asm
	nasm -f elf64 localarchdir.asm -o localarchdir.o

localarchdir.asm: localarchdir.m4 ../../../lib/m4/cf.m4

clean:
	git clean --force -x

test: localarchdir
	@prove --nocolor

.PHONY: clean test
