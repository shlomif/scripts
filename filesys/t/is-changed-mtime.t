#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;
use Expect;

my $test_prog = './is-changed-mtime';

my ( $tfh, $tf );
lives_ok(
    sub { ( $tfh, $tf ) = tempfile( "icm.XXXXXXXXXX", TMPDIR => 1, UNLINK => 1 ); },
    'tempfile creation should not croak'
);

my $exp = newexpect();
ok( $exp->spawn( $test_prog, '5s', "$^X -E 'say q{poke}'", $tf ),
    "expect object spawned" );

diag "tests will take some time...";

# NOTE KLUGE timing might be off on a busy system, or possibly buffering
# related things, PORTABILITY of the utime calls, etc
sleep 1;
utime undef, undef, $tf;
sleep 1;
utime undef, undef, $tf;
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
is( $pokes, 1, "one poke from two touches" );

done_testing(4);

sub newexpect {
    my $exp = Expect->new;
    $exp->raw_pty(1);
    #$exp->debug(3);
    return $exp;
}
