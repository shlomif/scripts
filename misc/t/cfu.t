#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $test_prog = './cfu';

# NOTE these must be quoted for the shell Test::Cmd runs things through
my @tests = (
    {   args   => qq{'puts("PID $$")'},
        stdout => ["PID $$"],
    },
    {   args => qq{-E '$$ "a b" 42' }
          . quotemeta 'printf("%s,%s,%s\n",*(argv+1),*(argv+2),*(argv+3))',
        stdout => ["$$,a b,42"],
    },
    # globals must now be quoted as that way you can also write functions
    {   args   => qq{-G 'const char *pid="PID";' 'printf("%s $$", pid)'},
        stdout => ["PID $$"],
    },
);
my $testcmd = Test::Cmd->new( prog => $test_prog, workdir => '', );

for my $test (@tests) {
    $test->{exit_status} //= 0;
    $test->{stderr}      //= '';

    $testcmd->run(
        args => $test->{args},
        exists $test->{chdir} ? ( chdir => $test->{chdir} ) : ()
    );

    exit_is( $?, $test->{exit_status}, "STATUS $test_prog $test->{args}" );
    eq_or_diff( [ map { s/\s+$//r } split $/, $testcmd->stdout ],
        $test->{stdout}, "STDOUT $test_prog $test->{args}" );
    is( $testcmd->stderr, $test->{stderr}, "STDERR $test_prog $test->{args}" );
}
$testcmd->run( args => '-h' );
exit_is( $?, 64, "EX_USAGE of sysexits(3) fame" );
ok( $testcmd->stderr =~ m/Usage/, "help mentions usage" );

# this output will vary wildly by platform but hopefully the string
# should appear exactly somewhere
$testcmd->run( args => qq{-S 'puts("grepfor$$")'} );
exit_is( $?, 0 );
ok( $testcmd->stdout, "grepfor$$" );
is( $testcmd->stderr, "" );
done_testing( @tests * 3 + 5 );
