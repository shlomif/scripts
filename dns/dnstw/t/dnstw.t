#!perl
use lib qw(../../lib/perl5);
use UtilityTestBelt;

my $test_prog = './dnstw';

diag( "srand seed " . srand );

# NOTE these must be kept in sync with dnstw.c and modules/_common.tcl
my $test_domain  = 'dnstw.test';
my $test_mx_prio = 10;
my $test_ttl     = 3600;

# NOTE and this in sync with the min/max TTL
my $rttl = int( 60 + rand(86000) );

my $rlabel = random_label();
my $rv4o   = int( 1 + rand(254) );

my @tests = (
    {   args   => '-n create-cname host pointer',
        stdout => [
            "yxdomain host.$test_domain.",
            "nxdomain pointer.$test_domain.",
            "add pointer.$test_domain. $test_ttl CNAME host.$test_domain.", "send",
        ],
    },
    # TODO does $domain need to be dot-qualified?
    {   args =>
          "-d $rlabel.$test_domain -n -S 192.0.2.$rv4o -T $rttl create-cname h$rlabel p$rlabel",
        stdout => [
            "server 192.0.2.$rv4o",
            "yxdomain h$rlabel.$rlabel.$test_domain",
            "nxdomain p$rlabel.$rlabel.$test_domain",
            "add p$rlabel.$rlabel.$test_domain $rttl CNAME h$rlabel.$rlabel.$test_domain",
            "send",
        ],
    },
    {   args   => "-F -n create-cname h$rlabel.$test_domain. p$rlabel.$test_domain.",
        stdout => [
            "yxdomain h$rlabel.$test_domain.",
            "nxdomain p$rlabel.$test_domain.",
            "add p$rlabel.$test_domain. $test_ttl CNAME h$rlabel.$test_domain.", "send",
        ],
    },
    # NOTE this test may not be portable depending on what inet_ntop(3)
    # returns especially for IPv6 addresses
    {   args   => "-n create-host skami042 192.0.2.42 2001:0db8::c000:22a",
        stdout => [
            "add skami042.$test_domain. $test_ttl A 192.0.2.42",
            "send",
            "nxrrset 42.2.0.192.in-addr.arpa. PTR",
            "add 42.2.0.192.in-addr.arpa. $test_ttl PTR skami042.$test_domain.",
            "send",
            "add skami042.$test_domain. $test_ttl AAAA 2001:db8::c000:22a",
            "send",
            "nxrrset a.2.2.0.0.0.0.c.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.8.b.d.0.1.0.0.2.ip6.arpa. PTR",
            "add a.2.2.0.0.0.0.c.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.8.b.d.0.1.0.0.2.ip6.arpa. $test_ttl PTR skami042.$test_domain.",
            "send",
        ],
    },
    {   args   => '-n create-mx host mxhost',
        stdout => [
            "yxdomain host.$test_domain.",
            "yxdomain mxhost.$test_domain.",
            "add host.$test_domain. $test_ttl MX $test_mx_prio mxhost.$test_domain.",
            "send",
        ],
    },
    {   args   => "-n create-mx host $rv4o $rlabel",
        stdout => [
            "yxdomain host.$test_domain.",
            "yxdomain $rlabel.$test_domain.",
            "add host.$test_domain. $test_ttl MX $rv4o $rlabel.$test_domain.", "send",
        ],
    },
    {   args => "-n create-reverse fatna 192.0.2.99 xn--zck2b3d7c 2001:db8::c000:0263",
        stdout => [
            "nxrrset 99.2.0.192.in-addr.arpa. PTR",
            "add 99.2.0.192.in-addr.arpa. $test_ttl PTR fatna.$test_domain.",
            "send",
            "nxrrset 3.6.2.0.0.0.0.c.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.8.b.d.0.1.0.0.2.ip6.arpa. PTR",
            "add 3.6.2.0.0.0.0.c.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.8.b.d.0.1.0.0.2.ip6.arpa. $test_ttl PTR xn--zck2b3d7c.$test_domain.",
            "send",
        ],
    },
    {   args   => "-n delete-cname foo bar",
        stdout => [
            "yxrrset foo.$test_domain. CNAME",
            "del foo.$test_domain. CNAME",
            "send",
            "yxrrset bar.$test_domain. CNAME",
            "del bar.$test_domain. CNAME",
            "send",
        ],
    },
    {   args   => "-n delete-host goner",
        stdout => [
            "yxdomain goner.$test_domain.",
            "nxrrset goner.$test_domain. NS",
            "del goner.$test_domain.",
            "send",
        ],
    },
    {   args   => "-n delete-host goner 192.0.2.254 2001:db8::c000:02fe",
        stdout => [
            "yxdomain goner.$test_domain.",
            "nxrrset goner.$test_domain. NS",
            "del goner.$test_domain.",
            "send",
            "yxrrset 254.2.0.192.in-addr.arpa. PTR",
            "del 254.2.0.192.in-addr.arpa. PTR",
            "send",
            "yxrrset e.f.2.0.0.0.0.c.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.8.b.d.0.1.0.0.2.ip6.arpa. PTR",
            "del e.f.2.0.0.0.0.c.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.8.b.d.0.1.0.0.2.ip6.arpa. PTR",
            "send",
        ],
    },
    {   args   => "-n delete-mx $rlabel",
        stdout => [
            "yxrrset $rlabel.$test_domain. MX",
            "del $rlabel.$test_domain. MX", "send",
        ],
    },
    {   args   => "-n delete-mx $rlabel 10 mx1 20 mx2",
        stdout => [
            "yxrrset $rlabel.$test_domain. MX 10 mx1",
            "del $rlabel.$test_domain. MX 10 mx1",
            "yxrrset $rlabel.$test_domain. MX 20 mx2",
            "del $rlabel.$test_domain. MX 20 mx2",
            "send",
        ],
    },
    {   args   => "-n delete-reverse 192.0.2.104 2001:db8::200:36ff:fecd:2fa",
        stdout => [
            "yxrrset 104.2.0.192.in-addr.arpa. PTR",
            "del 104.2.0.192.in-addr.arpa. 104.2.0.192.in-addr.arpa. PTR",
            "send",
            "yxrrset a.f.2.0.d.c.e.f.f.f.6.3.0.0.2.0.0.0.0.0.0.0.0.0.8.b.d.0.1.0.0.2.ip6.arpa. PTR",
            "del a.f.2.0.d.c.e.f.f.f.6.3.0.0.2.0.0.0.0.0.0.0.0.0.8.b.d.0.1.0.0.2.ip6.arpa. a.f.2.0.d.c.e.f.f.f.6.3.0.0.2.0.0.0.0.0.0.0.0.0.8.b.d.0.1.0.0.2.ip6.arpa. PTR",
            "send",
        ],
    },
    {   args => qq{-n -F make-record $test_domain. TXT '"v=spf1 mx ip6:2001:db8::/32"'},
        stdout => [
            qq{add $test_domain. $test_ttl TXT "v=spf1 mx ip6:2001:db8::/32"}, "send",
        ],
    },
    {   args   => '-n nscat',
        stdin  => $rlabel . "\n",
        stdout => [$rlabel],
    },
    {   args   => '-n repoint-cname host newp',
        stdout => [
            "yxrrset newp.$test_domain. CNAME",
            "yxdomain host.$test_domain.",
            "del newp.$test_domain. CNAME",
            "add newp.$test_domain. $test_ttl CNAME host.$test_domain.",
            "send",
        ],
    },
    {   args   => "-n unmake-record _VLMCS._TCP IN SRV 0 100 1688 mskms.$test_domain",
        stdout => [
            "del _VLMCS._TCP.$test_domain. 3600 IN SRV 0 100 1688 mskms.$test_domain", "send",
        ],
    },
    # limits
    {   args =>
          '-n create-cname host aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa',
        exit_status => 1,
        stderr      => qr/cname label cannot/,
    },
    {   args        => '-n create-cname host _nope',
        exit_status => 1,
        stderr      => qr/cname label may only/,
    },
    {   args =>
          '-n create-cname aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.a p',
        exit_status => 1,
        stderr      => qr/host cannot/,
    },
    {   args        => '-n -T 0 nscat',
        exit_status => 65,
        stderr      => qr/value for -T is below min/,
    },
    {   args        => '-n -T 9999999 nscat',
        exit_status => 65,
        stderr      => qr/value for -T is above max/,
    },
    # module with bad TCL code
    {   args        => 'test-badcode',
        exit_status => 1,
        stderr      => qr/wrong # args/,
    },
    # usage
    {   args        => '-h',
        exit_status => 64,
        stderr      => qr/Usage: /,
    },
    {   exit_status => 64,
        stderr      => qr/Usage: /,
    },
);
my $command = Test::Cmd->new( prog => $test_prog, workdir => '', );

for my $test (@tests) {
    $test->{exit_status} //= 0;
    $test->{stdout}      //= [];
    $test->{stderr}      //= qr/^$/;

    $command->run(
        exists $test->{args}  ? ( args  => $test->{args} )  : (),
        exists $test->{stdin} ? ( stdin => $test->{stdin} ) : ()
    );

    my $args = $test->{args} // '';

    exit_is( $?, $test->{exit_status}, "STATUS $test_prog $args" );
    eq_or_diff( [ map { s/\s+$//r } split $/, $command->stdout ],
        $test->{stdout}, "STDOUT $test_prog $args" );
    ok( $command->stderr =~ m/$test->{stderr}/, "STDERR $test_prog $args" )
      or diag "STDERR " . $command->stderr;
}
done_testing( @tests * 3 );

sub random_label {
    my @allowed = ( 'a' .. 'z', 0 .. 9 );
    join '', map { $allowed[ rand @allowed ] } 1 .. ( 1 + rand 7 );
}
