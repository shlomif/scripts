#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $test_prog = './v6addr';

my @tests = (
    {   args   => '::1',
        stdout => ["0000:0000:0000:0000:0000:0000:0000:0001"],
    },
    {   args => '-r 2001:db8::c000:22a',
        stdout =>
          ["a.2.2.0.0.0.0.c.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.8.b.d.0.1.0.0.2.ip6.arpa."],
    },
    {   args   => '-rR ::1',
        stdout => ["1.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0"],
    },
    {   args        => '::x',
        exit_status => 65,
        stdout      => [],
        stderr      => qr/could not parse/,
    },
    {   args        => '-q 2001::2010::2040',
        exit_status => 65,
        stdout      => [],
    },
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
