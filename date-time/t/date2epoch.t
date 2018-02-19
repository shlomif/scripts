#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $test_prog = './date2epoch';

$ENV{TZ} = 'UTC';

my @tests = (
    {   args   => '2003-07-31 17:44:00',
        stdout => ['1059673440'],
    },
    {   args   => '2014 15',
        stdout => ['1388588400'],
    },
    {   args   => '1970-04-26 10:46:39',
        env    => { TZ => 'US/Pacific' },
        stdout => ['9999999'],
    },
);
my $testcmd = Test::Cmd->new( prog => $test_prog, workdir => '', );

for my $test (@tests) {
    $test->{exit_status} //= 0;
    $test->{stderr}      //= '';

    local @ENV{ keys %{ $test->{env} } } = values %{ $test->{env} };

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
