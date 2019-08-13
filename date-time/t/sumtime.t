#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $cmd = Test::UnixCmdWrap->new;

$cmd->run(
    args   => '300',
    stdout => ['5m'],
);
$cmd->run(
    args   => '1w 3d 2h 6m 5s',
    stdout => ['1w 3d 2h 6m 5s'],
);
$cmd->run(
    args   => '-c -',
    stdin  => "1s 20m\n30m 40m\n",
    stdout => ['1h30m1s'],
);
$cmd->run(args => '-h', status => 64, stderr => qr/Usage/);

done_testing(12);
