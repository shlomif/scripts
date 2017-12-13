#!perl

use 5.14.0;
use warnings;
use Test::Cmd;
use Test::Most tests => 19;
use Test::UnixExit;
use Time::HiRes qw(gettimeofday tv_interval);

# environment sanitization and ease of testing
delete @ENV{qw(OUTPUT_PREFIX TIMEOUT)};
$ENV{CLIPBOARD} = 'cat';

# see TODOs elsewhere (in snooze.t in particular)
my $tolerance = 0.15;

my $test_prog = './forexample';

my $testcmd = Test::Cmd->new(
    prog    => $test_prog,
    verbose => 0,
    workdir => '',
);

# no args, help
$testcmd->run();
ok( $testcmd->stderr =~ m/Usage: / );
is( $testcmd->stdout, "" );
exit_is( $?, 64, "EX_USAGE no args" );

# what does a command-not-found failure look like? (while making wild
# assumptions concerning the not-existence of askdjfksdjfkds...)
$testcmd->run( args => "askdjfksdjfkds" );
is( $testcmd->stdout, "" );
ok( $testcmd->stderr =~ m/forexample/ );
exit_is( $?, 1 );

# ditto for bad CLIPBOARD command
{
    local $ENV{CLIPBOARD} = "askdjfksdjfkds";

    $testcmd->run( args => "echo hi" );
    is( $testcmd->stdout, "" );
    ok( $testcmd->stderr =~ m/forexample/ );
    exit_is( $?, 1 );
}

# can we at least echo/cat something?
$testcmd->run( args => "echo '$$'" );

# KLUGE strip out any CR from PTY
my $stdout = $testcmd->stdout =~ tr/\r//dr;

is( $stdout,          "    \$ echo $$\n    $$\n    \$ \n" );
is( $testcmd->stderr, "" );
exit_is( $?, 0 );

{
    local $ENV{OUTPUT_PREFIX} = "";

    $testcmd->run( args => "echo '$$'" );
    $stdout = $testcmd->stdout =~ tr/\r//dr;
    is( $stdout,          "\$ echo $$\n$$\n\$ \n" );
    is( $testcmd->stderr, "" );
    exit_is( $?, 0 );
}

# this one should timeout before completion
{
    local $ENV{TIMEOUT} = 0.5;

    my $start = [gettimeofday];
    $testcmd->run( args => "sleep 7" );

    is( $testcmd->stdout, "    \$ sleep 7\n    ...\n" );
    is( $testcmd->stderr, "" );
    exit_is( $?, 0 );

    ok( tv_interval($start) < 3 );
}
