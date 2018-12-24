#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $test_prog = './procrust';

my @tests = (
    {   args   => "4",
        stdin  => "asdf\nqwer\nzxcv\n",
        stdout => [qw(asdf qwer zxcv)],
    },
    {   args   => "3",
        stdin  => "asdf\nqwerty\nzxcvbnm\nuio\n",
        stdout => [qw(asd qwe zxc uio)],
    },
    {   args   => "5",
        stdin  => "a\nab\nabcde\nqwerty\n",
        stdout => [ "a    ", "ab   ", "abcde", "qwert" ],
    },
    {   args   => "-f q 3",
        stdin  => "\n1\n12\n123\n",
        stdout => [qw(qqq 1qq 12q 123)],
    },
    # this might be considered a bug by some (it might instead throw an
    # error) though do want some way to fill with NUL...
    {   args   => "-f '' 3",
        stdin  => "1\n12\n",
        stdout => [ "1\0\0", "12\0" ],
    },
    {   args   => "5 t/procrust.input t/procrust.input",
        stdout => [ "123  ", "12345", "12345", "123  ", "12345", "12345" ],
    },
);
my $testcmd = Test::Cmd->new( prog => $test_prog, workdir => '' );

for my $test (@tests) {
    $test->{exit_status} //= 0;
    $test->{stderr}      //= '';
    $testcmd->run(
        args => $test->{args},
        exists $test->{stdin} ? ( stdin => $test->{stdin} ) : ()
    );
    exit_is( $?, $test->{exit_status}, "STATUS $test_prog $test->{args}" );
    eq_or_diff( [ split $/, $testcmd->stdout ],
        $test->{stdout}, "STDOUT $test_prog $test->{args}" );
    is( $testcmd->stderr, $test->{stderr}, "STDERR $test_prog $test->{args}" );
}
$testcmd->run( args => '-h' );
exit_is( $?, 64, "EX_USAGE of sysexits(3) fame" );
ok( $testcmd->stderr =~ m/Usage/, "help mentions usage" );
$testcmd->run;
exit_is( $?, 64, "EX_USAGE of sysexits(3) fame" );
ok( $testcmd->stderr =~ m/Usage/, "help mentions usage" );
done_testing( @tests * 3 + 4 );
