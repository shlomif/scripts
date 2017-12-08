#!perl

use 5.14.0;
use warnings;
use Test::Cmd;
# 3 tests per item in @tests plus any extras
use Test::Most tests => 3 * 7 + 0;
use Test::UnixExit;

my $test_prog = './v6addr';

my @tests = (
    {   args   => '::1',
        stdout => ["0000:0000:0000:0000:0000:0000:0000:0001"],
    },
    {   args => '-r ::1',
        stdout =>
          ["1.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.ip6.arpa."],
    },
    {   args   => '-R ::1',
        stdout => ["1.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0"],
    },
    # should not parse
    {   args        => '::x',
        exit_status => 65,
        stdout      => [],
        stderr      => qr/could not parse ipv6-address/,
    },
    {   args        => '-q ::x',
        exit_status => 65,
        stdout      => [],
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

my $testcmd = Test::Cmd->new(
    prog    => $test_prog,
    verbose => 0,
    workdir => '',
);

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

# any extras
