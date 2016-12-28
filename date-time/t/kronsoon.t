#!perl

use 5.14.0;
use warnings;
use POSIX qw(strftime);
use Test::Cmd;
# 3 tests per item in @tests plus any extras
use Test::Most tests => 3 * 4 + 5;
use Test::UnixExit;

my $test_prog = 'kronsoon';

$ENV{TZ} = 'US/Pacific';

# KLUGE since have no (easy) means to lock the epoch time used by
# kronsoon to a particular, these mostly just confirm the interface.
my @tests = (
    {   args   => 'cmd to run',
        stdout => ['MM HH DD mm * cmd to run'],

    },
    {   stdin  => "filter\nrun\nthrough\n",
        stdout => [ 'MM HH DD mm * filter', 'run', 'through' ],

    },
    {   args   => '--gmtime',
        stdout => ['MM HH DD mm *'],

    },
    {   args   => '--padby=200',
        stdout => ['MM HH DD mm *'],
    },
);
my $testcmd = Test::Cmd->new(
    prog    => $test_prog,
    verbose => 0,
    workdir => '',
);

for my $test (@tests) {
    $test->{stderr}      //= '';
    $test->{exit_status} //= 0;

    $testcmd->run(
        exists $test->{args}  ? ( args  => $test->{args} )  : (),
        exists $test->{stdin} ? ( stdin => $test->{stdin} ) : ()
    );

    $test->{args} //= '';
    exit_is( $?, $test->{exit_status}, "STATUS $test_prog $test->{args}" );
    eq_or_diff(
        [   map {
                $_ =
                  s/^[0-5][0-9][ ]        # MM - minute 00-59
                 (?:[0-1][0-9]|2[0-3])[ ] # HH - hour 00-23
                 (?:[0-2][0-9]|3[01])[ ]  # DD - day 01-31
                 (?:0[0-9]|1[0-2])        # mm - month 01-12
                 /MM HH DD mm/rx;
                s/\s+$//r
              }
              split $/,
            $testcmd->stdout
        ],
        $test->{stdout},
        "STDOUT $test_prog $test->{args}"
    );
    is( $testcmd->stderr, $test->{stderr}, "STDERR $test_prog $test->{args}" );
}

# any extras

$testcmd->run( args => '-h' );
exit_is( $?, 64, "EX_USAGE of sysexits(3) fame" );
ok( $testcmd->stderr =~ m/Usage/, "help mentions usage" );

$testcmd->run( args => '--padtime=59' );
exit_is( $?, 65, "pad time too low" );

$testcmd->run( args => '--padtime=301' );
exit_is( $?, 65, "pad time too high" );

diag "time test may fail if system slow";
my $now = time();
$testcmd->run();
my $expected = strftime( '%M %H %d %m *', localtime( $now + 89 ) );
my ($got) = map { s/\s+$//r } split $/, $testcmd->stdout;
is( $got, $expected, "time test" );
