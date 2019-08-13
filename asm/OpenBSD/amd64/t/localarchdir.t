#!perl
use lib qw(../../../lib/perl5);
use UtilityTestBelt;

my $cmd = Test::UnixCmdWrap->new;

my $release = qx(uname -r);
chomp $release;
my $expected = "OpenBSD$release-amd64\n";

$cmd->run(stdout => $expected);

# Test::Cmd does not document a way to close stdout
my $prog   = $cmd->prog;
my $output = qx($prog >&-);
exit_is($?, 1);
is($output, "");

done_testing(5);
