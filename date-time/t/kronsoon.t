#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;
use POSIX qw(strftime);

my $cmd = Test::UnixCmdWrap->new;

$ENV{TZ} = 'US/Pacific';

# KLUGE since have no (easy) means to lock the epoch time used by
# kronsoon to a particular, these mostly just confirm the interface

my $timestamp = qr(
   [0-5][0-9][ ]                # MM - minute 00-59
   (?:[0-1][0-9]|2[0-3])[ ]     # HH - hour 00-23
   (?:[0-2][0-9]|3[01])[ ]      # DD - day 01-31
   (?:0[0-9]|1[0-2])            # mm - month 01-12
)x;

$cmd->run(
    args   => 'cmd to run',
    stdout => qr/^$timestamp \* cmd to run$/,
);
$cmd->run(
    stdin  => "filter\nrun\nthrough\n",
    stdout => qr/^$timestamp \* filter\nrun\nthrough$/,
);
# these tail pad by a space to make it easier to add soemthing after
$cmd->run(
    args   => '--gmtime',
    stdout => qr/^$timestamp \* $/,
);
$cmd->run(
    args   => '--padby=200',
    stdout => qr/^$timestamp \* $/,
);

$cmd->run(args => '-h',           status => 64, stderr => qr/Usage/);
$cmd->run(args => '--padby=59',   status => 65, stderr => qr/out of range/);
$cmd->run(args => '--padby=9999', status => 65, stderr => qr/out of range/);

diag "time test may fail if the system is slow";
my $now = time();
$cmd->run(stdout => [ strftime('%M %H %d %m *', localtime($now + 89)) ]);

done_testing(24);
