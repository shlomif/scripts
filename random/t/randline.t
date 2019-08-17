#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $cmd  = Test::UnixCmdWrap->new;
my $tcmd = $cmd->cmd;

# these are basic interface tests; more than one input line requires
# statistical guesswork on account of the (hopefully) random nature of
# the line selection
$cmd->run(
    stdin  => "one\n",
    stdout => "one\n",
);
$cmd->run(
    stdin  => "nonl",
    stdout => "nonl\n",
);
$cmd->run(
    args   => "t/randline-one",
    stdout => "one\n",
);
$cmd->run(
    args   => "t/randline-empty t/randline-one",
    stdout => "one\n",
);
# no input is an error, or at least non-zero exit status
$cmd->run(status => 1);
$cmd->run(args   => '-h', status => 64, stderr => qr/^Usage: /);

# this may false alarm if the RNG does not pick one of the choices in
# the given number of trials (versus wasting more CPU time...)
# TODO really do need more extensive statistical tests, perhaps by
# setting a special ENV variable
my %seen;
for (1 .. 30) {
    $tcmd->run(stdin => "a\nb\nc\n");
    chomp(my $out = $tcmd->stdout);
    $seen{$out}++;
}
eq_or_diff([ sort keys %seen ], [qw/a b c/]);

done_testing(19);
