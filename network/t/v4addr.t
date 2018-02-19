#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $test_prog = './v4addr';

my @tests = (
    {   args   => '127.0.0.1',
        stdout => ["127.0.0.1"],
    },
    {   args   => '-f 10.11.12.13',
        stdout => ["10.11.12.13"],
    },
    {   args   => '-r 192.0.2.42',
        stdout => ["42.2.0.192.in-addr.arpa."],
    },
    {   args   => '-a 192.0.2.43',
        stdout => [ "192.0.2.43", "43.2.0.192.in-addr.arpa." ],
    },
    {   args        => '127.1',
        stdout      => [],
        stderr      => qr/could not parse/,
        exit_status => 65,
    },
    {   args        => '-q 127.0x0.1',
        stdout      => [],
        exit_status => 65,
    },
    # standard help
    {   args        => '-h',
        exit_status => 64,
        stdout      => [],
        stderr      => qr/Usage/,
    },
    {   exit_status => 64,
        stdout      => [],
        stderr      => qr/Usage/,
    },
);
my $testcmd = Test::Cmd->new( prog => $test_prog, workdir => '', );

for my $test (@tests) {
    $test->{exit_status} //= 0;
    $test->{stderr}      //= qr/^$/;

    $testcmd->run( exists $test->{args} ? ( args => $test->{args} ) : () );

    my $args = $test->{args} // '';

    exit_is( $?, $test->{exit_status}, "STATUS $test_prog $args" );
    eq_or_diff( [ map { s/\s+$//r } split $/, ( $testcmd->stdout // '' ) ],
        $test->{stdout}, "STDOUT $test_prog $args" );
    ok( $testcmd->stderr =~ m/$test->{stderr}/, "STDERR $test_prog $args" );
}
done_testing( @tests * 3 );
