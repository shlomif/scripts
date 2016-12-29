#!perl

use 5.14.0;
use warnings;
use Test::Cmd;
# 3 tests per item in @tests plus any extras
use Test::Most tests => 3 * 1 + 3;
use Test::UnixExit;

my $test_prog = 'copycat';

# since may not have pbcopy/X11 available, fake things
$ENV{CLIPBOARD} = 'cat';

my @tests = (
    {   stdin  => "foo $$\n",
        stdout => [ "foo $$", "foo $$" ],
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

    $testcmd->run( stdin => $test->{stdin} );

    exit_is( $?, $test->{exit_status}, "STATUS $test->{stdin} | $test_prog" );
    eq_or_diff( [ map { s/\s+$//r } split $/, $testcmd->stdout ],
        $test->{stdout}, "STDOUT $test->{stdin} | $test_prog" );
    is( $testcmd->stderr, $test->{stderr}, "STDERR $test->{stdin} | $test_prog" );
}

# any extras

$testcmd->run( args => '-h' );
exit_is( $?, 64, "EX_USAGE of sysexits(3) fame" );
ok( $testcmd->stderr =~ m/Usage/, "help mentions usage" );

ok( !-e "$test_prog.core", "$test_prog did not produce core" );
