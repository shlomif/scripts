#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $test_prog = './xpquery';

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
    # in theory XML::LibXML handles the input encoding (via the ?xml ...
    # statement) and in theory Test::Cmd does not muck with the output
    # in any way
    {   args   => q{-E UTF-8 '//word/text()' t/utf8.xml},
        stdout => ["\xe5\xbd\x81"],
    },
    {   args   => q{-E UTF-8 '//word/text()' t/shift_jis.xml},
        stdout => ["\xe5\xbd\x81"],
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
