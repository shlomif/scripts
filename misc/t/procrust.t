#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $cmd = Test::UnixCmdWrap->new;

$cmd->run(
    args   => "4",
    stdin  => "asdf\nqwer\nzxcv\n",
    stdout => [qw(asdf qwer zxcv)],
);
$cmd->run(
    args   => "3",
    stdin  => "asdf\nqwerty\nzxcvbnm\nuio\n",
    stdout => [qw(asd qwe zxc uio)],
);
$cmd->run(
    args   => "5",
    stdin  => "a\nab\nabcde\nqwerty\n",
    stdout => qr/(?s)^a    .ab   .abcde.qwert$/,
);
$cmd->run(
    args   => "-f q 3",
    stdin  => "\n1\n12\n123\n",
    stdout => [qw(qqq 1qq 12q 123)],
);
# this might be considered a bug by some (it might instead throw an
# error) though do want some way to fill with NUL...
$cmd->run(
    args   => "-f '' 3",
    stdin  => "1\n12\n",
    stdout => [ "1\0\0", "12\0" ],
);
$cmd->run(
    args   => "5 t/procrust.input t/procrust.input",
    stdout => qr/(?s)^123  .12345.12345.123  .12345.12345$/,
);
$cmd->run(args => '-h', status => 64, stderr => qr/Usage/);
$cmd->run(status => 64, stderr => qr/Usage/);

done_testing(24);
