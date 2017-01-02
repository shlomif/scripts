depend:
	cpanm --installdeps .
	@expect -c "package require Tcl 8.5; package require fileutil 1.14.11"
	@echo also install gsl dev, goptfoo, etc.

test:
	@prove
