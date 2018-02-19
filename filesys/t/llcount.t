#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $test_prog = './llcount';
my $test_file = 't/llcount-input';

open my $tfh, '<', $test_file or die "could not open '$test_file': $!\n";
my $test_lines = do { local $/; readline $tfh };

my @tests = (
    {   args   => "'$test_file'",
        stdout => [
            '  1     0   16 The quick brown',
            '  2    16    4 fox',
            '  3    20    7 jumped'
        ],
    },
    {   args   => "-x '$test_file'",
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
