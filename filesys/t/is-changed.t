#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;
use Expect;

my $test_prog = './is-changed';

my ( $tfh, $tf );
lives_ok(
    sub {
        ( $tfh, $tf ) = tempfile( "ic.XXXXXXXXXX", TMPDIR => 1, UNLINK => 1 );
    },
    'tempfile creation should not croak'
);

$tfh->autoflush;

my $exp = newexpect();
ok( $exp->spawn( $test_prog, '5s', "$^X -E 'say q{poke}'", $tf ),
    "expect object spawned" );

diag "tests will take some time...";

# NOTE KLUGE timing might be off on a busy system, or possibly buffering
# related things...
sleep 1;
$tfh->say("poke");
sleep 1;
$tfh->say("poke");
sleep 5;

kill INT => $exp->pid;

my $return;
$exp->expect(
    10,
    'eof'     => sub { $return = 'eof' },
    'timeout' => sub { $return = 'timeout' }
);
is( $return, 'eof', "exits after control+c" );

my $pokes;
$pokes++ for $exp->before =~ m/poke/g;
is( $pokes, 1, "one poke from two changes" );

done_testing(4);

sub newexpect {
    my $exp = Expect->new;
    $exp->raw_pty(1);
    #$exp->debug(3);
    return $exp;
}
