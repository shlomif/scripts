#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $cmd      = Test::UnixCmdWrap->new;
my $test_dir = tempdir('rj.XXXXXXXXXX', CLEANUP => 1, TMPDIR => 1);

# KLUGE tests assume there is no whitespace in the file path; TODO
# would need to use NUL to delimit the file path in the output to
# make the output better parsable in such a case
ok $test_dir !~ m/\s/ or BAIL_OUT("whitespace in directory tree");

# TODO test without --keep-args and have the program write here to
# help confirm it was run
#my $output = catfile($test_dir, 'out');

# TODO that more than max-queue jobs get run

# TODO that jobs get run in parallel -- maybe record exit time and
# see if all got done from a starting point (oops don't have a prejob cmd)

# TODO that postjob gets run

my $rand_max_errors = 3 + int rand 3;

$cmd->run(
    args   => "--no-such-flag-$$",
    status => 64,
    stderr => qr/^./,
);
my $o = $cmd->run(
    args   => '--keep-logs',
    stdin  => "$^X -E 'say $$'",
    stderr => qr/start pid=(\d+).*end pid=\1/as,
);
post_checkout($o);
$o = $cmd->run(
    stdin  => "$^X -E 'say $$; exit 42'",
    status => 1,
    stderr => qr/drop too many errors/,
);
postgen_errors()->($o);
$o = $cmd->run(
    args   => "--max-errors=$rand_max_errors",
    stdin  => "$^X -E 'say $$; exit 43'",
    status => 1,
    stderr => qr/drop too many errors/,
);
postgen_errors($rand_max_errors)->($o);

# expect two lines matching canary value in output files: the first from
# the command line that runjobs puts at the top of the output file, and
# a second from the running of the job itself
sub check_output {
    my ($file, $canary) = @_;
    open my $fh, '<', $file;
    ok defined $fh, "can read $file";
    my $count = 0;
    while (readline $fh) {
        $count++ if m/\Q$canary/;
    }
    is $count, 2;
}

sub post_checkout {
    my ($o)    = @_;
    my $err    = $o->stderr;
    my ($file) = $err =~ m/pid=\d+ (\S+)/;
    check_output($file, $$);
}

sub postgen_errors {
    my ($maxerrors) = @_;
    # default Flag_MaxErrs before runjobs gives up on the job
    $maxerrors //= 2;
    sub {
        my ($o)   = @_;
        my $err   = $o->stderr;
        my $files = 0;
        while ($err =~ m/^runjobs: start pid=\d+ (\S+)/agm) {
            $files++;
            check_output($1, $$);
        }
        is $files, $maxerrors;
    };
}

done_testing(31);
