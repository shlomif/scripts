#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $test_prog = './sumtime';

my @tests = (
    {   args   => '300',
        stdout => ['5m'],
    },
    {   args   => '1w 3d 2h 6m 5s',
        stdout => ['1w 3d 2h 6m 5s'],
    },
    {   args   => '-c -',
        stdin  => "1s 20m\n30m 40m\n",
        stdout => ['1h30m1s'],
    },
);
my $testcmd = Test::Cmd->new( prog => $test_prog, workdir => '', );

for my $test (@tests) {
    $test->{exit_status} //= 0;
    $test->{stderr}      //= '';

    $testcmd->run(
        args => $test->{args},
        exists $test->{stdin} ? ( stdin => $test->{stdin} ) : ()
    );

    exit_is( $?, $test->{exit_status}, "STATUS $test_prog $test->{args}" );
    eq_or_diff( [ map { s/\s+$//r } split $/, $testcmd->stdout ],
        $test->{stdout}, "STDOUT $test_prog $test->{args}" );
    is( $testcmd->stderr, $test->{stderr}, "STDERR $test_prog $test->{args}" );
}
$testcmd->run( args => '-h' );
exit_is( $?, 64, "EX_USAGE of sysexits(3) fame" );
ok( $testcmd->stderr =~ m/Usage/, "help mentions usage" );
done_testing( @tests * 3 + 2 );
