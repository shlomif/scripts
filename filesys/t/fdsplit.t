#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $cmd = Test::UnixCmdWrap->new;

$cmd->run(
    args   => 'ext foo.bar',
    stdout => ['bar'],
);
$cmd->run(
    args   => 'root foo.bar',
    stdout => ['foo'],
);
$cmd->run(
    args   => 'root tar.ball.gz',
    stdout => ['tar.ball'],
);
$cmd->run(
    args   => 'ext tar.ball.gz',
    stdout => ['gz'],
);
# special cases with leading and trailing dots in name
$cmd->run(
    args   => 'ext .dotfile',
    stdout => ['dotfile'],
);
$cmd->run(
    args   => 'root .dotfile',
    stdout => [],
);
$cmd->run(
    args   => 'ext trail.',
    stdout => [],
);
$cmd->run(
    args   => 'root trail.',
    stdout => ['trail'],
);
# options
$cmd->run(
    args   => '-d x root fooxbar',
    stdout => ['foo'],
);
$cmd->run(
    args   => '-0 root null.blah',
    stdout => ["null\0"],
);
# with dirname portions specified
$cmd->run(
    args   => 'root /etc/pf.conf',
    stdout => ['/etc/pf'],
);
$cmd->run(
    args   => 'ext /etc/pf.conf',
    stdout => ['conf'],
);
$cmd->run(
    args   => 'root /etc/rc.d/',
    stdout => ['/etc/rc.d/'],
);
$cmd->run(args => 'ext /etc/rc.d/');
$cmd->run(args => 'h', status => 64, stderr => qr/Usage/);

done_testing(45);
