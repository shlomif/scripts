#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;
use Time::HiRes qw(gettimeofday tv_interval);

my $cmd = Test::UnixCmdWrap->new;

delete @ENV{qw(OUTPUT_PREFIX SHELL TIMEOUT)};
$ENV{CLIPBOARD} = 'cat';

# NOTE see TODOs elsewhere (snooze.t)
my $tolerance = 0.15;

# what does a command-not-found failure look like? (while making wild
# assumptions concerning the not-existence of askdjfksdjfkds...)
$cmd->run(
    args   => "/var/empty/askdjfksdjfkds",
    status => 1,
    stderr => qr/forexample/
);

# ditto for bad CLIPBOARD command
{
    local $ENV{CLIPBOARD} = "/var/empty/askdjfksdjfkds";
    $cmd->run(args => "echo hi", status => 1, stderr => qr/forexample/);
}

# can we at least echo/cat something?
$cmd->run(
    args   => "echo '$$'",
    stdout => [ "    \$ echo $$", "    $$", "    \$" ]
);
$cmd->run(
    args   => "echo '$$'",
    env    => { OUTPUT_PREFIX => '' },
    stdout => [ "\$ echo $$", "$$", "\$" ]
);
$cmd->run(
    args   => "echo '$$'",
    env    => { SHELL => 'zsh' },
    stdout => [ "    % echo $$", "    $$", "    %" ]
);

# this one should timeout before completion
my $start = [gettimeofday];
$cmd->run(
    args   => "sleep 7",
    env    => { TIMEOUT => '0.5' },
    stdout => qr/sleep 7/
);
ok(tv_interval($start) < 3);

$cmd->run(status => 64, stderr => qr/Usage/);

done_testing(22);
