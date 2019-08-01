#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;
use File::Slurper qw(read_text);

my $test_prog = './unwrap';
my $msg       = read_text('t/unwrap-msg');

my @tests = (
    {   args   => 't/unwrap-msg',
        stdout => [
            "From: bar", "Subject: foo", '', "some very short lines",
            '', "and some more"
        ],
    },
    # -a applies to all lines, so molests what is expected to be a
    # leading email header block
    {   args   => '-a t/unwrap-msg',
        stdout => [
            "From: bar Subject: foo", '', "some very short lines",
	    '', "and some more"
        ],
    },
    # must support POSIX ultimate newline, even should the input lack
    {   args   => 't/unwrap-noeofeol',
        stdout => [
            "From: bar", "Subject: foo", '', "some very short lines",
            '', "and some more"
        ],
    },
    # also should tidy up trailing newline spam
    {   args   => 't/unwrap-eofnlspam',
        stdout => [
            "From: bar", "Subject: foo", '', "some very short lines",
            '', "and some more"
        ],
    },
    {   args   => "t/unwrap-no-such-file-$$",
        status => 74,
        stderr => qr/could not open/,
    },
    # stdin probably the main interface, as a vi(1) filter
    {   stdin  => $msg,
        stdout => [
            "From: bar", "Subject: foo", '', "some very short lines",
            '', "and some more"
        ],
    },
    {   args   => '-',
        stdin  => $msg,
        stdout => [
            "From: bar", "Subject: foo", '', "some very short lines",
            '', "and some more"
        ],
    },
    # and leading whitespace needs to be folded into wrapped lines?
    {   args   => 't/unwrap-leadingws',
        stdin  => $msg,
        stdout => [
            "From: bar", "Subject: foo", "    zot", '',
	    "    some short lines", '', "and some more"
        ],
    },
    {   args   => '-h',
        status => 64,
        stderr => qr/^Usage: /,
    },
    {   args   => '-Q nosuch flag',
        status => 64,
        stderr => qr/Usage: /,
    },
);
my $cmd = Test::Cmd->new(prog => $test_prog, workdir => '');

for my $test (@tests) {
    $test->{status} //= 0;
    $test->{stdout} //= [];
    $test->{stderr} //= qr/^$/;
    $cmd->run(
        exists $test->{args}  ? (args  => $test->{args})  : (),
        exists $test->{stdin} ? (stdin => $test->{stdin}) : ()
    );
    $test->{args} //= '';
    exit_is($?, $test->{status}, "STATUS $test_prog $test->{args}");
    eq_or_diff([ map { s/\s+$//r } split $/, $cmd->stdout ],
        $test->{stdout}, "STDOUT $test_prog $test->{args}");
    ok($cmd->stderr =~ $test->{stderr}, "STDERR $test_prog $test->{args}")
      or diag 'STDERR >' . $cmd->stderr . '<';
}
done_testing(@tests * 3);
