#!perl

use 5.14.0;
use warnings;
use Test::Cmd;
use Test::Most;
use Test::UnixExit;

my @tests = (
    {   args   => "xom",
        stdout => "xom\n",
    },
    {   args        => '',
        exit_status => 64,
        stdout      => "",
        stderr      => qr/^Usage: /,
    },
    {   args        => '-h',
        exit_status => 64,
        stdout      => "",
        stderr      => qr/^Usage: /,
    },
);
my $command = Test::Cmd->new(
    prog    => './oneof',
    verbose => 0,
    workdir => '',
);

for my $test (@tests) {
    $test->{exit_status} //= 0;
    $test->{stderr} //= qr/^$/;

    $command->run( exists $test->{args}  ? ( args  => $test->{args} ) : () );

    my $args = ' ' . ( $test->{args} // '' );
    exit_is( $?, $test->{exit_status}, "STATUS ./oneof$args" );
    is( $command->stdout, $test->{stdout}, "STDOUT ./oneof$args" );
    ok( $command->stderr =~ $test->{stderr}, "STDERR ./oneof$args" );
}

my %seen;
for ( 1 .. 30 ) {
    $command->run( args => "a b c" );
    chomp( my $out = $command->stdout );
    $seen{$out}++;
}
eq_or_diff( [ sort keys %seen ], [qw/a b c/] );

done_testing( @tests * 3 + 1 );
