#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $test_prog = './fdsplit';

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
my $testcmd = Test::Cmd->new( prog => $test_prog, workdir => '', );

for my $test (@tests) {
    $test->{exit_status} //= 0;
    $test->{stderr}      //= '';

    $testcmd->run( args => $test->{args} );

    exit_is( $?, $test->{exit_status}, "STATUS $test_prog $test->{args}" );
    eq_or_diff( [ map { s/\s+$//r } split $/, $testcmd->stdout ],
        $test->{stdout}, "STDOUT $test_prog $test->{args}" );
    is( $testcmd->stderr, $test->{stderr}, "STDERR $test_prog $test->{args}" );
}
$testcmd->run( args => '-h' );
exit_is( $?, 64, "EX_USAGE of sysexits(3) fame" );
ok( $testcmd->stderr =~ m/Usage/, "help mentions usage" );
done_testing( @tests * 3 + 2 );
