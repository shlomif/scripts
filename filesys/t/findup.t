#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;
use Cwd qw(getcwd);
use File::Basename qw(dirname);

my $cmd = Test::UnixCmdWrap->new;

my $prog_dir  = getcwd;
my $first_dir = (splitdir($prog_dir))[1];

$cmd->run(
    args   => 'findup',
    stdout => [$prog_dir],
);
$cmd->run(
    args   => '-q -f findup',
    stdout => [],
);
$cmd->run(
    args   => 'filesys',
    stdout => [ dirname($prog_dir) ],
);
$cmd->run(
    args   => '-q -d filesys',
    stdout => [],
);
# NOTE low but non-zero odds of false positive
$cmd->run(
    args   => random_filename(),
    stdout => [],
    status => 1,
);
# special case
$cmd->run(
    args   => '/',
    stdout => ['/'],
);
# special special case
$cmd->run(
    args   => '-q /',
    stdout => [],
);
# special special special case
$cmd->run(
    args   => '-f /',
    stdout => [],
    status => 1,
);
# there are far better ways of doing this, but the customer is
# always right...
$cmd->run(
    args   => '-f /etc/passwd',
    stdout => ['/'],
);
$cmd->run(
    args   => '-d /etc/passwd',
    stdout => [],
    status => 1,
);
# NOTE this test makes assumptions about the repository directory
# path being under $HOME, as if not -H won't stop the upward search
# anywhere
$cmd->run(
    args   => "-H '$first_dir'",
    stdout => [],
    status => 1,
);

$cmd->run(args => '-h', status => 64, stderr => qr/Usage/);

# what if HOME is set to something unusual? (a productive use for this
# would be to run something like `HOME=/var/repository findup -H bla` to
# limit the search to under that custom directory)
{
    # no HOME set should fall back to the getpwuid(3) call, and again
    # making assumptions about where this repository is...
    local %ENV;
    delete $ENV{HOME};

    $cmd->run(args => "-H '$first_dir'", status => 1);

    # a bad HOME should (in theory) allow the first dir to be found...
    # (bonus! it exposed another edge-case in the C I had overlooked)
    $ENV{HOME} = random_filename();
    $cmd->run(args => "-H '$first_dir'", stdout => qr(/\n));
}

done_testing(42);

sub random_filename {
    my @allowed = ('A' .. 'Z', 'a' .. 'z', 0 .. 9, '_');
    join '', map { $allowed[ rand @allowed ] } 1 .. (32 + rand 32);
}
