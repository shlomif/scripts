#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

delete $ENV{SSH_ORIGINAL_COMMAND};
my $soc = 'rsync --server --sender -vnlHogDtprRxSe.iLsfxC --numeric-ids . ';

my $test_prog = './validate-rsnapshot';

# most of the code however involves syslog errors (hard to test) and
# running specific rsync commands (also hard to test)
my @tests = (
    # no SSH_ORIGINAL_COMMAND
    {   args        => '-n',
        exit_status => 2,
    },
    # should not match. hopefully not run.
    {   args        => '-n',
        env         => { SSH_ORIGINAL_COMMAND => 'rm -rf /' },
        exit_status => 4,
    },
    # assumes "t" test directory exists and is accessible
    {   args        => '-n',
        env         => { SSH_ORIGINAL_COMMAND => $soc . 't' },
        exit_status => 0,
        stdout      => qr(^t$),
    },
    # usually naughty .. runs are not allowed (if they need to be
    # supported for some reason, fiddle with the code to allow that)
    {   args        => '-n',
        env         => { SSH_ORIGINAL_COMMAND => $soc . 'asdf/../../etc' },
        exit_status => 7,
    },
    # NOTE some vendors put files into this directory (??)
    {   args        => '-n',
        env         => { SSH_ORIGINAL_COMMAND => $soc . '/var/empty/' . $$ },
        exit_status => 5,
    },
    {   args        => '-h',
        env         => { SSH_ORIGINAL_COMMAND => $soc . '/' },
        exit_status => 64,
        stderr      => qr/supported/,
    },
);
my $testcmd = Test::Cmd->new(prog => $test_prog, workdir => '',);

for my $test (@tests) {
    $test->{exit_status} //= 0;
    $test->{stdout}      //= qr/^$/;
    $test->{stderr}      //= qr/^$/;

    # NOTE Test::Cmd uses the shell which in turn might add environment
    # variables depending on what rc contains
    local %ENV;
    @ENV{ keys %{ $test->{env} } } = values %{ $test->{env} };

    $testcmd->run(exists $test->{args} ? (args => $test->{args}) : ());

    my $args = $test->{args} // '';

    exit_is($?, $test->{exit_status}, "STATUS $test_prog $args");
    ok($testcmd->stdout =~ m/$test->{stdout}/, "STDOUT $test_prog $args")
      or diag 'STDOUT: ' . ($testcmd->{stdout} // '');
    ok($testcmd->stderr =~ m/$test->{stderr}/, "STDERR $test_prog $args")
      or diag 'STDERR: ' . ($testcmd->{stderr} // '');
}
done_testing(@tests * 3);
