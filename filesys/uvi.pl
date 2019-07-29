#!/usr/bin/env perl
#
# uvi - edit files without changing the access times

if (!@ARGV) {
    print STDERR "Usage: uvi file [file2 ..]\n";
    exit 64;
}

foreach (@ARGV) {
    my @utime = ((stat)[ 8, 9 ], $_);
    if (not -f _) {
        warn "not a file: '$_'\n";
        exit 1;
    }
    push @restore, \@utime;
}

$ret = 0;
system($ENV{VISUAL} // $ENV{EDITOR} // 'vi', @ARGV) == 0 or $ret = 1;

foreach (@restore) {
    unless (utime @$_) {
        warn "could not restore ", $_->[-1], " to orig times: $!\n";
        $ret = 1;
    }
}
exit $ret;
