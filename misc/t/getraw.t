#!perl

use 5.14.0;
use warnings;
use Expect;
use Test::Most tests => 23;
use Test::UnixExit;

my $test_prog = './getraw';

diag "tests will take some time...";

my $eoft_result;    # set by eof_or_timeout depending on result of that

my $exp = newexpect();
ok( $exp->spawn($test_prog), "expect object spawned: $!" );
eof_or_timeout( $exp, 2, \$eoft_result );
is( $eoft_result, 'timeout', "getraw still waits" );
$exp->print("n");
eof_or_timeout( $exp, 4, \$eoft_result );
is( $eoft_result, 'eof', "getraw exited" );
exit_is( $exp->exitstatus, 2, "exit status for negative" );

$exp = newexpect();
ok( $exp->spawn($test_prog), "expect object spawned: $!" );
sleep 1;
$exp->print("y");
eof_or_timeout( $exp, 4, \$eoft_result );
is( $eoft_result, 'eof', "getraw exited" );
exit_is( $exp->exitstatus, 0, "exit status for positive" );

$exp = newexpect();
ok( $exp->spawn($test_prog), "expect object spawned: $!" );
sleep 1;
kill INT => $exp->pid;
eof_or_timeout( $exp, 4, \$eoft_result );
is( $eoft_result, 'eof', "getraw exited" );
exit_is( $exp->exitstatus, 3, "status for exit by signal" );

$exp = newexpect();
ok( $exp->spawn( $test_prog, '--timeout=1.9' ), "expect object spawned: $!" );
eof_or_timeout( $exp, 3, \$eoft_result );
is( $eoft_result, 'eof', "getraw exited" );
exit_is( $exp->exitstatus, 4, "status for exit by timeout" );

# "anykey" - any keypress (excepting signals, \003 will still kill)
# exits the program
$exp = newexpect();
ok( $exp->spawn( $test_prog, qw/-o *:0/ ), "expect object spawned: $!" );
sleep 1;
$exp->print("n");
eof_or_timeout( $exp, 4, \$eoft_result );
is( $eoft_result, 'eof', "getraw exited" );
exit_is( $exp->exitstatus, 0, "anykey exit status" );

# despite prints, this one should timeout as delay should prohibt keys
# from being accepted for longer than timeout allows
$exp = newexpect();
ok( $exp->spawn( $test_prog, qw/--delay=4.2 --timeout=2 -o *:0/ ),
    "expect object spawned: $!" );
$exp->print("n");
$exp->print("y");
eof_or_timeout( $exp, 3, \$eoft_result );
is( $eoft_result, 'eof', "getraw exited" );
exit_is( $exp->exitstatus, 4, "exit by timeout" );

$exp = newexpect();
ok( $exp->spawn( $test_prog, '-h' ), "expect object spawned: $!" );
eof_or_timeout( $exp, 3, \$eoft_result );
is( $eoft_result, 'eof', "getraw exited" );
exit_is( $exp->exitstatus, 64, "EX_USAGE of sysexits(3) fame" );
ok( $exp->before =~ m/Usage/, "help mentions usage" );

sub eof_or_timeout {
    my ( $e, $timeout, $resultref ) = @_;
    $e->expect(
        $timeout,
        'eof'     => sub { $$resultref = 'eof' },
        'timeout' => sub { $$resultref = 'timeout' }
    );
}

sub newexpect {
    my $exp = Expect->new;
    $exp->raw_pty(1);
    #$exp->debug(3);
    return $exp;
}
