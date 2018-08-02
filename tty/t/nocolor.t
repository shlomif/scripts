#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

# keep this in sync with the code
sub NOCOLOR_BUF_SIZE () { 1024 }

my $exact_buf    = "\e[31m";
my $pad_by       = NOCOLOR_BUF_SIZE - length $exact_buf;
my $expect_exact = "e" x $pad_by;
$exact_buf .= $expect_exact;

my $across_buf = "a" x ( NOCOLOR_BUF_SIZE - 1 );
my $expect_across = $across_buf;
$across_buf .= "\e[31m" . "b" x 5;
$expect_across .= "b" x 5;

my $across_buf2 = "a" x ( NOCOLOR_BUF_SIZE - 18 );
my $expect_across2 = $across_buf2;
$across_buf2 .= "\e[38:2:255:255:255m" . "b" x 5;
$expect_across2 .= "b" x 5;

my $test_prog = './nocolor';

my @tests = (
    {   exit_status => 64,
        stderr      => qr/^Usage: nocolor/,
        stdout      => "",
    },
    {   args   => qq{'$^X' -E 'say "no change"'},
        stdout => "no change\n",
    },
    {   args   => qq{'$^X' -E 'say "no \e[31mcolor"'},
        stdout => "no color\n",
    },
    # various fencepost related edge cases
    {   args   => qq{'$^X' -E 'say "\e[32mleading"'},
        stdout => "leading\n",
    },
    {   args   => qq{'$^X' -E 'say "trailing\e[33m"'},
        stdout => "trailing\n",
    },
    {   args   => qq{'$^X' -E 'say "\e[37m"'},
        stdout => "\n",
    },
    # multiple matches (/g)
    {   args   => qq{'$^X' -E 'say "in\e[31mter\e[31mlea\e[31mvea\e[31m"'},
        stdout => "interleavea\n",
    },
    {   args   => qq{'$^X' -E 'say "\e[31min\e[31mter\e[31mleav\e[31meb"'},
        stdout => "interleaveb\n",
    },
    # did someone forget the one-or-more-times on the regex? (lacking
    # this might lead to zero-sized writes which while inefficient may
    # not otherwise be a problem and would be hard for a test to detect)
    {   args   => qq{'$^X' -E 'say "matching\e[31m\e[32m\e[33mspree"'},
        stdout => "matchingspree\n",
    },
    # newlines may be treated specially by regex engines?
    {   args   => qq{'$^X' -E 'say "\e[33mone\ntwo\e[33m\nth\e[33mree"'},
        stdout => "one\ntwo\nthree\n",
    },
    # buffering is hard (probably needs to be more of these)
    {   args   => qq{'$^X' -E "print qq{$exact_buf}"},
        stdout => $expect_exact,
    },
    {   args   => qq{'$^X' -E "print qq{$exact_buf$exact_buf}"},
        stdout => "$expect_exact$expect_exact",
    },
    {   args   => qq{'$^X' -E "say qq{$across_buf}"},
        stdout => "$expect_across\n",
    },
    {   args   => qq{'$^X' -E "say qq{$across_buf2}"},
        stdout => "$expect_across2\n",
    },
    {   args   => qq{'$^X' -E "say qq{$exact_buf$across_buf}"},
        stdout => "$expect_exact$expect_across\n",
    },
    {   args   => qq{'$^X' -E "say qq{$across_buf2$exact_buf}"},
        stdout => "$expect_across2$expect_exact\n",
    },
    # STDERR should get the same treatment (but since it is presently
    # the same code path it is not tested as much)
    {   args   => qq{'$^X' -E 'say STDERR "no \e[37mcolor"'},
        stdout => "",
        stderr => qr/^no color$/,
    },
);
my $command = Test::Cmd->new( prog => $test_prog, workdir => '', );
my $test_count = 3 * @tests;

for my $test (@tests) {
    $test->{exit_status} //= 0;
    $test->{stderr}      //= qr/^$/;

    $command->run( exists $test->{args} ? ( args => $test->{args} ) : () );

    my $args = $test->{args} // '';

    exit_is( $?, $test->{exit_status}, "STATUS $test_prog $args" );

    # TODO is there a Test::* with hexdump-of-the-differences?
    my $out = $command->stdout;
    ( my $clean_out = $out ) =~ s/[^\x20-\x7e]/./g;
    ( my $want      = $test->{stdout} ) =~ s/[^\x20-\x7e]/./g;
    ok( $out eq $test->{stdout},
        sprintf(
            "$test_prog $args\nGOT  %vx $clean_out\nWANT %vx $want\n",
            $out, $test->{stdout}
        )
    );

    chomp( my $err = $command->stderr );
    ok( $err =~ m/$test->{stderr}/, "STDERR $test_prog $args" )
      or diag "WANT $test->{stderr} GOT '$err'";
}
# should any unescaped escaped escape codes leave things in a bad state
diag "set terminal to default...\e[0m";
done_testing($test_count);
