#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $cmd = Test::UnixCmdWrap->new;

# since may not have pbcopy/X11 available, fake things
$ENV{CLIPBOARD} = 'cat';

$cmd->run(
    stdin  => "foo $$\n",
    stdout => [ "foo $$", "foo $$" ],
);
$cmd->run(args => '-h', status => 64, stderr => qr/Usage/);

done_testing(6);
