#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $cmd = Test::UnixCmdWrap->new;

# hopefully bigger than any typical buffer used by who knows what
my $bfs = join '', map { chr(65 + rand(26)) } 1 .. (16411 + rand 1031);

$cmd->run(
    args   => "cat foo '$$' bar '$$' zot '$$' asdf '$$' qwer '$$' zxcv '$$' x y z",
    stdout => ["foo $$ bar $$ zot $$ asdf $$ qwer $$ zxcv $$ x y z"],
);
$cmd->run(
    args   => "cat '$bfs'",
    stdout => [$bfs],
);
$cmd->run(
    args   => "cat foo '$bfs' bar '$bfs'",
    stdout => ["foo $bfs bar $bfs"],
);

$cmd->run(args => '-h',  status => 64, stderr => qr/Usage/);
$cmd->run(args => 'foo', status => 64, stderr => qr/Usage/);

done_testing(15);
