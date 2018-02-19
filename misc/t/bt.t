#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;
use Cwd qw(getcwd);
use Expect;

my $test_prog = File::Spec->catfile( getcwd, 'bt' );
my $exp;

$ENV{BT_DEBUGGER} = "echo by env $$";

$exp = expect_spawn_ok( $test_prog, '-D', "echo got $$" );
expect_eoft( $exp, 3, 'eof', 'echo should have exited' );
exit_is( $exp->exitstatus, 0, "echo should exit" );
ok( $exp->before =~ m/got $$/, "arg passthrough of pid" );

$exp = expect_spawn_ok($test_prog);
expect_eoft( $exp, 3, 'eof', 'echo should have exited' );
ok( $exp->before =~ m/by env $$/, "env passthrough of pid" );

# could test that gdb runs, but that's somewhat complicated, and
# something the user will be directly interacting with, so in theory
# errors will be spotted. can create a directory with known contents and
# confirm that at least the *.core and executable detection works...

$ENV{BT_DEBUGGER} = "echo gdb";

my $tmpdir = tempdir( "bt.XXXXXXXXXX", CLEANUP => 1, TMPDIR => 1 );
chdir $tmpdir;

$exp = expect_spawn_ok($test_prog);
expect_eoft( $exp, 3, 'eof', 'echo should have exited' );
exit_is( $exp->exitstatus, 1, "bt error" );
ok( $exp->before =~ m/bt: no.*found/, "no files found error" );

my $fh;
open $fh, '>', "prog$$" or die "could not write file in '$tmpdir': $!\n";
chmod 0755, "prog$$";

$exp = expect_spawn_ok($test_prog);
expect_eoft( $exp, 3, 'eof', 'echo should have exited' );
ok( $exp->before =~ m/gdb prog$$/, "program to debug" );

open $fh, '>', "fail$$.core" or die "could not write file in '$tmpdir': $!\n";
$exp = expect_spawn_ok($test_prog);
expect_eoft( $exp, 3, 'eof', 'echo should have exited' );
ok( $exp->before =~ m/gdb fail$$ fail$$.core/, "program and core to debug" );

$exp = expect_spawn_ok( $test_prog, '-h' );
expect_eoft( $exp, 3, 'eof', 'program exited' );
exit_is( $exp->exitstatus, 64, "EX_USAGE of sysexits(3) fame" );
ok( $exp->before =~ m/Usage/, "help mentions usage" );
done_testing(21);

sub expect_eoft {
    my ( $exp, $timeout, $wanted, $name ) = @_;
    my $result;
    $exp->expect(
        $timeout,
        'eof'     => sub { $result = 'eof' },
        'timeout' => sub { $result = 'timeout' }
    );
    is( $result, $wanted, $name );
}

sub expect_spawn_ok {
    my $exp = Expect->new;
    $exp->raw_pty(1);
    #$exp->debug(3);
    ok( $exp->spawn(@_), "expect object spawned" );
    return $exp;
}
