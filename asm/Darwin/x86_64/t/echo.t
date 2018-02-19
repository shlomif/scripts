#!perl
use lib qw(../../../lib/perl5);
use UtilityTestBelt;
use Capture::Tiny qw(capture_stderr);

my @tests = (
    {   command => [qw(./echo)],
        output  => "\n",
    },
    {   command => [qw(./echo -n)],
        output  => "",
    },
    {   command => [qw(./echo asdf)],
        output  => "asdf\n",
    },
    {   command => [qw(./echo -n asdf)],
        output  => "asdf",
    },
    {   command => [qw(./echo -n asdf)],
        output  => "asdf",
    },
    # must quote \c both for Perl and for qx(...) shell
    {   command => [ "./echo", "'asdf\\c'" ],
        output  => "asdf",
    },
    {   command => [ "./echo", "-n", "'asdf\\c'" ],
        output  => "asdf",
    },
    {   command => [qw(./echo asdf asdf)],
        output  => "asdf asdf\n",
    },
    {   command => [qw(./echo asdf asdf asdf)],
        output  => "asdf asdf asdf\n",
    },
    # this one is tricky with the empty argument
    #   $ /bin/echo asdf '' asdf
    #   asdf  asdf
    #   $
    {   command => [ "./echo", "-n", "asdf", "''", "asdf" ],
        output  => "asdf  asdf",
    },
    # this should cause a non-zero exit as stdout is closed
    {   command     => ["./echo nope >&-"],
        exit_status => 1,
        output      => "",
    },
);

my $testnum = 1;
for my $t (@tests) {
    $t->{exit_status} //= 0;
    my $output = qx(@{ $t->{command} });
    $t->{testnum} = $testnum++;
    exit_is( $?, $t->{exit_status} );
    $t->{testnum} = $testnum++;
    is( $output, $t->{output} ) or diag what_test($t);
}

# what happens when we use the maximum amount of data exec can handle?
{
    # maximize argument length by minimizing the environment
    local %ENV;

    # ARG_MAX from /usr/include/sys/syslimits.h on Mac OS X 10.11.6
    my $arg_max = 0x40000;
    my $high    = $arg_max;
    my $low     = 1;
    my $argc    = between( $low, $high );

    my $command = './echo -n';
    my $argstr;

    while (1) {
        $argstr = "a" x $argc;
        capture_stderr { qx($command $argstr) };
        if ( $? == 0 ) {
            $low = $argc + 1;
        } else {
            $high = $argc - 1;
        }
        if ( $low >= $high ) {
            $argc = $low;
            last;
        }
        $argc = between( $low, $high );
    }

    diag "maximum arg length $argc (of $arg_max)";
    $argstr = "a" x $argc;
    my $output = qx($command $argstr);
    exit_is( $?, 0 );
    # the output is not shown by default as there's a lot of it; if this
    # test breaks collect and review the output manually...
    ok( $output eq $argstr, "maximum args output differs from input" );
}
done_testing( @tests * 2 + 2 );

sub between { int( ( $_[0] + $_[1] ) / 2 ) }

sub what_test {
    my ($t) = @_;
    return "test " . $t->{testnum} . ":  " . join ' ',
      map { '"' . $_ . '"' } @{ $t->{command} };
}
