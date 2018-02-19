#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $command = Test::Cmd->new( prog => './coinflip', workdir => '', );

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
done_testing(6);
