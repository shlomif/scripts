#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $cmd = Test::UnixCmdWrap->new;

my ( $tfh, $tf );
lives_ok(
    sub {
        ( $tfh, $tf ) = tempfile( "twiddle.XXXXXXXXXX", TMPDIR => 1, UNLINK => 1 );
    },
    'tempfile creation should not croak'
);

$tfh->say("twiddle");
$tfh->flush;
$tfh->sync;
seek( $tfh, 0, 0 );

# make no sparse files (because the pread fails)
$cmd->run( args => "-b 1 -o 99999 '$tf'", status => 74, stderr => qr/read of/ );

# these two for flipping a bit forth and then back
$cmd->run( args => "-b 3 -o 2 '$tf'" );
is( readline($tfh),   "twaddle\n", "twiddle became twaddle" );
seek( $tfh, 0, 0 );

$cmd->run( args => "-b 3 -o 2 '$tf'" );
is( readline($tfh),   "twiddle\n", "twaddle became twiddle" );

done_testing(12);
