depend:
	cpanm --installdeps .
	expect -c "package require Tcl 8.5; package require fileutil 1.13.0"

.PHONY: depend
