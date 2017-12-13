#!perl
use 5.14.0;
use warnings;
use Test::Most tests => 4;
use Test::UnixExit;

my $expected = "Darwin15.6.0-x86_64\n";
my $output;

$output = qx(./localarchdir);
exit_is( $?, 0 );
ok( $output eq $expected ) or compare( $output, $expected );

$output = qx(./localarchdir >&-);
exit_is( $?, 1 );
is( $output, "" );

sub compare {
    for my $arg (@_) {
        my $nofancy = $arg =~ s/[^\x20-\x7e]/./gr;
        diag sprintf "%0*v2x\t%s\n", ' ', $arg, $nofancy;
    }
}
