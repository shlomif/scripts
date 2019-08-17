#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $cmd = Test::UnixCmdWrap->new;

my $test_file = 't/llcount-input';

open my $tfh, '<', $test_file or BAIL_OUT("could not open '$test_file': $!");
my $test_lines = do { local $/; readline $tfh };

$cmd->run(
    args   => "'$test_file'",
    stdout => [
        '  1     0   16 The quick brown',
        '  2    16    4 fox',
        '  3    20    7 jumped'
    ],
);
$cmd->run(
    args   => "-x '$test_file'",
    stdout => [
        '  1     0   16 The quick brown',
        '  2  0x10    4 fox',
        '  3  0x14    7 jumped'
    ],
);
$cmd->run(
    args   => '-',
    stdin  => $test_lines,
    stdout => [
        '  1     0   16 The quick brown',
        '  2    16    4 fox',
        '  3    20    7 jumped'
    ],
);
$cmd->run(args => '-h', status => 64, stderr => qr/Usage/);

done_testing(12);
