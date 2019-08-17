#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

# TODO may need flag if noatime is set on filesystem

my $cmd = Test::UnixCmdWrap->new;

delete @ENV{ my @editor = qw(EDITOR VISUAL) };

my $test_dir = tempdir('uvi.XXXXXXXXXX', CLEANUP => 1, TMPDIR => 1);
my $subdir   = 'foo';

# the mtime + 1 (below in various places) is to test that atime and
# mtime are not mixed up
my $randtime = int rand 10000;

# this is for the "not a file" test so that it passes the mtime tests
my $sd = catdir($test_dir, $subdir);
mkdir $sd or BAIL_OUT("mkdir failed '$sd': $!");
utime($randtime, $randtime + 1, $sd) or BAIL_OUT("utime error dir $sd?? $!");

sub with_files {
    my ($files, $param) = @_;
    for my $f ($files->@*) {
        $f = catfile($test_dir, $f);
        next if -d $f;
        open my $fh, '>', $f or BAIL_OUT("could not write $f: $!");
    }
    utime($randtime, $randtime + 1, $files->@*) or BAIL_OUT("utime error?? $!");
    $param->{env} = { VISUAL => 'touch' } unless exists $param->{env};
    $cmd->run($param->%*, args => join ' ', $files->@*);
    for my $f ($files->@*) {
        my ($atime, $mtime) = (stat $f)[ 8, 9 ];
        is $atime, $randtime;
        is $mtime, $randtime + 1;
    }
}

with_files(["no.$$"]);
with_files([qw(pa re ci)]);

# nope on directory edits
with_files([$subdir], { status => 74, stderr => qr/not a file/ });

# EDITOR should be equivalent to VISUAL
with_files(['vo'], { env => { EDITOR => 'touch' } });

$cmd->run(status => 64, stderr => qr/^Usage: /);

done_testing(27);
