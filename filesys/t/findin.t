#!perl

use 5.14.0;
use warnings;
use Cwd qw(getcwd);
use File::Spec ();
use Test::Cmd;
# 3 tests per item in @tests plus any extras
use Test::Most tests => 1 + 3 * 9 + 6;

my $test_prog = 'findin';

my $prog_dir = getcwd;

bail_on_fail;

# NixOS in particular does not follow what limited conventions there are
my $ls_path = whereis('ls');
ok( defined $ls_path, "'ls' is in PATH" );

restore_fail;

# assuming no race condition creation afterwards, or perhaps some sort
# of OS-level "hey! let's run something else instead!" "feature"...
my $nosuch;
do { $nosuch = random_filename() } until !defined whereis($nosuch);

$ENV{FINDIN_PATH1} = join ':', 'findin-nodir', 'findin-nadadir', $prog_dir;
$ENV{FINDIN_PATH2} = $prog_dir;
$ENV{FINDIN_EMPTY} = '';

my $findin_path = File::Spec->catfile( $prog_dir, 'findin' );

my $findin_dirpath1 = $ENV{FINDIN_PATH1} =~ s/:/\n/gr;
my $findin_dirpath1_null = $ENV{FINDIN_PATH1} =~ s/:/\0/gr;

# TODO stdin - tests as well...
my @tests = (
    {   args   => 'ls',
        stdout => [$ls_path],
    },
    {   args        => $nosuch,
        stdout      => [],
        exit_status => 2,
    },
    {   args   => 'ls PATH',
        stdout => [$ls_path],
    },
    {   args   => 'findin FINDIN_PATH1',
        stdout => [$findin_path],
    },
    {   args   => 'findin FINDIN_PATH2',
        stdout => [$findin_path],
    },
    {   args        => 'ls FINDIN_EMPTY',
        stdout      => [],
        exit_status => 2,
    },
    {   args   => 'findin -',
        stdin  => $findin_dirpath1,
        stdout => ["$findin_path"],
    },
    # stdin read -0 also means stdin must be nullsep
    {   args   => '-0 findin -',
        stdin  => $findin_dirpath1_null,
        stdout => ["$findin_path\0"],
    },
    # ENV regardless splits on :
    {   args   => '-0 ls',
        stdout => ["$ls_path\0"],
    },
);
my $testcmd = Test::Cmd->new(
    prog    => $test_prog,
    verbose => 0,
    workdir => '',
);

for my $test (@tests) {
    $test->{exit_status} //= 0;
    $test->{stderr}      //= '';

    $testcmd->run(
        args => $test->{args},
        exists $test->{stdin} ? ( stdin => $test->{stdin} ) : (),
    );

    is( $? >> 8, $test->{exit_status}, "STATUS $test_prog $test->{args}" );
    eq_or_diff( [ map { s/\s+$//r } split $/, $testcmd->stdout ],
        $test->{stdout}, "STDOUT $test_prog $test->{args}" );
    is( $testcmd->stderr, $test->{stderr}, "STDERR $test_prog $test->{args}" );
}

# any extras

$testcmd->run( args => '-h' );
is( $? >> 8, 64, "EX_USAGE of sysexits(3) fame" );
ok( $testcmd->stderr =~ m/Usage/, "help mentions usage" );

# Note, be sure to quote:
# for the shell will glob
$testcmd->run( args => q{'findin.*' FINDIN_PATH1} );
is( $? >> 8, 0, "found files by glob" );
my $count;
$count++ for $testcmd->stdout =~ m{.findin\.[1c]$}gm;
is( $count, 2, "dot c and man page found" );
ok( $testcmd->stderr =~ m/^$/, "no stderr" );

ok( !-e "$test_prog.core", "$test_prog did not produce core" );

sub random_filename {
    my @allowed = ( 'A' .. 'Z', 'a' .. 'z', 0 .. 9, '_' );
    join '', map { $allowed[ rand @allowed ] } 1 .. 32;
}

sub whereis {
    my ($what) = @_;
    my $path;
    for my $dir ( split ':', $ENV{PATH} ) {
        my $tmp = File::Spec->catfile( $dir, $what );
        if ( -x $tmp ) {
            $path = $tmp;
            last;
        }
    }
    return $path;
}
