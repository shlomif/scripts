dist:
	cvs2cl -r --stdout wcdns > wcdns.pl.log
	cp wcdns wcdns.pl
	pod2html wcdns > wcdns.pl.html
	rsync -e ssh -avz wcdns.pl* sial.org:www/code/perl/scripts
	rm wcdns.pl* pod2htm*
