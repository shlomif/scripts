#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $test_prog = './feed';

# TODO Expect perhaps to test that expect can arrgh too many layers
my @tests = (
    {   args        => "- '$^X' -d -e 42",
        env         => { FEEDRC => 't/feed-nosuchrc' },
        exit_status => 1,
        stdin       => qq{print "should not run\n"\n},
        stderr      => qr/couldn't read file/,
        stdout      => [],
    },
);
my $testcmd = Test::Cmd->new( prog => $test_prog, workdir => '', );

for my $test (@tests) {
    $test->{exit_status} //= 0;
    $test->{stderr}      //= '';

    local @ENV{ keys %{ $test->{env} } } = values %{ $test->{env} };

    $testcmd->run(
        args => $test->{args},
        exists $test->{stdin} ? ( stdin => $test->{stdin} ) : ()
    );

    exit_is( $?, $test->{exit_status}, "STATUS $test_prog $test->{args}" );
    eq_or_diff( [ map { s/\s+$//r } split $/, $testcmd->stdout ],
        $test->{stdout}, "STDOUT $test_prog $test->{args}" );
    ok( $testcmd->stderr =~ $test->{stderr}, "STDERR $test_prog $test->{args}" );
}
$testcmd->run( args => 'foo' );
exit_is( $?, 64, "EX_USAGE of sysexits(3) fame" );
ok( $testcmd->stderr =~ m/Usage/, "help mentions usage" );
done_testing( @tests * 3 + 2 );
