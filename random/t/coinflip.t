#!perl

use 5.14.0;
use warnings;
use Test::Cmd;
use Test::Most tests => 6;
use Test::UnixExit;

my $command = Test::Cmd->new(
    prog    => './coinflip',
    verbose => 0,
    workdir => '',
);

$command->run( args => '-q' );
is( $command->stdout, "" );

my ( %seen, %status );
for ( 1 .. 20 ) {
    $command->run;
    chomp( my $out = $command->stdout );
    $seen{$out}++;
    $status{ $? >> 8 }++;
}
eq_or_diff( [ sort keys %seen ],   [qw/heads tails/] );
eq_or_diff( [ sort keys %status ], [qw/0 1/] );

$command->run( args => '-h' );
exit_is( $?, 64 );
is( $command->stdout, "" );
ok( $command->stderr =~ qr/Usage: / );
