#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;
use Cwd qw(getcwd);

my $cmd = Test::UnixCmdWrap->new;

# from the Cwd(3pm) docs this should be compatible with the realpath(3)
# that getpof uses when directories are given...
my $test_dir = catfile(getcwd, 't');

$cmd->run(
    args   => 'getpof.t',
    stdout => ['./t'],
);
$cmd->run(
    args   => 'getpof.t . t',
    stdout => [ $test_dir, $test_dir ],
);
$cmd->run(
    args   => '-r recurse',
    stdout => [ qw(./t ./t/recurse) ],
);
$cmd->run(
    args   => '-0r recurse',
    stdout => ["./t\0./t/recurse\0"],
);
$cmd->run(args => '-h', status => 64, stderr => qr/Usage/);

done_testing(15);
