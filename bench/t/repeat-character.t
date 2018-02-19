#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $test_prog = './repeat-character';

# TODO test that e.g. full buffer actually buffers fully, etc
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
my $testcmd = Test::Cmd->new( prog => $test_prog, workdir => '', );

for my $test (@tests) {
    $test->{exit_status} //= 0;
    $test->{stdout}      //= '';
    $test->{stderr}      //= qr/^$/;

    $testcmd->run( exists $test->{args} ? ( args => $test->{args} ) : (), );

    $test->{args} //= '';
    exit_is( $?, $test->{exit_status}, "STATUS $test_prog $test->{args}" );
    is( $testcmd->stdout, $test->{stdout}, "STDOUT $test_prog $test->{args}" );
    ok( $testcmd->stderr =~ m/$test->{stderr}/, "STDERR $test_prog $test->{args}" );
}
done_testing( @tests * 3 );
