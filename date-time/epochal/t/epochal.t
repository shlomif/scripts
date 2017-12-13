#!perl

use 5.14.0;
use warnings;
use POSIX qw(strftime);
use Test::Cmd;
# 3 tests per item in @tests plus any extras
use Test::Most tests => 3 * 4 + 2;
use Test::UnixExit;

my $test_prog = './epochal';

$ENV{TZ} = 'UTC';

my $utc_year = strftime( "%Y", gmtime );

my @tests = (
    {   args => q{-f '%b %d %H:%M:%S' -Y 2016 t/messages},
        stdout =>
          [ '1481827952 host Dec 15 18:52:32', '1481827952 host', '1481849429' ],
    },
    {   args   => q{-f '%b %d %H:%M:%S' -Y 2016 -g t/messages},
        stdout => [ '1481827952 host 1481827952', '1481827952 host', '1481849429' ],
    },
    {   args   => q{-f '%H:%M:%S' -o '%H:%M' -},
        env    => { TZ => 'US/Pacific' },
        stdin  => '18:40:22',
        stdout => ['18:40'],
    },
    {   args   => q{-f '%b %d %H:%M:%S' -o '%Y' -s -y t/messages},
        stdout => [ $utc_year, $utc_year, $utc_year ],
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
