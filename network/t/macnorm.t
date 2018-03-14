#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $test_prog = './macnorm';

my @tests = (
    {   args   => '00:00:36:af:Af:AF',
        stdout => ["00:00:36:af:af:af"],
    },
    {   args   => '-O 3 -s - 12:34:56',
        stdout => ["12-34-56"],
    },
    {   args   => q{-O 2 -s '' aa/bb CC/DD Ee/Ff},
        stdout => [ "aabb", "ccdd", "eeff" ],
    },
    {   args   => '-O 1 -p 01- aa bb cc',
        stdout => [ "01-aa", "01-bb", "01-cc" ],
    },
    {   args   => '0:0:36:f:F:AF',
        stdout => ["00:00:36:0f:0f:af"],
    },
    # each unique input can have own separator (but must internally use
    # only that character)
    {   args   => q{00:00:36:af:Af:AF 00-00-36-af-Af-AF},
        stdout => [ "00:00:36:af:af:af", "00:00:36:af:af:af" ],
    },
    # case choice
    {   args   => '-X -O 1 aa',
        stdout => [ "AA" ],
    },
    {   args   => '-X -x -O 1 aa',
        stdout => [ "aa" ],
    },
    # invalid stuff
    {   args        => q{''},
        exit_status => 65,
        stderr      => qr/empty string/,
    },
    # this assumes getopt(3) uses -- to stop option processing
    {   args        => q{-- -00:00:36:af:Af:AF},
        exit_status => 65,
        stderr      => qr/xdigit/,
    },
    {   args        => q{00::00:36:af:Af:AF},
        exit_status => 65,
        stderr      => qr/xdigit/,
    },
    {   args        => q{00:00,36:af:Af:AF},
        exit_status => 65,
        stderr      => qr/separator/,
    },
    {   args        => q{00},
        exit_status => 65,
        stderr      => qr/NUL/,
    },
    # trailing garbage
    {   args        => q{-O 1 eat},
        exit_status => 65,
        stderr      => qr/garbage/,
    },
    # even if the trailing stuff is valid...
    {   args        => '0:0:0:0:0:0:0:0:0:0:0',
        exit_status => 65,
        stderr      => qr/garbage/,
    },
    {   args        => '-q 0:0:0:0:0:0:0:0:0:0:0',
        exit_status => 65,
    },
    # not a feature unless you can turn it off
    {   args   => q{-O 1 -T eat},
        stdout => ["ea"],
    },
    {   args   => '-T 0:0:0:0:0:0:0:0:0:0:0',
        stdout => ["00:00:00:00:00:00"],
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
    $test->{stdout}      //= [];
    $test->{stderr}      //= qr/^$/;

    $testcmd->run( exists $test->{args} ? ( args => $test->{args} ) : () );

    my $args = $test->{args} // '';

    exit_is( $?, $test->{exit_status}, "STATUS $test_prog $args" );
    eq_or_diff( [ map { s/\s+$//r } split $/, ( $testcmd->stdout // '' ) ],
        $test->{stdout}, "STDOUT $test_prog $args" );
    ok( $testcmd->stderr =~ m/$test->{stderr}/, "STDERR $test_prog $args" )
      or diag "STDERR " . $testcmd->stderr;
}
done_testing( @tests * 3 );
