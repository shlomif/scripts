#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my @tests = (
    {   args        => "''",
        exit_status => 64,
        stdout      => qr/^$/,
        stderr      => qr/empty string/,
    },
    {   args        => "bar",
        exit_status => 64,
        stdout      => qr/^$/,
        stderr      => qr/number/,
    },
    {   args        => "'0'",
        exit_status => 64,
        stdout      => qr/^$/,
        stderr      => qr/below minimum/,
    },
    {   args        => "9999999",
        exit_status => 64,
        stdout      => qr/^$/,
        stderr      => qr/above maximum/,
    },
    {   args        => "1s20",
        exit_status => 64,
        stdout      => qr/^$/,
        stderr      => qr/unknown char/,
    },
    {   args        => "1d",
        exit_status => 64,
        stdout      => qr/^$/,
        stderr      => qr/stray char/,
    },
    {   args        => "1d1",
        exit_status => 64,
        stdout      => qr/^$/,
        stderr      => qr/below minimum/,
    },
    # proper statistical tests should also be done...
    {   args        => "1d2",
        stdout      => qr/^[12]$/,
        stderr      => qr/^$/,
    },
    {   args        => '-h',
        exit_status => 64,
        stdout      => qr/^$/,
        stderr      => qr/^Usage: /,
    },
    {   exit_status => 64,
        stdout      => qr/^$/,
        stderr      => qr/^Usage: /,
    },
);
my $command = Test::Cmd->new( prog => './roll', workdir => '', );

for my $test (@tests) {
    $test->{exit_status} //= 0;
    $test->{stderr}      //= qr/^$/;

    $command->run( exists $test->{args} ? ( args => $test->{args} ) : () );

    my $args = ' ' . ( $test->{args} // '' );
    exit_is( $?, $test->{exit_status}, "STATUS ./roll$args" );
    ok( $command->stdout =~ $test->{stdout}, "STDOUT ./roll$args" );
    ok( $command->stderr =~ $test->{stderr}, "STDERR ./roll$args" );
}
done_testing( @tests * 3 );
