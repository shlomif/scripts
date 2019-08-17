#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $cmd = Test::UnixCmdWrap->new;

$cmd->run(
    stdin  => "1. a\n1. b\n",
    stdout => [ '1. a', '1. b' ],
);
$cmd->run(
    args   => q{'all=>1'},
    stdin  => "1. a\n1. b\n",
    stdout => [ '1. a', '2. b' ],
);

done_testing(6);
