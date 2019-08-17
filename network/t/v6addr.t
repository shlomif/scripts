#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $cmd = Test::UnixCmdWrap->new;

$cmd->run(
    args   => '::1',
    stdout => ["0000:0000:0000:0000:0000:0000:0000:0001"],
);
$cmd->run(
    args => '-r 2001:db8::c000:22a',
    stdout =>
      ["a.2.2.0.0.0.0.c.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.8.b.d.0.1.0.0.2.ip6.arpa."],
);
$cmd->run(
    args   => '-rR ::1',
    stdout => ["1.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0"],
);
$cmd->run(
    args   => '::x',
    status => 65,
    stderr => qr/could not parse/,
);
$cmd->run(
    args   => '-q 2001::2010::2040',
    status => 65
);
$cmd->run(args => '-h', status => 64, stderr => qr/Usage/);
$cmd->run(status => 64, stderr => qr/Usage/);

done_testing(21);
