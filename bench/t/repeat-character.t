#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $cmd = Test::UnixCmdWrap->new;

# TODO test that e.g. full buffer actually buffers fully, etc
$cmd->run(stderr => qr/^Usage/, status => 64);
$cmd->run(
    args   => q{hello computer could you repeat characters for me},
    stderr => qr/^Usage/,
    status => 64,
);
$cmd->run(
    args   => q{e 3 7 chrome},
    stderr => qr/^Usage/,
    status => 64,
);
$cmd->run(
    args   => q{x 3 7 none},
    stdout => ['xxxxxxxxxxxxxxxxxxxxx'],
);
$cmd->run(
    args   => q{x 3 7 full},
    stdout => ['xxxxxxxxxxxxxxxxxxxxx'],
);
# six characters only because we count one to the newline so that
# "7 3" generates 21 characters regardless of the buffer style
$cmd->run(
    args   => q{l 7 3 line},
    stdout => [ 'llllll', 'llllll', 'llllll' ],
);

done_testing(18);
