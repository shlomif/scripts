#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $cmd = Test::UnixCmdWrap->new;

$cmd->run(
    args   => "''",
    status => 64,
    stderr => qr/empty string/,
);
$cmd->run(
    args   => "bar",
    status => 64,
    stderr => qr/number/,
);
$cmd->run(
    args   => "'0'",
    status => 64,
    stderr => qr/below minimum/,
);
$cmd->run(
    args   => "9999999",
    status => 64,
    stderr => qr/above maximum/,
);
$cmd->run(
    args   => "1s20",
    status => 64,
    stderr => qr/unknown char/,
);
$cmd->run(
    args   => "1d",
    status => 64,
    stderr => qr/stray char/,
);
$cmd->run(
    args   => "1d1",
    status => 64,
    stderr => qr/below minimum/,
);
# proper statistical tests should also be done...
$cmd->run(
    args   => "1d2",
    stdout => qr/^[12]$/,
    stderr => qr/^$/,
);
$cmd->run(args => '-h', status => 64, stderr => qr/^Usage: /);
$cmd->run(status => 64, stderr => qr/^Usage: /);

done_testing(30);
