#!perl
use lib qw(../../../lib/perl5);
use UtilityTestBelt;

my $expected = "OpenBSD6.4-amd64\n";
my $output;

$output = qx(./localarchdir);
exit_is( $?, 0 );
ok( $output eq $expected ) or compare( $output, $expected );

$output = qx(./localarchdir >&-);
exit_is( $?, 1 );
is( $output, "" );

done_testing(4);

sub compare {
    for my $arg (@_) {
        my $nofancy = $arg =~ s/[^\x20-\x7e]/./gr;
        diag sprintf "%0*v2x\t%s\n", ' ', $arg, $nofancy;
    }
}
