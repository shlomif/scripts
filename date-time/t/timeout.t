#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;
use Time::HiRes qw(gettimeofday tv_interval);

my $cmd = Test::UnixCmdWrap->new;

# TODO see discussion in snooze.t
my $tolerance = 0.15;

diag "timeout tests will take time...";

sub taking :prototype($&) {
    my ($duration, $function) = @_;
    my $start = [gettimeofday];
    $function->();
    my $elapsed_error = abs(tv_interval($start) - $duration) / $duration;
    ok($elapsed_error < $tolerance,
        "duration variance out of bounds: $elapsed_error vs $duration");
}

taking 3, sub { $cmd->run(args => '7 sleep 3') };

taking 4, sub {
    $cmd->run(
        args   => '4 sleep 7',
        stderr => qr/^timeout: duration 4 exceeded/,
        status => 2,
    );
};

taking 2, sub { $cmd->run(args => '3m sleep 2') };

$cmd->run(args => '-h', status => 64, stderr => qr/Usage/);

done_testing(15);
