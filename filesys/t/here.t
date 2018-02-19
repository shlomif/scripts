#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $test_prog = './here';

# NOTE these will fail should the repo be moved outside its usual
# checkout path under home
my @tests = (
    { stdout => '^' . File::Spec->catfile(qw/co scripts filesys/) . '$' },
    {   args   => 'subdir',
        stdout => '^' . File::Spec->catfile(qw/co scripts filesys subdir/) . '$'
    },
);
my $testcmd = Test::Cmd->new( prog => $test_prog, workdir => '', );

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
done_testing( @tests * 3 );
