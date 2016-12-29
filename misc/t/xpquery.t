#!perl

use 5.14.0;
use warnings;
use Test::Cmd;
# 3 tests per item in @tests plus any extras
use Test::Most tests => 3 * 4 + 2;
use Test::UnixExit;

my $test_prog = 'xpquery';

my @tests = (
    {   args   => q{'//zoot/text()' t/xip.xml},
        stdout => [ 'cat', 'dog', 'fish' ],
    },
    {   args   => q{-S 'zoot/text()' '//zop' t/xip.xml},
        stdout => [ 'cat', 'dog', 'fish' ],
    },
    {   args => q{-n 'xlink:http://www.w3.org/1999/xlink' '//foo/@xlink:href' t/ns.xml},
        stdout => [' xlink:href="http://example.org"'],
    },
    {   args   => q{-p html '//title/text()' t/foo.html},
        stdout => ['xyz'],
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
