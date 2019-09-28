#!perl
#
# NOTE if dupenv is buggy then things here will be sad, too

use lib qw(../../lib/perl5);
use UtilityTestBelt;

# for eq_or_diff; see "DIFF STYLES" in perldoc Test::Differences
unified_diff;

my $cmd       = Test::UnixCmdWrap->new;
my $test_prog = './nodupenv';
my $env_prog  = './dupenv';

# exec chain time (does nothing (besides burn CPU))
$cmd->run(args => "'$env_prog' -i $test_prog '$env_prog'");

$cmd->run(
    args   => "'$env_prog' -i FOO=bar '$test_prog' '$env_prog'",
    stdout => ["FOO=bar"],
);

# env(1) by contrast (at least the version I have) will pass instead zot
#   $ env -i FOO=bar FOO=zot env
#   FOO=zot
$cmd->run(
    args   => "'$env_prog' -i FOO=bar FOO=zot '$test_prog' '$env_prog'",
    stdout => ["FOO=bar"]
);

# no args is help (nodupenv has no command line flags)
$cmd->run(status => 64, stderr => qr/Usage/);

done_testing(12);
