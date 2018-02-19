#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;
use Time::HiRes qw(gettimeofday tv_interval);

my $test_prog = './timeout';

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
my $testcmd = Test::Cmd->new( prog => $test_prog, workdir => '', );

for my $test (@tests) {
    $test->{exit_status} //= 0;
    $test->{stderr}      //= qr/^$/;
    $test->{stdout}      //= [];

    my $start = [gettimeofday];
    $testcmd->run( args => $test->{args} );
    my $elapsed_error =
      abs( tv_interval($start) - $test->{duration} ) / $test->{duration};

    exit_is( $?, $test->{exit_status}, "STATUS $test_prog $test->{args}" );
    eq_or_diff( [ map { s/\s+$//r } split $/, $testcmd->stdout ],
        $test->{stdout}, "STDOUT $test_prog $test->{args}" );
    ok( $testcmd->stderr =~ m/$test->{stderr}/,
        "STDERR $test_prog $test->{args}: " . $testcmd->stderr );

    ok( $elapsed_error < $tolerance,
        "duration variance out of bounds: $elapsed_error" );
}
$testcmd->run( args => '-h' );
exit_is( $?, 64, "EX_USAGE of sysexits(3) fame" );
ok( $testcmd->stderr =~ m/Usage/, "help mentions usage" );
done_testing( @tests * 4 + 2 );
