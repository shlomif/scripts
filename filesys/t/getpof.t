#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;
use Cwd qw(getcwd);

my $test_prog = './getpof';

# from the Cwd(3pm) docs this should be compatible with the realpath(3)
# that getpof uses when directories are give...
my $test_dir = File::Spec->catfile( getcwd, 't' );

my @tests = (
    {   args   => 'getpof.t',
        stdout => ['./t'],
    },
    {   args   => 'getpof.t . t',
        stdout => [ $test_dir, $test_dir ],
    },
    {   args   => '-r recurse',
        stdout => [ './t', './t/recurse' ],
    },
);
my $testcmd = Test::Cmd->new( prog => $test_prog, workdir => '', );

for my $test (@tests) {
    $test->{exit_status} //= 0;
    $test->{stderr}      //= '';

    $testcmd->run( args => $test->{args} );

    exit_is( $?, $test->{exit_status}, "STATUS $test_prog $test->{args}" );
    eq_or_diff( [ map { s/\s+$//r } split $/, $testcmd->stdout ],
        $test->{stdout}, "STDOUT $test_prog $test->{args}" );
    is( $testcmd->stderr, $test->{stderr}, "STDERR $test_prog $test->{args}" );
}
$testcmd->run( args => '-h' );
exit_is( $?, 64, "EX_USAGE of sysexits(3) fame" );
ok( $testcmd->stderr =~ m/Usage/, "help mentions usage" );
done_testing( @tests * 3 + 2 );
