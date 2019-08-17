#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $cmd = Test::UnixCmdWrap->new;

$cmd->run(
    args   => '127.0.0.1',
    stdout => ["127.0.0.1"],
);
$cmd->run(
    args   => '-f 10.11.12.13',
    stdout => ["10.11.12.13"],
);
$cmd->run(
    args   => '-r 192.0.2.42',
    stdout => ["42.2.0.192.in-addr.arpa."],
);
$cmd->run(
    args   => '-a 192.0.2.43',
    stdout => [ "192.0.2.43", "43.2.0.192.in-addr.arpa." ],
);
$cmd->run(
    args   => '127.1',
    stderr => qr/could not parse/,
    status => 65,
);
$cmd->run(
    args   => '-q 127.0x0.1',
    status => 65,
);
$cmd->run(args => '-h', status => 64, stderr => qr/Usage/);
$cmd->run(status => 64, stderr => qr/Usage/);

done_testing(24);
