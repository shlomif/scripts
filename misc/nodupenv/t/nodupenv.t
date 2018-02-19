#!perl
# NOTE if dupenv is buggy then things here will be sad, too
use 5.14.0;
use warnings;
use Test::Cmd;
use Test::Most tests => 12;
use Test::UnixExit;

# for eq_or_diff; see "DIFF STYLES" in perldoc Test::Differences
unified_diff;

my $test_prog = './nodupenv';
my $env_prog  = './dupenv';

my $testcmd = Test::Cmd->new(
    prog    => $test_prog,
    verbose => 0,
    workdir => '',
);

# exec chain time
$testcmd->run( args => "'$env_prog' -i $test_prog '$env_prog'" );
is( $testcmd->stdout, "", "no stdout because -i clears env" );
is( $testcmd->stderr, "", "no stderr on do-nothing test" );
exit_is( $?, 0, "zero exit on do-nothing test" );

$testcmd->run( args => "'$env_prog' -i FOO=bar '$test_prog' '$env_prog'" );
is( $testcmd->stdout, "FOO=bar\n" );
is( $testcmd->stderr, "" );
exit_is( $?, 0, "zero exit on do-nothing test" );

# env(1) by contrast (at least the version I have) will pass instead zot
#   $ env -i FOO=bar FOO=zot env
#   FOO=zot
$testcmd->run(
    args => "'$env_prog' -i FOO=bar FOO=zot '$test_prog' '$env_prog'" );
is( $testcmd->stdout, "FOO=bar\n" );
is( $testcmd->stderr, "" );
exit_is( $?, 0, "zero exit on do-nothing test" );

# no args is help (nodupenv has no command line flags)
$testcmd->run();
exit_is( $?, 64, "EX_USAGE of sysexits(3) fame" );
is( $testcmd->stdout, "", "no stdout on help" );
ok( $testcmd->stderr =~ m/Usage/, "help mentions usage" );
