#!perl

use 5.14.0;
use warnings;
use Test::Cmd;
# 3 tests per item in @tests plus any extras
use Test::Most tests => 3 * 3 + 3;
use Test::UnixExit;

my $test_prog = './date2epoch';

$ENV{TZ} = 'UTC';

my @tests = (
    {   args   => '2003-07-31 17:44:00',
        stdout => ['1059673440'],
    },
    {   args   => '2014 15',
        stdout => ['1388588400'],
    },
    {   args   => '1970-04-26 10:46:39',
        env    => { TZ => 'US/Pacific' },
        stdout => ['9999999'],
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

ok( !-e "$test_prog.core", "$test_prog did not produce core" );
