#!perl
use lib qw(../../../lib/perl5);
use UtilityTestBelt;

my $test_count = 6;

my $output;

$output = qx(./false);
exit_is( $?, 1, "false" );
is( $output, "" );

$output = qx(./regpollute);
exit_is( $?, 1, "register pollution" );
is( $output, "" );

$output = qx(./true);
exit_is( $?, 0, "true" );
is( $output, "" );

# Mac OS X apparently does not follow the System V ABI for AMD6 and may
# or may not 16-byte align the stack pointer so depending on the number
# of arguments (and what calls are used?) the program may run correctly,
# or segfault. this does not appear to be a problem for {true,false} as
# there is no -lc nor CALL instructions, but test with some arguments to
# help confirm no hidden segfaults on random arguments
$test_count += test_args( "./false", 1 );
$test_count += test_args( "./true",  0 );

done_testing($test_count);

sub test_args {
    my ( $program, $exit ) = @_;
    my $tests = 0;
  TEST: for my $argc ( 1 .. 8 ) {
        for my $len (qw(0 5 9 17)) {
            my @args = map { "a" x $len } 1 .. $argc;
            qx($program @args);
            $tests++;
            exit_is( $?, $exit ) or last TEST;
        }
    }
    return $tests;
}
