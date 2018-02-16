depend:
	cpanm --installdeps .
	expect -c "package require Tcl 8.5; package require fileutil 1.13.0"

clean:
	-make -C bench clean
	-make -C date-time clean
	-make -C filesys clean
	-make -C logging clean
	-make -C math clean
	-make -C misc clean
	-make -C music clean
	-make -C network clean
	-make -C password clean
	-make -C random clean
	-make -C signal clean
	-make -C tty clean

.PHONY: clean depend
