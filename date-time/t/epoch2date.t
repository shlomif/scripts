#!perl

use 5.14.0;
use warnings;
use Test::Cmd;
# 3 tests per item in @tests plus any extras
use Test::Most tests => 3 * 3 + 2;
use Test::UnixExit;

my $test_prog = './epoch2date';

$ENV{TZ} = 'UTC';

my @tests = (
    {   args   => '-',
        stdin  => '1059673440 1404433529',
        stdout => [ '2003-07-31 17:44:00 UTC', '2014-07-04 00:25:29 UTC' ],

    },
    {   args   => '1475682078014325',
        stdout => ['2016-10-05 15:41:18 UTC'],
        stderr => "notice: guess microseconds for 1475682078014325\n",
    },
    {   args   => '-f "%Y" 1483228801',     # 2017 in UTC
        env    => { TZ => 'US/Pacific' },
        stdout => ['2016'],
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

    local @ENV{ keys %{ $test->{env} } } = values %{ $test->{env} };

    $testcmd->run(
        args => $test->{args},
        exists $test->{stdin} ? ( stdin => $test->{stdin} ) : ()
    );

    exit_is( $?, $test->{exit_status}, "STATUS $test_prog $test->{args}" );
    eq_or_diff( [ map { s/\s+$//r } split $/, $testcmd->stdout ],
        $test->{stdout}, "STDOUT $test_prog $test->{args}" );
    is( $testcmd->stderr, $test->{stderr}, "STDERR $test_prog $test->{args}" );
}

# any extras

$testcmd->run( args => '-h' );
exit_is( $?, 64, "EX_USAGE of sysexits(3) fame" );
ok( $testcmd->stderr =~ m/Usage/, "help mentions usage" );
