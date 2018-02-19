#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;
use Expect;
use Time::HiRes qw(gettimeofday tv_interval);

# TODO how tight can this be vs. false positive risk? TODO also should
# skew to allow for longer durations, but not shorter ones
# also used in timeout.t
my $tolerance = 0.15;

my $test_prog = './snooze';

diag "snooze tests will take time...";

control_plus_c();

sleep_for( '3',      3, 10 );
sleep_for( '1s1s1s', 3, 10 );

sig_info();

done_testing(16);

sub control_plus_c {
    my $exp = newexpect();
    ok( $exp->spawn( $test_prog, '10m' ), "expect object spawned" );

    sleep 2;
    $exp->send("\003");

    my $return;
    $exp->expect(
        6,
        'eof'     => sub { $return = 'eof' },
        'timeout' => sub { $return = 'timeout' }
    );

    ok( $exp->before =~ m/snooze: 9m5[0-9]s rem/,
        "time remains message: " . $exp->before
    );

    is( $return,               'eof', "snooze exits after control+c" );
    exit_is( $exp->exitstatus, 1,     "exit code for handled control+c" );
}

sub newexpect {
    my $exp = Expect->new;
    $exp->raw_pty(1);
    #$exp->debug(3);
    return $exp;
}

sub sig_info {
    my $exp      = newexpect();
    my $how_long = 6;
    my $start    = [gettimeofday];
    ok( $exp->spawn( $test_prog, $how_long ), "expect object spawned" );

    sleep 2;
    kill( INFO => $exp->pid );

    my $return;
    $exp->expect(
        10,
        'eof'     => sub { $return = 'eof' },
        'timeout' => sub { $return = 'timeout' }
    );
    my $elapsed_error = abs( tv_interval($start) - $how_long ) / $how_long;

    ok( $exp->before =~ m/snooze: [0-9]s rem/,
        "time remains message: " . $exp->before
    );
    is( $return, 'eof', "snooze exited" );
    ok( $elapsed_error < $tolerance,
        "duration variance out of bounds: $elapsed_error" );
}

sub sleep_for {
    my ( $how_long, $expected, $timeout_guard ) = @_;
    my $exp   = newexpect();
    my $start = [gettimeofday];
    ok( $exp->spawn( $test_prog, $how_long ), "expect object spawned" );

    my $return;
    $exp->expect(
        $timeout_guard,
        'eof'     => sub { $return = 'eof' },
        'timeout' => sub { $return = 'timeout' }
    );
    my $elapsed_error = abs( tv_interval($start) - $expected ) / $expected;

    is( $return,               'eof', "snooze exits normally" );
    exit_is( $exp->exitstatus, 0,     "exit code for normal exit" );
    ok( $elapsed_error < $tolerance,
        "duration variance out of bounds: $elapsed_error" );
}
