#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $cmd  = Test::UnixCmdWrap->new;
my $tcmd = $cmd->cmd;

$cmd->run(args => "xom", stdout => ["xom"]);

my %seen;
# NOTE may false alarm if RNG streaks
for (1 .. 30) {
    $tcmd->run(args => "a b c");
    chomp(my $out = $tcmd->stdout);
    $seen{$out}++;
}
eq_or_diff([ sort keys %seen ], [qw/a b c/]);

$cmd->run(status => 64,   stderr => qr/^Usage: /);
$cmd->run(args   => '-h', status => 64, stderr => qr/^Usage: /);

done_testing(10);
