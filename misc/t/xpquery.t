#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $cmd = Test::UnixCmdWrap->new;

$cmd->run(
    args   => q{'//zoot/text()' t/xip.xml},
    stdout => [ 'cat', 'dog', 'fish' ],
);
$cmd->run(
    args   => q{-S 'zoot/text()' '//zop' t/xip.xml},
    stdout => [ 'cat', 'dog', 'fish' ],
);
$cmd->run(
    args => q{-n 'xlink:http://www.w3.org/1999/xlink' '//foo/@xlink:href' t/ns.xml},
    stdout => [' xlink:href="http://example.org"'],
);
$cmd->run(
    args   => q{-p html '//title/text()' t/foo.html},
    stdout => ['xyz'],
);
# in theory XML::LibXML handles the input encoding (via the ?xml ...
# statement) and in theory Test::Cmd does not muck with the output
# in any way
$cmd->run(
    args   => q{-E UTF-8 '//word/text()' t/utf8.xml},
    stdout => ["\xe5\xbd\x81"],
);
$cmd->run(
    args   => q{-E UTF-8 '//word/text()' t/shift_jis.xml},
    stdout => ["\xe5\xbd\x81"],
);

$cmd->run(args => '-h', status => 64, stderr => qr/Usage/);

done_testing
