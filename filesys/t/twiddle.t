#!perl

use 5.14.0;
use warnings;
use File::Temp qw(tempfile);
use Test::Cmd;
use Test::Most tests => 12;
use Test::UnixExit;

my $test_prog = './twiddle';

my ( $tfh, $test_file );
lives_ok(
    sub {
        ( $tfh, $test_file ) =
          tempfile( "twiddle.XXXXXXXXXX", TMPDIR => 1, UNLINK => 1 );
    },
    'tempfile creation should not croak'
);

print $tfh "twiddle\n";
$tfh->flush;
$tfh->sync;
seek( $tfh, 0, 0 );

my $testcmd = Test::Cmd->new(
    prog    => $test_prog,
    verbose => 0,
    workdir => '',
);

# make no sparse files (because the pread fails)
$testcmd->run( args => "-b 1 -o 99999 $test_file" );
exit_is( $?, 74, "big offset IO failure" );
is( $testcmd->stdout, "", "big offset no stdout" );
ok( $testcmd->stderr =~ m/read of/, "big offset stderr" );

# these two for flipping a bit forth and then back
$testcmd->run( args => "-b 3 -o 2 $test_file" );
exit_is( $?, 0, "twaddle exit status" );
is( $testcmd->stdout, "",          "twaddle no stdout" );
is( $testcmd->stderr, "",          "twaddle no stderr" );
is( readline($tfh),   "twaddle\n", "twiddle became twaddle" );
seek( $tfh, 0, 0 );

$testcmd->run( args => "-b 3 -o 2 $test_file" );
exit_is( $?, 0, "twiddle exit status" );
is( $testcmd->stdout, "",          "twiddle no stdout" );
is( $testcmd->stderr, "",          "twiddle no stderr" );
is( readline($tfh),   "twiddle\n", "twaddle became twiddle" );
