#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $test_prog = './autoformat';

my @tests = (
    {   stdin  => "1. a\n1. b\n",
        stdout => [ '1. a', '1. b' ],
    },
    {   args   => q{'all=>1'},
        stdin  => "1. a\n1. b\n",
        stdout => [ '1. a', '2. b' ],
    },
);
my $testcmd = Test::Cmd->new( prog => $test_prog, workdir => '', );

for my $test (@tests) {
    $test->{exit_status} //= 0;
    $test->{stderr}      //= '';

    $testcmd->run( exists $test->{args} ? ( args => $test->{args} ) : (),
        stdin => $test->{stdin} );

    $test->{args} //= '';

    exit_is( $?, $test->{exit_status}, "STATUS $test_prog $test->{args}" );
    eq_or_diff( [ map { s/\s+$//r } split $/, $testcmd->stdout ],
        $test->{stdout}, "STDOUT $test_prog $test->{args}" );
    is( $testcmd->stderr, $test->{stderr}, "STDERR $test_prog $test->{args}" );
}
done_testing( @tests * 3 );
