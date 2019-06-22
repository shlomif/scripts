#DEBUGS = -g -F dwarf

ebmap_linux_amd64: ebmap_linux_amd64.o
	ld -o ebmap_linux_amd64 ebmap_linux_amd64.o
ebmap_linux_amd64.o: ebmap_linux_amd64.asm
	nasm -f elf64 $(DEBUGS) -o ebmap_linux_amd64.o ebmap_linux_amd64.asm

clean:
	git clean --force -x

.PHONY: clean
