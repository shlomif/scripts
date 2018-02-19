#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $test_prog = './sgrax';

# hopefully bigger than any typical buffer used by who knows what
my $bfs = join '', map { chr( 65 + rand(26) ) } 1 .. ( 16411 + rand 1031 );

my @tests = (
    {   args   => "cat foo '$$' bar '$$' zot '$$' asdf '$$' qwer '$$' zxcv '$$' x y z",
        stdout => ["foo $$ bar $$ zot $$ asdf $$ qwer $$ zxcv $$ x y z"],
    },
    {   args   => "cat '$bfs'",
        stdout => [$bfs],
    },
    {   args   => "cat foo '$bfs' bar '$bfs'",
        stdout => ["foo $bfs bar $bfs"],
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

$testcmd->run( args => 'foo' );
exit_is( $?, 64, "EX_USAGE of sysexits(3) fame" );
ok( $testcmd->stderr =~ m/Usage/, "help mentions usage" );
done_testing( @tests * 3 + 4 );
