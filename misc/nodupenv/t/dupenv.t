#!perl

use 5.14.0;
use warnings;
use Test::Cmd;
use Test::Most tests => 64;
use Test::UnixExit;

# for eq_or_diff; see "DIFF STYLES" in perldoc Test::Differences
unified_diff;

my $test_prog = './dupenv';

my @tests = (
    # these all should have no arguments to exec so like env(1) should
    # report the set environment variables
    {   env    => { DUPENVTEST => 'goodifseen' },
        stdout => [qr/(?m)^DUPENVTEST=goodifseen$/],
    },
    # -i should wipe out the existing environment
    {   args   => '-i',
        env    => { DUPENVTEST => 'badifseen' },
        stdout => [qr/^$/],
    },
    # as should -i followed by something that prints the env (but
    # without any new env being set)
    {   args   => "-i '$test_prog'",
        env    => { DUPENVTEST => 'badifseen' },
        stdout => [qr/^$/],
    },
    {   args   => '-i DUPENVTEST=goodifseen',
        stdout => [qr/(?m)^DUPENVTEST=goodifseen$/],
    },
    {   args => '-i DUPENVTEST=goodifseen DUPENVTEST=alsogoodifseen',
        stdout =>
          [ qr/(?m)^DUPENVTEST=goodifseen$/, qr/(?m)^DUPENVTEST=alsogoodifseen$/, ],
    },
    {   args => 'DUPENVTEST=alsogoodifseen',
        env  => { DUPENVTEST => 'goodifseen' },
        stdout =>
          [ qr/(?m)^DUPENVTEST=goodifseen$/, qr/(?m)^DUPENVTEST=alsogoodifseen$/, ],
    },
    # can we exec ourself? (vendor provided env(1) who knows how it
    # handles duplicate envs...)
    {   args   => "-i DUPENVTEST=goodifseen '$test_prog'",
        env    => { DUPENVTEST => 'badifseen' },
        stdout => [qr/(?m)^DUPENVTEST=goodifseen$/],
    },
    {   args => "DUPENVTEST=goodifseen DUPENVTEST=alsogoodifseen '$test_prog'",
        stdout =>
          [ qr/(?m)^DUPENVTEST=goodifseen$/, qr/(?m)^DUPENVTEST=alsogoodifseen$/, ],
    },
    # so very wrong
    {   args        => "-i =nameless '$test_prog'",
        exit_status => 1,
        stderr      => qr/invalid/,
        stdout      => [qr/^$/],
    },
    # but allowed! with a flag
    {   args   => "-U -i =nameless '$test_prog'",
        stdout => [qr/^=nameless$/],
    },
);

my $testcmd = Test::Cmd->new(
    prog    => $test_prog,
    verbose => 0,
    workdir => '',
);

for my $test (@tests) {
    $test->{exit_status} //= 0;
    $test->{stderr}      //= qr/^$/;

    # Test::Cmd complicates matters by running things through the shell
    # which can in turn introduce envrionment variables so the stdout
    # tests instead use regex to look for things
    local %ENV;
    @ENV{ keys %{ $test->{env} } } = values %{ $test->{env} };

    $testcmd->run( exists $test->{args} ? ( args => $test->{args} ) : () );

    $test->{args} //= '';

    exit_is( $?, $test->{exit_status}, "STATUS $test_prog $test->{args}" );
    for my $re ( @{ $test->{stdout} } ) {
        ok( $testcmd->stdout =~ m/$re/, "STDOUT $test_prog $test->{args} ($re)" );
    }
    ok( $testcmd->stderr =~ m/$test->{stderr}/, "STDERR $test_prog $test->{args}" );
}

# look for realloc bugs around the default newenv_alloc value starting
# from a clean slate environment
{
    my $env_count = 62;

    my @envspam = map { sprintf "ENVSPAM=%02d", $_ } 1 .. $env_count;

    for my $i ( 1 .. 4 ) {
        my $cur = $env_count + $i;
        push @envspam, sprintf "ENVSPAM=%02d", $cur;

        $testcmd->run( args => "'$test_prog' -i " . join ' ', @envspam );
        eq_or_diff( [ map { tr/\n//dr } $testcmd->stdout ],
            \@envspam, "-i envspam count $cur" );

        exit_is( $?, 0, "-i realloc test $cur" );
        is( $testcmd->stderr, "", "no stderr for -i envspam count $cur" );
    }
}

# and also when "the usual environment" is being duplicated onto as that
# involves different code than the above clean slate
{
    local %ENV;
    # ... for some definition of "the usual environment" as we need
    # something less than newenv_alloc which the user's default
    # environment may be larger than
    $ENV{FOO} = 'bar';

    $testcmd->run( args => $test_prog );
    exit_is( $?, 0, "obtain default env used by Test::Cmd" );

    my @default_env = map { tr/\n//dr } $testcmd->stdout;

    # the heck shell are they running that adds this many by default?
    BAIL_OUT("too many env vars set by sh??") if @default_env > 59;

    my $env_count = 60 - @default_env;
    my @envspam = map { sprintf "ENVSPAM=%02d", $_ } 1 .. $env_count;

    for my $i ( 1 .. 5 ) {
        my $cur = $env_count + $i;
        push @envspam, sprintf "ENVSPAM=%02d", $cur;

        $testcmd->run( args => "'$test_prog' " . join ' ', @envspam );
        eq_or_diff(
            [ map { tr/\n//dr } $testcmd->stdout ],
            [ @default_env, @envspam ],
            "envspam count $cur"
        );

        exit_is( $?, 0, "realloc test $cur" );
        is( $testcmd->stderr, "", "no stderr for envspam count $cur" );
    }
}

$testcmd->run( args => '-h' );
exit_is( $?, 64, "EX_USAGE of sysexits(3) fame" );
is( $testcmd->stdout, "", "no stdout on help" );
ok( $testcmd->stderr =~ m/Usage/, "help mentions usage" );
