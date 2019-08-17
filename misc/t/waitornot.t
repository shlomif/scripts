#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;
use Expect;
use Time::HiRes qw(gettimeofday tv_interval);

my $cmd       = Test::UnixCmdWrap->new;
my $test_prog = $cmd->prog;

# NOTE see TODOs elsewhere (snooze.t)
my $tolerance = 0.15;

$cmd->run(status => 64,   stderr => qr/Usage/);
$cmd->run(args   => "-h", status => 64, stderr => qr/Usage/);

# no TTY - should fail (and quickly (unless the system is busy, or a
# SIGSTOP/CONT is used on the process, etc))
my $start = [gettimeofday];
$cmd->run(
    args   => "sleep 7",
    stdin  => "any key",
    status => 1,
    stderr => qr/(?i)tty/
);
ok(tv_interval($start) < 3);

diag "tests will take some time...";

my $eoft_result;    # set by eof_or_timeout depending on result of that

# full 3 second run with some standard out from the program
$start = [gettimeofday];

my $exp = newexpect();
ok($exp->spawn($test_prog, $^X, '-E', "say qq{out $$}; sleep 3"),
    "expect object spawned");
eof_or_timeout($exp, 7, \$eoft_result);
is($eoft_result, 'eof',      "$test_prog exited");
is($exp->before, "out $$\n", "passthrough of stdout");
exit_is($exp->exitstatus, 0);

my $elapsed_error = abs(tv_interval($start) - 3) / 3;
ok($elapsed_error < $tolerance,
    "duration variance out of bounds: $elapsed_error");

# and this time with an interrupt - should run for about a second
$exp   = newexpect();
$start = [gettimeofday];

ok($exp->spawn($test_prog, qw(sleep 7)), "expect object spawned");
sleep 1;
$exp->send("a");
eof_or_timeout($exp, 11, \$eoft_result);
is($eoft_result, 'eof', "$test_prog exited");
is($exp->before, "",    "no output");
exit_is($exp->exitstatus, 0, "default exit for user interrupt");

$elapsed_error = abs(tv_interval($start) - 1) / 1;
ok($elapsed_error < $tolerance,
    "duration variance out of bounds: $elapsed_error");

$elapsed_error = abs(tv_interval($start) - 1) / 1;
ok($elapsed_error < $tolerance,
    "duration variance out of bounds: $elapsed_error");

# ISIG handling, default case: ^C should cause a signal-numbered exit
$exp = newexpect();
ok($exp->spawn($test_prog, qw(sleep 7)), "expect object spawned");
sleep 1;
$exp->send("\003");
eof_or_timeout($exp, 3, \$eoft_result);
is($eoft_result, 'eof', "$test_prog exited");
exit_is($exp->exitstatus, { signal => 2 }, "SIGINT");

# ISIG disabled; ^C should now be the same as (almost) any other key
$exp = newexpect();
ok($exp->spawn($test_prog, qw(-I sleep 7)), "expect object spawned");
sleep 1;
$exp->send("\003");
eof_or_timeout($exp, 3, \$eoft_result);
is($eoft_result, 'eof', "$test_prog exited");
exit_is($exp->exitstatus, 0, "default exit for user interrupt");

done_testing(27);

sub eof_or_timeout {
    my ($e, $timeout, $resultref) = @_;
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
