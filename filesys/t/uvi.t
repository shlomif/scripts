#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $test_prog = './uvi';
my $test_dir  = tempdir('uvi.XXXXXXXXXX', CLEANUP => 1, TMPDIR => 1);

delete @ENV{qw(EDITOR VISUAL)};
$ENV{EDITOR} = 'touch';

my $subdir = "nope";

my @tests = (
    { files => ["no.$$"] },
    { files => [qw(pa re ci)] },
    # nope on directory edits
    {   files       => [$subdir],
        exit_status => 1,
        stderr      => qr/^/,
    },
    {   exit_status => 64,
        stderr      => qr/^Usage: /
    }
);
my $cmd = Test::Cmd->new(prog => $test_prog, workdir => '',);

# the mtime + 1 (below in various places) is to test that atime and
# mtime are not reversed by uvi(1)
my $randtime = int rand 10000;

# this is for the "not a file" test so that it passes the mtime tests
my $sd = File::Spec->catfile($test_dir, $subdir);
mkdir $sd;
utime $randtime, $randtime + 1, $sd;

for my $test (@tests) {
    $test->{exit_status} //= 0;
    $test->{stderr}      //= qr/^$/;
    $test->{stdout}      //= qr/^$/;

    my $args;
    for my $f (@{ $test->{files} }) {
        $f = File::Spec->catfile($test_dir, $f);
        if (!-d $f) {
            open my $fh, '>', $f or die "could not write $f: $!\n";
            utime($randtime, $randtime + 1, $f) > 0 or die "utime error?? $!";
        }
        $args .= $f . ' ';
    }
    $cmd->run(defined $args ? (args => $args) : ());

    $args //= '';
    exit_is($?, $test->{exit_status}, "STATUS $test_prog $args");
    ok($cmd->stdout =~ $test->{stdout}, "STDOUT $test_prog $args")
      or diag "STDOUT >" . $cmd->stdout . "<";
    ok($cmd->stderr =~ $test->{stderr}, "STDERR $test_prog $args")
      or diag "STDERR >" . $cmd->stderr . "<";

    for my $f (@{ $test->{files} }) {
        my ($atime, $mtime) = (stat $f)[ 8, 9 ];
        is $atime, $randtime;
        is $mtime, $randtime + 1;
    }
}

done_testing;
