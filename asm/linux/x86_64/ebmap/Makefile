#DEBUGS = -g -F dwarf

ebmap_linux_amd64: ebmap_linux_amd64.o
	ld -o ebmap_linux_amd64 ebmap_linux_amd64.o
ebmap_linux_amd64.o: ebmap_linux_amd64.asm
	nasm -f elf64 $(DEBUGS) -o ebmap_linux_amd64.o ebmap_linux_amd64.asm

clean:
	@-rm -f ebmap ebmap_linux_amd64 2>/dev/null

realclean: clean
	@-rm -f *.o 2>/dev/null

.PHONY: clean realclean
