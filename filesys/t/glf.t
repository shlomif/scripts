#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $cmd = Test::UnixCmdWrap->new;

my $test_epoch = 915148800;
my $test_dir   = tempdir("glf-t.XXXXXXXXX", CLEANUP => 1, TMPDIR => 1);

my %test_files = (test => 0, newer => 300, older => -900);

while (my ($file, $epmod) = each %test_files) {
    $file = catfile($test_dir, $file);
    open my $fh, '>', $file, or die "could not create $file: $!\n";
    $epmod += $test_epoch;
    utime $epmod, $epmod, $file;
}

$cmd->run(
    chdir  => $test_dir,
    stdout => ['newer'],
);
$cmd->run(
    args   => q{-e '^new'},
    chdir  => $test_dir,
    stdout => ['test'],
);
$cmd->run(
    args   => q{'^old'},
    chdir  => $test_dir,
    stdout => ['older'],
);
$cmd->run(
    args   => "'^' '$test_dir'",
    stdout => ['newer'],
);
$cmd->run(args => '-h', status => 64, stderr => qr/Usage/);

done_testing(15);
