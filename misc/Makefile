#CFLAGS = -g
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

register2hex32: register2hex32.asm
	nasm -f bin -o register2hex32 register2hex32.asm
	chmod +x register2hex32

# NOTE portability tricky wrt libevent, may need to fiddle with the flags
# or use -levent_core depending on the platform and available *.pc files.
waitornot: waitornot.c
	$(CC) $(CFLAGS) `pkg-config --cflags --libs libevent` -o waitornot waitornot.c

test: bt copycat
	@prove

clean:
	@-rm -fr bt copycat muss-with-environ register2hex32 setsid threading waitornot *.core *.dSYM 2>/dev/null
