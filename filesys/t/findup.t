#!perl

use 5.14.0;
use warnings;
use Cwd qw(getcwd);
use File::Basename qw(dirname);
use File::Spec ();
use Test::Cmd;
# 3 tests per item in @tests plus any extras
use Test::Most tests => 3 * 11 + 8;
use Test::UnixExit;

my $test_prog = './findup';

my $prog_dir  = getcwd;
my $first_dir = ( File::Spec->splitdir($prog_dir) )[1];

my @tests = (
    {   args   => 'findup',
        stdout => [$prog_dir],
    },
    {   args   => '-q -f findup',
        stdout => [],
    },
    {   args   => 'filesys',
        stdout => [ dirname($prog_dir) ],
    },
    {   args   => '-q -d filesys',
        stdout => [],
    },
    # NOTE low but non-zero odds of false positive
    {   args        => random_filename(),
        stdout      => [],
        exit_status => 1,
    },
    # special case
    {   args   => '/',
        stdout => ['/'],
    },
    # special special case
    {   args   => '-q /',
        stdout => [],
    },
    # special special special case
    {   args        => '-f /',
        stdout      => [],
        exit_status => 1,
    },
    # there are far better ways of doing this, but the customer is
    # always right...
    {   args   => '-f /etc/passwd',
        stdout => ['/'],
    },
    {   args        => '-d /etc/passwd',
        stdout      => [],
        exit_status => 1,
    },
    # NOTE this test makes assumptions about the repository directory
    # path being under $HOME, as if not -H won't stop the upward search
    # anywhere
    {   args        => "-H '$first_dir'",
        stdout      => [],
        exit_status => 1,
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

    $testcmd->run( args => $test->{args} );

    exit_is( $?, $test->{exit_status}, "STATUS $test_prog $test->{args}" );
    eq_or_diff( [ map { s/\s+$//r } split $/, $testcmd->stdout ],
        $test->{stdout}, "STDOUT $test_prog $test->{args}" );
    is( $testcmd->stderr, $test->{stderr}, "STDERR $test_prog $test->{args}" );
}

# any extras

$testcmd->run( args => '-h' );
exit_is( $?, 64, "EX_USAGE of sysexits(3) fame" );
ok( $testcmd->stderr =~ m/Usage/, "help mentions usage" );

# what if HOME is set to something unusual? (a productive use for this
# would be to run something like `HOME=/var/repository findup -H bla` to
# limit the search to under that custom directory)
{
    # no HOME set should fall back to the getpwuid(3) call, and again
    # making assumptions about where this repository is...
    local %ENV;
    delete $ENV{HOME};

    $testcmd->run( args => "-H '$first_dir'" );

    exit_is( $?, 1, "should not escape HOME from getpwuid" );
    ok( $testcmd->stdout eq "" );
    ok( $testcmd->stderr eq "" );

    # a bad HOME should (in theory) allow the first dir to be found...
    # (bonus! it exposed another edge-case in the C I had overlooked)
    $ENV{HOME} = random_filename();
    $testcmd->run( args => "-H '$first_dir'" );

    exit_is( $?, 0, "found first dir" );
    ok( $testcmd->stdout eq "/\n", "found dir of first dir" );
    ok( $testcmd->stderr eq "" );
}

sub random_filename {
    my @allowed = ( 'A' .. 'Z', 'a' .. 'z', 0 .. 9, '_' );
    join '', map { $allowed[ rand @allowed ] } 1 .. ( 32 + rand 32 );
}
