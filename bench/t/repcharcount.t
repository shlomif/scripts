#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $test_prog = './repcharcount';

my @tests = (
    {   args        => '-h',
        stderr      => qr/^Usage/,
        exit_status => 64,
    },
    {   stdin  => 'aabbbb a',
        stdout => "2 a\n4 b\n1 0x20\n1 a\n",
    },
    {   args   => '-',
        stdin  => join( '', "\t" x 9999, "b" x 9999 ),
        stdout => "9999 0x09\n9999 b\n",
    },
    {   args   => 't/rcc-input',
        stdout => "5 a\n1 0x0A\n",
    },
);
my $testcmd = Test::Cmd->new( prog => $test_prog, workdir => '', );

for my $test (@tests) {
    $test->{exit_status} //= 0;
    $test->{stdout}      //= '';
    $test->{stderr}      //= qr/^$/;

    $testcmd->run(
        exists $test->{args}  ? ( args  => $test->{args} )  : (),
        exists $test->{stdin} ? ( stdin => $test->{stdin} ) : (),
    );

    $test->{args} //= '';
    exit_is( $?, $test->{exit_status}, "STATUS $test_prog $test->{args}" );
    is( $testcmd->stdout, $test->{stdout}, "STDOUT $test_prog $test->{args}" );
    ok( $testcmd->stderr =~ m/$test->{stderr}/, "STDERR $test_prog $test->{args}" );
}
done_testing( @tests * 3 );
