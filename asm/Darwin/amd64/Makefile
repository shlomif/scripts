#ASFLAGS=-g

# NOTE must use native {as,ld}; if some other compiler is first in PATH
# then the binary may need a writable data segment which these may lack

echo: echo.o
	/usr/bin/ld echo.o -o echo

echo.o: echo.s Makefile
	/usr/bin/as $(ASFLAGS) echo.s -o echo.o

false: false.o
	/usr/bin/ld -arch x86_64 -no_pie -macosx_version_min 10.6 false.o -o false

false.o: false.s Makefile
	/usr/bin/as $(ASFLAGS) -mmacosx-version-min=10.6 false.s -o false.o

hexip4todottedquad: hexip4todottedquad.o Makefile
	/usr/bin/ld hexip4todottedquad.o -o hexip4todottedquad

hexip4todottedquad.o: hexip4todottedquad.s Makefile
	/usr/bin/as $(ASFLAGS) hexip4todottedquad.s -o hexip4todottedquad.o

localarchdir: localarchdir.o
	/usr/bin/ld localarchdir.o -o localarchdir

localarchdir.o: localarchdir.asm
	nasm -f macho64 localarchdir.asm -o localarchdir.o

regpollute: regpollute.o
	/usr/bin/ld -arch x86_64 -no_pie -macosx_version_min 10.6 regpollute.o -o regpollute

regpollute.o: regpollute.s Makefile
	/usr/bin/as $(ASFLAGS) -mmacosx-version-min=10.6 regpollute.s -o regpollute.o

true: true.o
	/usr/bin/ld -arch x86_64 -no_pie -macosx_version_min 10.6 true.o -o true

true.o: true.s Makefile
	/usr/bin/as $(ASFLAGS) -mmacosx-version-min=10.6 true.s -o true.o

bench: echo false true
	@perl bench-echo
	@perl bench-truefalse

clean:
	git clean --force -x

test: false localarchdir regpollute true
	@prove --nocolor

.PHONY: bench test clean
