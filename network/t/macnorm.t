#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $cmd = Test::UnixCmdWrap->new;

$cmd->run(
    args   => '00:00:36:af:Af:AF',
    stdout => ["00:00:36:af:af:af"],
);
$cmd->run(
    args   => '-O 3 -s - 12:34:56',
    stdout => ["12-34-56"],
);
$cmd->run(
    args   => q{-O 2 -s '' aa/bb CC/DD Ee/Ff},
    stdout => [ "aabb", "ccdd", "eeff" ],
);
$cmd->run(
    args   => '-O 1 -p 01- aa bb cc',
    stdout => [ "01-aa", "01-bb", "01-cc" ],
);
$cmd->run(
    args   => '0:0:36:f:F:AF',
    stdout => ["00:00:36:0f:0f:af"],
);
# each unique input can have own separator (but must internally use
# only that character)
$cmd->run(
    args   => q{00:00:36:af:Af:AF 00-00-36-af-Af-AF},
    stdout => [ "00:00:36:af:af:af", "00:00:36:af:af:af" ],
);
# case choice
$cmd->run(
    args   => '-X -O 1 aa',
    stdout => ["AA"],
);
$cmd->run(
    args   => '-X -x -O 1 aa',
    stdout => ["aa"],
);
# invalid stuff
$cmd->run(
    args   => q{''},
    status => 65,
    stderr => qr/empty string/,
);
# this assumes getopt(3) uses -- to stop option processing
$cmd->run(
    args   => q{-- -00:00:36:af:Af:AF},
    status => 65,
    stderr => qr/xdigit/,
);
$cmd->run(
    args   => q{00::00:36:af:Af:AF},
    status => 65,
    stderr => qr/xdigit/,
);
$cmd->run(
    args   => q{00:00,36:af:Af:AF},
    status => 65,
    stderr => qr/separator/,
);
$cmd->run(
    args   => q{00},
    status => 65,
    stderr => qr/NUL/,
);
# trailing garbage
$cmd->run(
    args   => q{-O 1 eat},
    status => 65,
    stderr => qr/garbage/,
);
# even if the trailing stuff is valid...
$cmd->run(
    args   => '0:0:0:0:0:0:0:0:0:0:0',
    status => 65,
    stderr => qr/garbage/,
);
$cmd->run(
    args   => '-q 0:0:0:0:0:0:0:0:0:0:0',
    status => 65,
);
# not a feature unless you can turn it off
$cmd->run(
    args   => q{-O 1 -T eat},
    stdout => ["ea"],
);
$cmd->run(
    args   => '-T 0:0:0:0:0:0:0:0:0:0:0',
    stdout => ["00:00:00:00:00:00"],
);
$cmd->run(args => '-h', status => 64, stderr => qr/Usage/);
$cmd->run(status => 64, stderr => qr/Usage/);

done_testing(60);
