all:
	cvs2cl.pl -r --stdout pbotutil > pbotutil.pl.log
	cp pbotutil pbotutil.pl
	pod2html pbotutil > pbotutil.pl.html
	rsync -avz pbotutil.pl* sial.org:www/code/perl/scripts

test:
	date | ./pbotutil -s test -c '' | xargs ./pbotutil get
