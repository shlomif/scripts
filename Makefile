depend:
	cpanm --installdeps .
	@echo also install expect, tcllib, gsl dev, etc.

test:
	@prove
