#!perl
# there all sorts of race conditions and portability issues in this code
use lib qw(../lib/perl5);
use UtilityTestBelt;
use Capture::Tiny qw(:all);

my $test_prog = './blocksig';

my ( $out, $err, $exit ) = capture { system $test_prog, '-h' };
exit_is( $exit, 64, "EX_USAGE of sysexits(3) fame" );
is( $out, '' );
ok( $err =~ m/Usage/, "help mentions usage" );

sub startup {
    my $pid = fork // die "fork failed: $!\n";
    if ( $pid == 0 ) {
        exec(@_) or die "could not exec '@_': $!\n";
    }
    ok( $pid > 0 );
    return $pid;
}
sub teardown { kill KILL => @_ }

$SIG{CHLD} = 'IGNORE';

#diag "should exit";
my $pid = startup $^X, "-e", "sleep 99";
sleep 3;
is( kill( 0, $pid ), 1, "should exit started up" );
#diag `ps axo pid,command | grep "${pid}[ ]"`;
kill INT => $pid;
sleep 3;
#diag `ps axo pid,command | grep "${pid}[ ]"`;
is( kill( 0, $pid ), 0, "should exit after INT" );

#diag "should not exit";
$pid = startup $test_prog, "--", $^X, "-e", "sleep 99";
sleep 3;
is( kill( 0, $pid ), 1, "should still run started up" );
#diag `ps axo pid,command | grep "${pid}[ ]"`;
kill INT => $pid;
sleep 3;
#diag `ps axo pid,command | grep "${pid}[ ]"`;
is( kill( 0, $pid ), 1, "should still run after INT" );
teardown $pid;

done_testing 9
