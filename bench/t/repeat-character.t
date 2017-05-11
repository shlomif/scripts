#!perl

# TODO test that e.g. full buffer actually buffers fully, etc

use 5.14.0;
use warnings;
use Test::Cmd;
# 3 tests per item in @tests plus any extras
use Test::Most tests => 3 * 6 + 0;
use Test::UnixExit;

my $test_prog = './repeat-character';

my @tests = (
    {   stderr      => qr/^Usage/,
        exit_status => 64,
    },
    {   args        => q{hello computer could you repeat characters for me},
        stderr      => qr/^Usage/,
        exit_status => 64,
    },
    {   args        => q{e 3 7 chrome},
        stderr      => qr/^Usage/,
        exit_status => 64,
    },
    {   args   => q{x 3 7 none},
        stdout => 'xxxxxxxxxxxxxxxxxxxxx',
    },
    {   args   => q{x 3 7 full},
        stdout => 'xxxxxxxxxxxxxxxxxxxxx',
    },
    # six characters only because we count one to the newline so that
    # "7 3" generates 21 characters regardless of the buffer style
    {   args   => q{l 7 3 line},
        stdout => "llllll\nllllll\nllllll\n",
    },
);

my $testcmd = Test::Cmd->new(
    prog    => $test_prog,
    verbose => 0,
    workdir => '',
);

for my $test (@tests) {
    $test->{exit_status} //= 0;
    $test->{stdout}      //= '';
    $test->{stderr}      //= qr/^$/;

    $testcmd->run(
        exists $test->{args}  ? ( args  => $test->{args} )  : (),
        exists $test->{chdir} ? ( chdir => $test->{chdir} ) : ()
    );

    $test->{args} //= '';
    exit_is( $?, $test->{exit_status}, "STATUS $test_prog $test->{args}" );
    is( $testcmd->stdout, $test->{stdout}, "STDOUT $test_prog $test->{args}" );
    ok( $testcmd->stderr =~ m/$test->{stderr}/, "STDERR $test_prog $test->{args}" );
}
