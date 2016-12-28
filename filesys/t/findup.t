#!perl

use 5.14.0;
use warnings;
use Cwd qw(getcwd);
use File::Basename qw(dirname);
use File::Spec ();
use Test::Cmd;
# 3 tests per item in @tests plus any extras
use Test::Most tests => 3 * 7 + 2;
use Test::UnixExit;

my $test_prog = 'findup';

my $prog_dir  = getcwd;
my $first_dir = ( File::Spec->splitpath($prog_dir) )[1];

my @tests = (
    {   args   => 'findup',
        stdout => [$prog_dir],
    },
    {   args   => '-q -f findup',
        stdout => [],
    },
    {   args   => 'filesys',
        stdout => [ dirname($prog_dir) ],
    },
    {   args   => '-q -d filesys',
        stdout => [],
    },
    # NOTE low but non-zero odds of false positive
    {   args        => random_filename(),
        stdout      => [],
        exit_status => 1,
    },
    # special case
    {   args   => '/',
        stdout => ['/'],
    },
    # NOTE makes assumptions about directory path being under $HOME, as
    # if not -H won't stop the upward search anywhere
    {   args        => "-H '$first_dir'",
        stdout      => [],
        exit_status => 1,
    },
);
my $testcmd = Test::Cmd->new(
    prog    => $test_prog,
    verbose => 0,
    workdir => '',
);

for my $test (@tests) {
    $test->{exit_status} //= 0;
    $test->{stderr}      //= '';

    $testcmd->run( args => $test->{args} );

    exit_is( $?, $test->{exit_status}, "STATUS $test_prog $test->{args}" );
    eq_or_diff( [ map { s/\s+$//r } split $/, $testcmd->stdout ],
        $test->{stdout}, "STDOUT $test_prog $test->{args}" );
    is( $testcmd->stderr, $test->{stderr}, "STDERR $test_prog $test->{args}" );
}

# any extras

$testcmd->run( args => '-h' );
exit_is( $?, 64, "EX_USAGE of sysexits(3) fame" );
ok( $testcmd->stderr =~ m/Usage/, "help mentions usage" );

sub random_filename {
    my @allowed = ( 'A' .. 'Z', 'a' .. 'z', 0 .. 9, '_' );
    join '', map { $allowed[ rand @allowed ] } 1 .. 32;
}
