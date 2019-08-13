#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $cmd = Test::UnixCmdWrap->new;

my $few = 3 + int rand 3;

$cmd->run(
    stderr => qr/^Usage/,
    status => 64,
);
$cmd->run(
    args   => 'tribbles',
    stderr => qr/^Usage/,
    status => 64,
);
$cmd->run(
    args   => '42',
    stderr => qr/^Usage/,
    status => 64,
);
$cmd->run(
    args   => "$few echo tribbles",
    stdout => qr/^(?s)(?:tribbles.){$few}/,
);
# $N is made available to the command with the current count number
$cmd->run(
    args   => "$few sh -c 'echo tribble \$N'",
    stdout => qr/^(?sa)(?:tribble \d+.){$few}/,
);
done_testing(15);
