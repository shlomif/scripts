#!perl

use 5.14.0;
use warnings;
use Test::Cmd;
# 3 tests per item in @tests plus any extras
use Test::Most tests => 3 * 1 + 2;
use Test::UnixExit;

my $test_prog = './stitch';

my @tests = (
    {   args   => '--ofs=\\\\t t/stitch-a:2 t/stitch-b:1,3 -',
        stdin  => "i1\ni2\n",
        stdout => [ "b\t1\t3\ti1", "\t4\t6\ti2", "\t7\t9" ],
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
