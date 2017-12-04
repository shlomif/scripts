#!perl

use 5.14.0;
use warnings;
use File::Spec ();
use Test::Cmd;
# 3 tests per item in @tests
use Test::Most tests => 3 * 2;
use Test::UnixExit;

my $test_prog = './here';

# NOTE these will fail should the repo be moved outside its usual
# checkout path under home
my @tests = (
    { stdout => '^' . File::Spec->catfile(qw/co scripts filesys/) . '$' },
    {   args   => 'subdir',
        stdout => '^' . File::Spec->catfile(qw/co scripts filesys subdir/) . '$'
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

    $testcmd->run( exists $test->{args} ? ( args => $test->{args} ) : () );

    $test->{args} //= '';
    exit_is( $?, $test->{exit_status}, "STATUS $test_prog $test->{args}" );
    ok( $testcmd->stdout =~ m/$test->{stdout}/,
        "STDOUT $test_prog $test->{args}: " . $testcmd->stdout );
    is( $testcmd->stderr, $test->{stderr}, "STDERR $test_prog $test->{args}" );
}
