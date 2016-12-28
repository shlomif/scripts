#!perl

use 5.14.0;
use warnings;
use Test::Cmd;
# 3 tests per item in @tests plus any extras
use Test::Most tests => 3 * 3 + 2;
use Test::UnixExit;

my $test_prog = 'cfu';

my @tests = (
    {   args   => qq{'puts("PID $$")'},
        stdout => ["PID $$"],
    },
    {   args   => "-E $$ " . quotemeta 'printf("PID %s\n", *++argv)',
        stdout => ["PID $$"],
    },
    {   args   => qq{-g 'const char *pid="PID"' 'printf("%s $$", pid)'},
        stdout => ["PID $$"],
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
        exists $test->{chdir} ? ( chdir => $test->{chdir} ) : ()
    );

    exit_is( $?, $test->{exit_status}, "STATUS $test_prog $test->{args}" );
    # NOTE sort as cannot assume what order the files will be in
    eq_or_diff( [ sort map { s/\s+$//r } split $/, $testcmd->stdout ],
        $test->{stdout}, "STDOUT $test_prog $test->{args}" );
    is( $testcmd->stderr, $test->{stderr}, "STDERR $test_prog $test->{args}" );
}

# any extras

$testcmd->run( args => '-h' );
exit_is( $?, 64, "EX_USAGE of sysexits(3) fame" );
ok( $testcmd->stderr =~ m/Usage/, "help mentions usage" );
