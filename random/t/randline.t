#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

# these are basic interface tests; more than one input line requires
# statistical guesswork on account of the (hopefully) random nature of
# the line selection
my @tests = (
    {   stdin  => "one\n",
        stdout => "one\n",
    },
    {   stdin  => "nonl",
        stdout => "nonl\n",
    },
    {   args   => "t/randline-one",
        stdout => "one\n",
    },
    {   args   => "t/randline-empty t/randline-one",
        stdout => "one\n",
    },
    # no input is an error, or at least non-zero exit status
    {   exit_status => 1,
        stdout      => "",
    },
    {   args        => '-h',
        exit_status => 64,
        stdout      => "",
        stderr      => qr/^Usage: /,
    },
);
my $command = Test::Cmd->new( prog => './randline', workdir => '', );

for my $test (@tests) {
    $test->{exit_status} //= 0;
    $test->{stderr}      //= qr/^$/;

    $command->run(
        exists $test->{args}  ? ( args  => $test->{args} )  : (),
        exists $test->{stdin} ? ( stdin => $test->{stdin} ) : (),
    );

    my $args = ' ' . ( $test->{args} // '' );
    exit_is( $?, $test->{exit_status}, "STATUS ./randline$args" );
    is( $command->stdout, $test->{stdout}, "STDOUT ./randline$args" );
    ok( $command->stderr =~ $test->{stderr}, "STDERR ./randline$args" );
}
# this may false alarm if the RNG does not pick one of the choices in
# the given number of trials (versus wasting more CPU time...)
# TODO really do need more extensive statistical tests, perhaps by
# setting a special ENV variable
my %seen;
for ( 1 .. 30 ) {
    $command->run( stdin => "a\nb\nc\n" );
    chomp( my $out = $command->stdout );
    $seen{$out}++;
}
eq_or_diff( [ sort keys %seen ], [qw/a b c/] );
done_testing( @tests * 3 + 1 );
