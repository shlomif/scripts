#!perl

use 5.14.0;
use warnings;
use Test::Cmd;
# 3 tests per item in @tests plus any extras
use Test::Most tests => 3 * 14 + 3;

my $test_prog = 'fdsplit';

my @tests = (
    {   args   => 'ext foo.bar',
        stdout => ['bar'],
    },
    {   args   => 'root foo.bar',
        stdout => ['foo'],
    },
    {   args   => 'root tar.ball.gz',
        stdout => ['tar.ball'],
    },
    {   args   => 'ext tar.ball.gz',
        stdout => ['gz'],
    },
    # special cases with leading and trailing dots in name
    {   args   => 'ext .dotfile',
        stdout => ['dotfile'],
    },
    {   args   => 'root .dotfile',
        stdout => [],
    },
    {   args   => 'ext trail.',
        stdout => [],
    },
    {   args   => 'root trail.',
        stdout => ['trail'],
    },
    # options
    {   args   => '-d x root fooxbar',
        stdout => ['foo'],
    },
    {   args   => '-0 root null.blah',
        stdout => ["null\0"],
    },
    # with dirname portions specified
    {   args   => 'root /etc/pf.conf',
        stdout => ['/etc/pf'],
    },
    {   args   => 'ext /etc/pf.conf',
        stdout => ['conf'],
    },
    {   args   => 'root /etc/rc.d/',
        stdout => ['/etc/rc.d/'],
    },
    {   args   => 'ext /etc/rc.d/',
        stdout => [],
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

    is( $? >> 8, $test->{exit_status}, "STATUS $test_prog $test->{args}" );
    eq_or_diff( [ map { s/\s+$//r } split $/, $testcmd->stdout ],
        $test->{stdout}, "STDOUT $test_prog $test->{args}" );
    is( $testcmd->stderr, $test->{stderr}, "STDERR $test_prog $test->{args}" );
}

# any extras

$testcmd->run( args => '-h' );
is( $? >> 8, 64, "EX_USAGE of sysexits(3) fame" );
ok( $testcmd->stderr =~ m/Usage/, "help mentions usage" );

ok( !-e "$test_prog.core", "$test_prog did not produce core" );
