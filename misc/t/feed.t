#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $cmd = Test::UnixCmdWrap->new;

# TODO Expect perhaps to test that expect can arrgh too many layers
$cmd->run(
    args   => "- '$^X' -d -e 42",
    env    => { FEEDRC => 't/feed-nosuchrc' },
    status => 1,
    stdin  => qq{print "should not run\n"\n},
    stderr => qr/couldn't read file/,
);
$cmd->run(args => 'foo', status => 64, stderr => qr/Usage/);

done_testing(6);
