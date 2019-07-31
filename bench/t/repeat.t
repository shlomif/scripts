#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $test_prog = './repeat';

my $few = 3 + int rand 3;

my @tests = (
    {   stderr      => qr/^Usage/,
        exit_status => 64,
    },
    {   args        => 'tribbles',
        stderr      => qr/^Usage/,
        exit_status => 64,
    },
    {   args        => '42',
        stderr      => qr/^Usage/,
        exit_status => 64,
    },
    {   args   => "$few echo tribbles",
        stdout => qr/^(?s)(?:tribbles.){$few}/,
    },
    # $N is made available to the command with the current count number
    {   args   => "$few sh -c 'echo tribble \$N'",
        stdout => qr/^(?sa)(?:tribble \d+.){$few}/,
    },
);
my $cmd = Test::Cmd->new(prog => $test_prog, workdir => '',);

for my $test (@tests) {
    $test->{exit_status} //= 0;
    $test->{stdout}      //= qr/^$/;
    $test->{stderr}      //= qr/^$/;

    $cmd->run(exists $test->{args} ? (args => $test->{args}) : ());

    $test->{args} //= '';
    exit_is($?, $test->{exit_status}, "STATUS $test_prog $test->{args}");
    ok($cmd->stdout =~ m/$test->{stdout}/, "STDOUT $test_prog $test->{args}")
      or diag 'STDOUT ' . $cmd->stdout;
    ok($cmd->stderr =~ m/$test->{stderr}/, "STDERR $test_prog $test->{args}")
      or diag 'STDERR ' . $cmd->stderr;
}
done_testing(@tests * 3);
