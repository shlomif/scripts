all:
	cvs2cl -r --stdout pbotutil > pbotutil.pl.log
	cp pbotutil pbotutil.pl
	pod2html pbotutil > pbotutil.pl.html
	rsync -e ssh -avz pbotutil.pl* sial.org:www/code/perl/scripts
	rm pbotutil.pl* pod2htm*

test:
	date | ./pbotutil -s test -c '' | xargs ./pbotutil get
