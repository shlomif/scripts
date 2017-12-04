CC?=gcc
CFLAGS?=-std=c99 -O2 -Wall -pedantic -fno-diagnostics-color -fstack-protector-all -fPIE -pie -pipe

bt: bt.c
copycat: copycat.c
muss-with-environ: muss-with-environ.c
setsid: setsid.c
sgrax: sgrax.c
threading: threading.c

depend:
	cpanm --installdeps ..
	cpanm --installdeps .
	@expect -c "package require Tcl 8.5; package require fileutil 1.13.0"

register2hex32: register2hex32.asm
	nasm -f bin -o register2hex32 register2hex32.asm
	chmod +x register2hex32

waitornot: waitornot.c

test: bt copycat sgrax waitornot
	@prove --nocolor

clean:
	@-rm -fr bt copycat muss-with-environ register2hex32 setsid threading waitornot *.core *.dSYM 2>/dev/null
