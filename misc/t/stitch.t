#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $test_prog = './stitch';

my @tests = (
    {   args   => '--ofs=\\\\t t/stitch-a:2 t/stitch-b:1,3 -',
        stdin  => "i1\ni2\n",
        stdout => [ "b\t1\t3\ti1", "\t4\t6\ti2", "\t7\t9" ],
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
