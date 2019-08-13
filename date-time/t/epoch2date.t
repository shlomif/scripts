#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $cmd = Test::UnixCmdWrap->new;

$ENV{TZ} = 'UTC';

$cmd->run(
    args   => '-',
    stdin  => '1059673440 1404433529',
    stdout => [ '2003-07-31 17:44:00 UTC', '2014-07-04 00:25:29 UTC' ],
);
$cmd->run(
    args   => '1475682078014325',
    stdout => ['2016-10-05 15:41:18 UTC'],
    stderr => qr/guess microseconds for 1475682078014325/,
);
$cmd->run(
    args   => '-f "%Y" 1483228801',     # 2017 in UTC
    env    => { TZ => 'US/Pacific' },
    stdout => ['2016'],
);
$cmd->run(args => '-h', status => 64, stderr => qr/Usage/);

done_testing(12);
