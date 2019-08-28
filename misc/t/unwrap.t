#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;
use File::Slurper qw(read_text);

my $cmd = Test::UnixCmdWrap->new;
my $msg = read_text('t/unwrap-msg');

# originally had -a for "all lines" but figure will mostly be using the
# tool as a filter in vi(1) so there will want a different flag to tell
# it not to molest the first paragraph
$cmd->run(
    args   => '-H t/unwrap-msg',
    stdout => [
        "From: bar", "Subject: foo", '', "some very short lines",
        '', "and some more"
    ],
);
$cmd->run(
    args   => 't/unwrap-msg',
    stdout => [
        "From: bar Subject: foo", '', "some very short lines", '', "and some more"
    ],
);
# must support POSIX ultimate newline, even should the input lack
$cmd->run(
    args   => '-H t/unwrap-noeofeol',
    stdout => [
        "From: bar", "Subject: foo", '', "some very short lines",
        '', "and some more"
    ],
);
# also should tidy up trailing newline spam
$cmd->run(
    args   => '-H t/unwrap-eofnlspam',
    stdout => [
        "From: bar", "Subject: foo", '', "some very short lines",
        '', "and some more"
    ],
);
$cmd->run(
    args   => "t/unwrap-no-such-file-$$",
    status => 74,
    stderr => qr/could not open/,
);
# stdin probably the main interface, as a vi(1) filter
$cmd->run(
    stdin  => $msg,
    stdout => [
        "From: bar Subject: foo", '', "some very short lines", '', "and some more"
    ],
);
$cmd->run(
    args   => '-',
    stdin  => $msg,
    stdout => [
        "From: bar Subject: foo", '', "some very short lines", '', "and some more"
    ],
);
# and leading whitespace needs to be folded into wrapped lines?
$cmd->run(
    args   => '-H t/unwrap-leadingws',
    stdin  => $msg,
    stdout => [
        "From: bar", "Subject: foo", "    zot", '', "    some short lines",
        '', "and some more"
    ],
);
# and trailing whitespace needs squashing too (this is also a not-POSIX
# input lacking the ultimate newline POSIX demands)
$cmd->run(
    stdin  => "foo  \t   \nbar   \t   \nzot    \t  ",
    stdout => qr/^foo bar zot$/,
);
# what happens for lines that are only whitespace?
# TODO probably need to mix these in with paragraphs of text
$cmd->run(
    stdin  => "    \t   \t\t  \n  \t  \t   \t\n",
    stdout => qr/^$/,
);

$cmd->run(
    args   => '-h',
    status => 64,
    stderr => qr/^Usage: /,
);
$cmd->run(
    args   => '-Q nosuch flag',
    status => 64,
    stderr => qr/Usage: /,
);

done_testing(36);
