#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $cmd = Test::UnixCmdWrap->new;

my $tcmd = $cmd->cmd;
$tcmd->run( args => '-q' );
is( $tcmd->stdout, "" );
is( $tcmd->stderr, "" );

my ( %seen, %status );
# NOTE this could false alarm if there is a (rare) streak of all heads
# or all tails
for ( 1 .. 13 ) {
    $tcmd->run;
    $status{ $? >> 8 }++;
    chomp( my $out = $tcmd->stdout );
    $seen{$out}++;
}
eq_or_diff( [ sort keys %seen ],   [qw/heads tails/] );
eq_or_diff( [ sort keys %status ], [qw/0 1/] );

$cmd->run( args => '-h', status => 64, stderr => qr/Usage/ );

done_testing(7);
