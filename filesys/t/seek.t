#!perl

use 5.14.0;
use warnings;
use File::Spec ();
use File::Temp qw(tempdir);
use Test::Cmd;
use Test::Most;
use Test::UnixExit;

my $test_prog = './seek';

# there are two different code paths for seekable (a file) or not (from
# stdin) so there are duplicate tests on reading from this file, or the
# contents of this file being passed by stdin
open my $fh, '<', 't/seek-nopnopnop' or die "could not open 't/seek-nopnopnop': $!\n";
binmode $fh, ':raw';
my $nopnopnop = do { local $/; readline $fh };
close $fh;

open $fh, '<', 't/seek-bufsize' or die "could not open 't/seek-bufsize': $!\n";
my $bigbufsize = do { local $/; readline $fh };
close $fh;

my $test_dir = tempdir( 'seek.XXXXXXXXXX', CLEANUP => 1, TMPDIR => 1 );
my $file_f = File::Spec->catfile( $test_dir, "F" );

my @tests = (
    {   args   => '82 t/seek-nopnopnop',
        stdout => "\x90\x90\x90",
    },
    {   args   => '0x53 t/seek-nopnopnop',
        stdout => "\x90\x90",
    },
    # -m or "maximum" or "at most"
    {   args   => '-m 3 1 t/seek-nopnopnop',
        stdout => "ELF",
    },
    # it is not a error if you want more than is in the file
    {   args   => '-m 9999 0x54',
        stdin  => $nopnopnop,
        stdout => "\x90",
    },
    # but it is if the seek is beyond the size of the file
    {   args        => '86 t/seek-nopnopnop',
        exit_status => 74,
        stdout      => "",
        stderr      => qr/could not seek/,
    },
    {   args        => '86',
        exit_status => 74,
        stdin       => $nopnopnop,
        stdout      => "",
        stderr      => qr/could not seek/,
    },
    # exactly to EOF is not an error (it perhaps should be but the stdin
    # code path would need too much extra code to handle this edge case)
    {   args        => '0x55 t/seek-nopnopnop',
        stdout      => "",
    },
    {   args        => '0x55',
        stdin       => $nopnopnop,
        stdout      => "",
    },
    # another way to request input from stdin
    {   args   => '83 -',
        stdin  => $nopnopnop,
        stdout => "\x90\x90",
    },
    # output to stdout, the verbose way
    {   args   => '-m 2 -o - 2 -',
        stdin  => $nopnopnop,
        stdout => "LF",
    },
    # and to a file (the contents of which are checked later)
    {   args   => "-m 1 -o '$file_f' 3 t/seek-nopnopnop",
        stdout => "",
    },
    # test reads across BUFSIZE 
    {   args   => '8190 t/seek-bufsize',
        stdout => "bcde",
    },
    {   args   => '8190',
        stdin => $bigbufsize,
        stdout => "bcde",
    },
    {   args   => '-m 3 8190 t/seek-bufsize',
        stdout => "bcd",
    },
    {   args   => '-m 3 8190',
        stdin => $bigbufsize,
        stdout => "bcd",
    },
    {   args   => '0 t/seek-bufsize',
        stdout => $bigbufsize,
    },
    # additional layer of quoting necessary as otherwise Test::Cmd 1.09
    # removes the bare 0 from the argument
    {   args   => "'0'",
        stdin => $bigbufsize,
        stdout => $bigbufsize,
    },
    # help and other argument list sanity checks
    {   args   => '-h',
        exit_status => 64,
        stdout => "",
        stderr => qr/Usage/,
    },
    {   exit_status => 64,
        stdout => "",
        stderr => qr/Usage/,
    },
    {   args => "bla blah blah bla blah blah blah",
        exit_status => 64,
        stdout => "",
        stderr => qr/Usage/,
    },
    # lojban numbers (also here toki pona) are not valid; need a
    # strtoul(3) compatible value
    {   args => "mu t/seek-nopnopnop",
        exit_status => 65,
        stdout => "",
        stderr => qr/seek value/,
    },
    # negative seeks don't make a lick of sense so should fail
    {   args => "-- -42 t/seek-nopnopnop",
        exit_status => 65,
        stdout => "",
        stderr => qr/seek value/,
    },
    # this tests how goptfoo handles negative values (-42 can turn into
    # 18446744073709551574 without guards for that)
    {   args => "-m -42 0 t/seek-nopnopnop",
        exit_status => 65,
        stdout => "",
        stderr => qr/seek: /,
    },
);
my $test_count = 3 * @tests;

my $testcmd = Test::Cmd->new(
    prog    => $test_prog,
    verbose => 0,
    workdir => '',
);

for my $test (@tests) {
    $test->{exit_status} //= 0;
    $test->{stderr}      //= qr/^$/;

    $testcmd->run(
        exists $test->{args} ? ( args => $test->{args} ) : (),
        exists $test->{stdin} ? ( stdin => $test->{stdin} ) : ()
    );

    $test->{args} //= '';

    exit_is( $?, $test->{exit_status}, "STATUS $test_prog $test->{args}" );
    is( $testcmd->stdout, $test->{stdout}, "STDOUT $test_prog $test->{args}" );
    ok( $testcmd->stderr =~ $test->{stderr}, "STDERR $test_prog $test->{args}" )
      or diag "STDERR >" . $testcmd->stderr . "<";
}

# did -o create the expected output file?
{
    my $fh;
    lives_ok { open $fh, '<', $file_f or die "could not open '$file_f': $!\n" };
    my $just_a_f = do { local $/; readline $fh };

    is( $just_a_f, "F" );

    # features, of course, complicate testing as now we must append
    $testcmd->run( args => "-a -m 1 -o '$file_f' 3 t/seek-nopnopnop" );

    exit_is( $?, 0 );
    is( $testcmd->stdout, "" );
    is( $testcmd->stderr, "" );

    seek( $fh, 0, 0 );
    $just_a_f = do { local $/; readline $fh };

    is( $just_a_f, "FF" );

    # and now that the truncate truncates
    $testcmd->run( args => "-m 1 -o '$file_f' 3 t/seek-nopnopnop" );

    lives_ok { open $fh, '<', $file_f or die "could not open '$file_f': $!\n" };
    $just_a_f = do { local $/; readline $fh };

    is( $just_a_f, "F" );

    $test_count += 8;
}

done_testing( $test_count );
