#!perl

use 5.14.0;
use warnings;
use Test::Cmd;
# 3 tests per item in @tests plus any extras
use Test::Most tests => 3 * 2 + 0;
use Test::UnixExit;

my $test_prog = './maxof';
my @inputs    = qw(t/maxof-a t/maxof-b);

my @tests = (
    {   stdin  => "3.14 pi\n2.72 e\n99 bottles of beer\n42 answers\n",
        stdout => ['99 bottles of beer'],
    },
    {   args   => "@inputs",
        stdout => ['99 bottles of beer'],
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

    $testcmd->run(
        exists $test->{args}  ? ( args  => $test->{args} )  : (),
        exists $test->{stdin} ? ( stdin => $test->{stdin} ) : ()
    );

    $test->{args} //= '';
    exit_is( $?, $test->{exit_status}, "STATUS $test_prog $test->{args}" );
    eq_or_diff( [ map { s/\s+$//r } split $/, $testcmd->stdout ],
        $test->{stdout}, "STDOUT $test_prog $test->{args}" );
    is( $testcmd->stderr, $test->{stderr}, "STDERR $test_prog $test->{args}" );
}

# any extras
