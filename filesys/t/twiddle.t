#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $test_prog = './twiddle';

my ( $tfh, $tf );
lives_ok(
    sub {
        ( $tfh, $tf ) = tempfile( "twiddle.XXXXXXXXXX", TMPDIR => 1, UNLINK => 1 );
    },
    'tempfile creation should not croak'
);

print $tfh "twiddle\n";
$tfh->flush;
$tfh->sync;
seek( $tfh, 0, 0 );

my $testcmd = Test::Cmd->new( prog => $test_prog, workdir => '', );

# make no sparse files (because the pread fails)
$testcmd->run( args => "-b 1 -o 99999 '$tf'" );
exit_is( $?, 74, "big offset IO failure" );
is( $testcmd->stdout, "", "big offset no stdout" );
ok( $testcmd->stderr =~ m/read of/, "big offset stderr" );

# these two for flipping a bit forth and then back
$testcmd->run( args => "-b 3 -o 2 '$tf'" );
exit_is( $?, 0, "twaddle exit status" );
is( $testcmd->stdout, "",          "twaddle no stdout" );
is( $testcmd->stderr, "",          "twaddle no stderr" );
is( readline($tfh),   "twaddle\n", "twiddle became twaddle" );
seek( $tfh, 0, 0 );

$testcmd->run( args => "-b 3 -o 2 '$tf'" );
exit_is( $?, 0, "twiddle exit status" );
is( $testcmd->stdout, "",          "twiddle no stdout" );
is( $testcmd->stderr, "",          "twiddle no stderr" );
is( readline($tfh),   "twiddle\n", "twaddle became twiddle" );

done_testing(12);
