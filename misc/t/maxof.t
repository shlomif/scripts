#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $cmd    = Test::UnixCmdWrap->new;
my @inputs = qw(t/maxof-a t/maxof-b);

$cmd->run(
    stdin  => "3.14 pi\n2.72 e\n99 bottles of beer\n42 answers\n",
    stdout => ['99 bottles of beer']
);
$cmd->run(
    args   => "@inputs",
    stdout => ['99 bottles of beer']
);

done_testing(6);
