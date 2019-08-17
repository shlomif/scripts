#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $cmd = Test::UnixCmdWrap->new;

delete $ENV{SSH_ORIGINAL_COMMAND};
my $soc = 'rsync --server --sender -vnlHogDtprRxSe.iLsfxC --numeric-ids . ';

# most of the code however involves syslog errors (hard to test) and
# running specific rsync commands (also hard to test)

# no SSH_ORIGINAL_COMMAND
$cmd->run(
    args   => '-n',
    status => 2,
);
# should not match. hopefully not run.
$cmd->run(
    args   => '-n',
    env    => { SSH_ORIGINAL_COMMAND => 'rm -rf /' },
    status => 4,
);
# assumes "t" test directory exists and is accessible
$cmd->run(
    args   => '-n',
    env    => { SSH_ORIGINAL_COMMAND => $soc . 't' },
    status => 0,
    stdout => qr(^t$),
);
# usually naughty .. runs are not allowed (if they need to be
# supported for some reason, fiddle with the code to allow that)
$cmd->run(
    args   => '-n',
    env    => { SSH_ORIGINAL_COMMAND => $soc . 'asdf/../../etc' },
    status => 7,
);
# NOTE some vendors put files into this directory (??)
$cmd->run(
    args   => '-n',
    env    => { SSH_ORIGINAL_COMMAND => $soc . '/var/empty/' . $$ },
    status => 5,
);
$cmd->run(
    args   => '-h',
    env    => { SSH_ORIGINAL_COMMAND => $soc . '/' },
    status => 64,
    stderr => qr/supported/,
);

done_testing(18);
