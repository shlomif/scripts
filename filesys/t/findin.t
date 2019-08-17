#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;
use Cwd qw(getcwd);
use File::Which;

my $cmd = Test::UnixCmdWrap->new;

my $prog_dir = getcwd();

bail_on_fail;

# NixOS in particular does not follow what limited conventions there are
my $ls_path = which('ls');
ok(defined $ls_path, "'ls' is in PATH");

restore_fail;

# assuming no race condition creation afterwards, or perhaps some sort
# of OS-level "hey! let's run something else instead!" "feature"...
my $nosuch;
do { $nosuch = random_filename() } until !defined which($nosuch);

$ENV{FINDIN_PATH1} = join ':', 'findin-nodir', 'findin-nadadir', $prog_dir;
$ENV{FINDIN_PATH2} = $prog_dir;
$ENV{FINDIN_EMPTY} = '';

my $findin_path = catfile($prog_dir, 'findin');

my $findin_dirpath1      = $ENV{FINDIN_PATH1} =~ s/:/\n/gr;
my $findin_dirpath1_null = $ENV{FINDIN_PATH1} =~ s/:/\0/gr;

$cmd->run(
    args   => 'ls',
    stdout => [$ls_path],
);
$cmd->run(
    args   => "'$nosuch'",
    stdout => [],
    status => 2,
);
$cmd->run(
    args   => 'ls PATH',
    stdout => [$ls_path],
);
$cmd->run(
    args   => 'findin FINDIN_PATH1',
    stdout => [$findin_path],
);
$cmd->run(
    args   => 'findin FINDIN_PATH2',
    stdout => [$findin_path],
);
$cmd->run(
    args   => 'ls FINDIN_EMPTY',
    stdout => [],
    status => 2,
);
$cmd->run(
    args   => 'findin -',
    stdin  => $findin_dirpath1,
    stdout => ["$findin_path"],
);
# are /path/ trailing slashes cleaned up?
$cmd->run(
    args   => 'findin.1 -',
    stdin  => "$findin_dirpath1/",
    stdout => ["$findin_path.1"],
);
# stdin read -0 also means stdin must be nullsep
$cmd->run(
    args   => '-0 findin.c -',
    stdin  => $findin_dirpath1_null,
    stdout => ["$findin_path.c\0"],
);
# ENV regardless splits on :
$cmd->run(
    args   => '-0 ls',
    stdout => ["$ls_path\0"],
);
$cmd->run(args => '-h', status => 64, stderr => qr/Usage/);

# NOTE be sure to quote:
# for the shell will glob
# and all that you wrote
# oh--a belly flop
my $o = $cmd->run(args => q{'findin.*' FINDIN_PATH1}, stdout => qr/^/);
my $count = () = $o->stdout =~ m{.findin\.[1c]$}gm;
is($count, 2, "dot c and man page found");

done_testing(38);

sub random_filename {
    my @allowed = ('A' .. 'Z', 'a' .. 'z', 0 .. 9, '_');
    join '', map { $allowed[ rand @allowed ] } 1 .. 32;
}
