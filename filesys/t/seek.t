#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $cmd = Test::UnixCmdWrap->new;

# there are two different code paths for seekable (a file) or not (from
# stdin) so there are duplicate tests on reading from this file, or the
# contents of this file being passed by stdin
open my $fh, '<', 't/seek-nopnopnop'
  or die "could not open 't/seek-nopnopnop': $!\n";
binmode $fh, ':raw';
my $nopnopnop = do { local $/; readline $fh };
close $fh;

open $fh, '<', 't/seek-bufsize' or die "could not open 't/seek-bufsize': $!\n";
my $bigbufsize = do { local $/; readline $fh };
close $fh;

my $test_dir = tempdir('seek.XXXXXXXXXX', CLEANUP => 1, TMPDIR => 1);
my $file_f   = catfile($test_dir, 'F');

$cmd->run(
    args   => '82 t/seek-nopnopnop',
    stdout => "\x90\x90\x90",
);
$cmd->run(
    args   => '0x53 t/seek-nopnopnop',
    stdout => "\x90\x90",
);
# -m or "maximum" or "at most"
$cmd->run(
    args   => '-m 3 1 t/seek-nopnopnop',
    stdout => "ELF",
);
# it is not a error if you want more than is in the file
$cmd->run(
    args   => '-m 9999 0x54',
    stdin  => $nopnopnop,
    stdout => "\x90",
);
# but it is if the seek is beyond the size of the file
$cmd->run(
    args   => '86 t/seek-nopnopnop',
    status => 74,
    stdout => "",
    stderr => qr/could not seek/,
);
$cmd->run(
    args   => '86',
    status => 74,
    stdin  => $nopnopnop,
    stdout => "",
    stderr => qr/could not seek/,
);
# exactly to EOF is not an error (it perhaps should be but the stdin
# code path would need too much extra code to handle this edge case)
$cmd->run(
    args   => '0x55 t/seek-nopnopnop',
    stdout => "",
);
$cmd->run(
    args   => '0x55',
    stdin  => $nopnopnop,
    stdout => "",
);
# another way to request input from stdin
$cmd->run(
    args   => '83 -',
    stdin  => $nopnopnop,
    stdout => "\x90\x90",
);
# output to stdout, the verbose way
$cmd->run(
    args   => '-m 2 -o - 2 -',
    stdin  => $nopnopnop,
    stdout => "LF",
);
# and to a file (the contents of which are checked later)
$cmd->run(
    args   => "-m 1 -o '$file_f' 3 t/seek-nopnopnop",
    stdout => "",
);
# test reads across BUFSIZE
$cmd->run(
    args   => '8190 t/seek-bufsize',
    stdout => "bcde",
);
$cmd->run(
    args   => '8190',
    stdin  => $bigbufsize,
    stdout => "bcde",
);
$cmd->run(
    args   => '-m 3 8190 t/seek-bufsize',
    stdout => "bcd",
);
$cmd->run(
    args   => '-m 3 8190',
    stdin  => $bigbufsize,
    stdout => "bcd",
);
$cmd->run(
    args   => '0 t/seek-bufsize',
    stdout => $bigbufsize,
);
# additional layer of quoting necessary as otherwise Test::Cmd 1.09
# removes the bare 0 from the argument
$cmd->run(
    args   => "'0'",
    stdin  => $bigbufsize,
    stdout => $bigbufsize,
);
# help and other argument list sanity checks
$cmd->run(
    args   => '-h',
    status => 64,
    stdout => "",
    stderr => qr/Usage/,
);
$cmd->run(
    status => 64,
    stdout => "",
    stderr => qr/Usage/,
);
$cmd->run(
    args   => "bla blah blah bla blah blah blah",
    status => 64,
    stdout => "",
    stderr => qr/Usage/,
);
# lojban numbers (also here toki pona) are not valid; need a
# strtoul(3) compatible value
$cmd->run(
    args   => "mu t/seek-nopnopnop",
    status => 65,
    stdout => "",
    stderr => qr/seek value/,
);
# negative seeks don't make a lick of sense so should fail
$cmd->run(
    args   => "-- -42 t/seek-nopnopnop",
    status => 65,
    stdout => "",
    stderr => qr/seek value/,
);
# this tests how goptfoo handles negative values (-42 can turn into
# 18446744073709551574 without guards for that)
$cmd->run(
    args   => "-m -42 0 t/seek-nopnopnop",
    status => 65,
    stdout => "",
    stderr => qr/seek: /,
);

# did -o create the expected output file?
{
    my $fh;
    lives_ok { open $fh, '<', $file_f or die "could not open '$file_f': $!\n" };
    my $just_a_f = do { local $/; readline $fh };

    is($just_a_f, "F");

    # features, of course, complicate testing as now we must append
    $cmd->run(args => "-a -m 1 -o '$file_f' 3 t/seek-nopnopnop");

    seek($fh, 0, 0);
    $just_a_f = do { local $/; readline $fh };

    is($just_a_f, "FF");

    # and now that the truncate truncates
    $cmd->run(args => "-m 1 -o '$file_f' 3 t/seek-nopnopnop");

    lives_ok { open $fh, '<', $file_f or die "could not open '$file_f': $!\n" };
    $just_a_f = do { local $/; readline $fh };

    is($just_a_f, "F");
}

done_testing(80);
