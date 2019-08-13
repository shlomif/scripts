#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;
use POSIX qw(strftime);

my $cmd = Test::UnixCmdWrap->new;

$ENV{TZ} = 'US/Pacific';

my $cur_epoch = time();
# NOTE tests will fail if the day rolls over somewhere between here and
# the last date-related ->run, below.
my $three_days = strftime("%b %d", localtime($cur_epoch + 3 * 86400));
my $four_weeks = strftime("%b %d", localtime($cur_epoch + 4 * 7 * 86400));

$cmd->run(
    args   => '3',
    stdout => [$three_days],
);
$cmd->run(
    args   => '+3',
    stdout => [$three_days],
);
$cmd->run(
    args   => '+3d',
    stdout => [$three_days],
);
$cmd->run(
    args   => '+4w',
    stdout => [$four_weeks],
);
$cmd->run(args => '-h', status => 64, stderr => qr/Usage/);

done_testing(15);
