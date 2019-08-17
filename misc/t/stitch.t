#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $cmd = Test::UnixCmdWrap->new;

$cmd->run(
    args   => '--ofs=\\\\t t/stitch-a:2 t/stitch-b:1,3 -',
    stdin  => "i1\ni2\n",
    stdout => [ "b\t1\t3\ti1", "\t4\t6\ti2", "\t7\t9" ],
);
$cmd->run(args => '-h', status => 64, stderr => qr/Usage/);

done_testing(6);
