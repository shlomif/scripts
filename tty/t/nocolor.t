#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $cmd = Test::UnixCmdWrap->new(cmd => './nocolor-test');

# smaller buffer size for testing as that makes it easier to debug; NOTE
# that some tests below assume this length
sub NOCOLOR_BUF_SIZE () { 32 }

my $exact_buf    = "\e[31m";
my $pad_by       = NOCOLOR_BUF_SIZE - length $exact_buf;
my $expect_exact = "e" x $pad_by;
$exact_buf .= $expect_exact;

my $across_buf    = "a" x (NOCOLOR_BUF_SIZE - 1);
my $expect_across = $across_buf;
$across_buf    .= "\e[31m" . "b" x 5;
$expect_across .= "b" x 5;

my $across_buf2    = "a" x (NOCOLOR_BUF_SIZE - 18);
my $expect_across2 = $across_buf2;
$across_buf2    .= "\e[38:2:255:255:255m" . "b" x 5;
$expect_across2 .= "b" x 5;

$ENV{PERLIO} = ':raw';

$cmd->run(
    args   => qq{'$^X' -E 'say "no change"'},
    stdout => qr/^no change$/,
);
$cmd->run(
    args   => qq{'$^X' -E 'say "no \e[31mcolor"'},
    stdout => qr/^no color$/,
);
# various fencepost related edge cases
$cmd->run(
    args   => qq{'$^X' -E 'say "\e[32mleading"'},
    stdout => qr/^leading$/,
);
$cmd->run(
    args   => qq{'$^X' -E 'say "trailing\e[33m"'},
    stdout => qr/^trailing$/,
);
$cmd->run(
    args   => qq{'$^X' -E 'say "\e[37m"'},
    stdout => qr/^$/,
);
# multiple matches (/g)
$cmd->run(
    args   => qq{'$^X' -E 'say "in\e[31mter\e[31mlea\e[31mvea\e[31m"'},
    stdout => qr/^interleavea$/,
);
$cmd->run(
    args   => qq{'$^X' -E 'say "\e[31min\e[31mter\e[31mleav\e[31meb"'},
    stdout => qr/^interleaveb$/,
);
# did someone forget the one-or-more-times on the regex? (lacking
# this might lead to zero-sized writes which while inefficient may
# not otherwise be a problem and would be hard for a test to detect)
$cmd->run(
    args   => qq{'$^X' -E 'say "matching\e[31m\e[32m\e[33mspree"'},
    stdout => qr/^matchingspree$/,
);
# newlines may be treated specially by regex engines?
$cmd->run(
    args   => qq{'$^X' -E 'say "\e[33mone\ntwo\e[33m\nth\e[33mree"'},
    stdout => qr/^one\ntwo\nthree$/
);
# buffering is hard (probably needs to be more of these)
$cmd->run(
    args   => qq{'$^X' -E "print qq{$exact_buf}"},
    stdout => [$expect_exact],
);
$cmd->run(
    args   => qq{'$^X' -E "print qq{$exact_buf$exact_buf}"},
    stdout => ["$expect_exact$expect_exact"],
);
$cmd->run(
    args   => qq{'$^X' -E "say qq{$across_buf}"},
    stdout => qr/^$expect_across$/,
);
$cmd->run(
    args   => qq{'$^X' -E "say qq{$across_buf2}"},
    stdout => qr/^$expect_across2$/,
);
$cmd->run(
    args   => qq{'$^X' -E "say qq{$exact_buf$across_buf}"},
    stdout => qr/^$expect_exact$expect_across$/,
);
$cmd->run(
    args   => qq{'$^X' -E "say qq{$across_buf2$exact_buf}"},
    stdout => qr/^$expect_across2$expect_exact$/,
);
$cmd->run(
    args => qq{'$^X' -pe 1 t/nocolor.input1},
    stdout =>
      qr(^\e\[1;28r\e\[2;1H H - an uncursed ring of protection from fire\r\e\[3d J - an uncursed ring of magical power\e\(B\e\[m\e\[K\r\e\[4dWands          \(select all with /\)\e\(B\e\[m\e\[K\r\e\[5d i - a wand of flame \(30\)\r\e\[6d n - a wand of disintegration \(5\)\e\[K\r\e\[7d o - a wand of polymorph \(28\)\r\e\[8d A - a wand of paralysis \(25\)\r\e\[9d U - a wand of acid \(2\)$),
);
# exactly NOCOLOR_BUF_SIZE characters and with a (false) trailing
# potential escape sequence and also EOF
$cmd->run(
    args   => qq{'$^X' -e "print qq{1234\e[42m5678901234567890\e[3d123}"},
    stdout => ["12345678901234567890\e[3d123"],
);
# STDERR should get the same treatment (but since it is presently
# the same code path it is not tested as much)
$cmd->run(
    args   => qq{'$^X' -E 'say "o\e[40mut"; say STDERR "no \e[37mcolor"'},
    stdout => ["out"],
    stderr => qr/^no color$/,
);
$cmd->run(status => 64, stderr => qr/^Usage/);

# should any unescaped escaped escape codes leave things in a bad state
diag "set terminal to default...\e[0m";

done_testing(57);
