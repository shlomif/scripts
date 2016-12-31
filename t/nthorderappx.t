#!perl

use 5.14.0;
use warnings;
use Test::Cmd;
# 3 tests per item in @tests plus any extras
use Test::Most tests => 3 * 1 + 2;
use Test::UnixExit;

my $test_prog = 'nthorderappx';

my @tests = (
    {   args  => '--lines=1',
        stdin => 'the quick brown fox jumped over the lazy dog',
        # TODO locking srand in nthorderappx would make it more testable
        stdout => qr/^[ywajqnctorzelhbdigvux kmpf]+$/,
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
    ok( $testcmd->stdout =~ m/$test->{stdout}/, "STDOUT $test_prog $test->{args}" );
    is( $testcmd->stderr, $test->{stderr}, "STDERR $test_prog $test->{args}" );
}

# any extras

$testcmd->run( args => '-h' );
exit_is( $?, 64, "EX_USAGE of sysexits(3) fame" );
ok( $testcmd->stderr =~ m/Usage/, "help mentions usage" );
