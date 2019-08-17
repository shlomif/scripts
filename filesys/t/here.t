#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $cmd = Test::UnixCmdWrap->new;

# TODO better tests so don't need to warn and change things if it
# does differ
diag "these will fail if the repo is checked out differently";

$cmd->run(stdout => '^' . catfile(qw/co scripts filesys/) . '$');
$cmd->run(
    args   => 'subdir',
    stdout => '^' . catfile(qw/co scripts filesys subdir/) . '$'
);

done_testing(6);
