#!perl

use 5.14.0;
use warnings;
use POSIX qw(strftime);
use Test::Cmd;
# 3 tests per item in @tests plus any extras
use Test::Most tests => 3 * 4 + 3;

my $test_prog = 'now';

$ENV{TZ} = 'US/Pacific';

my $cur_epoch  = time();
# NOTE tests will fail if the day rolls over somewhere between here and
# the last date-related ->run, below.
my $three_days = strftime( "%b %d", localtime( $cur_epoch + 3 * 86400 ) );
my $four_weeks = strftime( "%b %d", localtime( $cur_epoch + 4 * 7 * 86400 ) );

my @tests = (
    {   args   => '3',
        stdout => [$three_days],
    },
    {   args   => '+3',
        stdout => [$three_days],
    },
    {   args   => '+3d',
        stdout => [$three_days],
    },
    {   args   => '+4w',
        stdout => [$four_weeks],
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
