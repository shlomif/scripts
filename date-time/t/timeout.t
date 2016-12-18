#!perl

use 5.14.0;
use warnings;
use Test::Cmd;
# 4 tests per item in @tests plus any extras
use Test::Most tests => 4 * 3 + 3;
use Time::HiRes qw(gettimeofday tv_interval);

my $test_prog = 'timeout';

# TODO see discussion in snooze.t
my $tolerance = 0.15;

diag "timeout tests will take time...";

my @tests = (
    {   args     => '7 sleep 3',
        duration => 3,
    },
    {   args        => '3 sleep 7',
        stderr      => qr/^timeout: duration 3 exceeded/,
        exit_status => 2,
        duration    => 3,
    },
    {   args     => '3m sleep 2',
        duration => 2,
    },
);
my $testcmd = Test::Cmd->new(
    prog    => $test_prog,
    verbose => 0,
    workdir => '',
);

for my $test (@tests) {
    $test->{exit_status} //= 0;
    $test->{stderr}      //= qr/^$/;
    $test->{stdout}      //= [];

    my $start = [gettimeofday];
    $testcmd->run( args => $test->{args} );
    my $elapsed_error =
      abs( tv_interval($start) - $test->{duration} ) / $test->{duration};

    is( $? >> 8, $test->{exit_status}, "STATUS $test_prog $test->{args}" );
    eq_or_diff( [ map { s/\s+$//r } split $/, $testcmd->stdout ],
        $test->{stdout}, "STDOUT $test_prog $test->{args}" );
    ok( $testcmd->stderr =~ m/$test->{stderr}/,
        "STDERR $test_prog $test->{args}: " . $testcmd->stderr );

    ok( $elapsed_error < $tolerance,
        "duration variance out of bounds: $elapsed_error" );
}

# any extras

$testcmd->run( args => '-h' );
is( $? >> 8, 64, "EX_USAGE of sysexits(3) fame" );
ok( $testcmd->stderr =~ m/Usage/, "help mentions usage" );

ok( !-e "$test_prog.core", "$test_prog did not produce core" );
