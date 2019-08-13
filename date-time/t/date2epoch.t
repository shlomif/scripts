#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $cmd = Test::UnixCmdWrap->new;

$ENV{TZ} = 'UTC';

$cmd->run(
    args   => '2003-07-31 17:44:00',
    stdout => ['1059673440'],
);
$cmd->run(
    args   => '2014 15',
    stdout => ['1388588400'],
);
$cmd->run(
    args   => '1970-04-26 10:46:39',
    env    => { TZ => 'US/Pacific' },
    stdout => ['9999999'],
);
$cmd->run(args => '-h', status => 64, stderr => qr/Usage/);

done_testing(12);
