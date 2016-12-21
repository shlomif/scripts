#!perl

use 5.14.0;
use warnings;
use Test::Cmd;
# 3 tests per item in @tests plus any extras
use Test::Most tests => 3 * 3 + 3;

my $test_prog = 'llcount';
my $test_file = 't/llcount-input';

open my $tfh, '<', $test_file or die "could not open '$test_file': $!\n";
my $test_lines = do { local $/; readline $tfh };

my @tests = (
    {   args   => $test_file,
        stdout => [
            '  1     0   16 The quick brown',
            '  2    16    4 fox',
            '  3    20    7 jumped'
        ],
    },
    {   args   => "-x $test_file",
        stdout => [
            '  1     0   16 The quick brown',
            '  2  0x10    4 fox',
            '  3  0x14    7 jumped'
        ],
    },
    {   args   => '-',
        stdin  => $test_lines,
        stdout => [
            '  1     0   16 The quick brown',
            '  2    16    4 fox',
            '  3    20    7 jumped'
        ],
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

    $testcmd->run(
        args => $test->{args},
        exists $test->{stdin} ? ( stdin => $test->{stdin} ) : ()
    );

    is( $? >> 8, $test->{exit_status}, "STATUS $test_prog $test->{args}" );
    eq_or_diff( [ map { s/\s+$//r } split $/, $testcmd->stdout ],
        $test->{stdout}, "STDOUT $test_prog $test->{args}" );
    is( $testcmd->stderr, $test->{stderr}, "STDERR $test_prog $test->{args}" );
}

# any extras

$testcmd->run( args => '-h' );
is( $? >> 8, 64, "EX_USAGE of sysexits(3) fame" );
ok( $testcmd->stderr =~ m/Usage/, "help mentions usage" );

ok( !-e "$test_prog.core", "$test_prog did not produce core" );
