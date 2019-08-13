#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $cmd = Test::UnixCmdWrap->new;

$cmd->run(args => '-h', stderr => qr/^Usage/, status => 64);
$cmd->run(stdin => 'aabbbb a', stdout => [ '2 a', '4 b', '1 0x20', '1 a' ]);
$cmd->run(
    args   => '-',
    stdin  => join('', "\t" x 9999, 'b' x 9999),
    stdout => [ '9999 0x09', '9999 b' ],
);
$cmd->run(args => 't/rcc-input', stdout => [ '5 a', '1 0x0A' ]);

done_testing(12);
