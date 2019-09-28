#!perl
use lib qw(../../lib/perl5);
use UtilityTestBelt;

# for eq_or_diff; see "DIFF STYLES" in perldoc Test::Differences
unified_diff;

my $cmd       = Test::UnixCmdWrap->new;
my $test_prog = './dupenv';

delete $ENV{DUPENVTEST};

# these all should have no arguments to exec so like env(1) should
# report the set environment variables
$cmd->run(
    env    => { DUPENVTEST => 'goodifseen' },
    stdout => qr/(?m)^DUPENVTEST=goodifseen$/,
);

# -i should wipe out the existing environment
$cmd->run(
    args => '-i',
    env  => { DUPENVTEST => 'badifseen' },
);

# as should -i followed by something that prints the env (but
# without any new env being set)
$cmd->run(
    args => "-i '$test_prog'",
    env  => { DUPENVTEST => 'badifseen' },
);

$cmd->run(
    args   => '-i DUPENVTEST=goodifseen',
    stdout => qr/(?m)^DUPENVTEST=goodifseen$/,
);

my $o = $cmd->run(
    args   => '-i DUPENVTEST=goodifseen DUPENVTEST=alsogoodifseen',
    stdout => qr/(?m)^DUPENVTEST=goodifseen$/
);
ok($o->stdout =~ m/(?m)^DUPENVTEST=alsogoodifseen$/);

$o = $cmd->run(
    args   => 'DUPENVTEST=alsogoodifseen',
    env    => { DUPENVTEST => 'goodifseen' },
    stdout => qr/(?m)^DUPENVTEST=goodifseen$/
);
ok($o->stdout =~ m/(?m)^DUPENVTEST=alsogoodifseen$/);

# can we exec ourself? (vendor provided env(1) who knows how it
# handles duplicate envs...)
$cmd->run(
    args   => "-i DUPENVTEST=goodifseen '$test_prog'",
    env    => { DUPENVTEST => 'badifseen' },
    stdout => qr/(?m)^DUPENVTEST=goodifseen$/
);

$o = $cmd->run(
    args   => "DUPENVTEST=goodifseen DUPENVTEST=alsogoodifseen '$test_prog'",
    stdout => qr/(?m)^DUPENVTEST=goodifseen$/
);
ok($o->stdout =~ m/(?m)^DUPENVTEST=alsogoodifseen$/);

# so very wrong
$cmd->run(
    args   => "-i =nameless '$test_prog'",
    status => 1,
    stderr => qr/invalid/,
);

# but allowed! ... with a flag
$cmd->run(
    args   => "-U -i =nameless '$test_prog'",
    stdout => qr/^=nameless$/,
);

# look for realloc bugs around the default newenv_alloc value starting
# from a clean slate environment
{
    my $env_count = 62;

    my @envspam = map { sprintf "ENVSPAM=%02d", $_ } 1 .. $env_count;

    for my $i (1 .. 4) {
        push @envspam, sprintf "ENVSPAM=%02d", $env_count + $i;

        $cmd->run(
            args   => "'$test_prog' -i " . join(' ', @envspam),
            stdout => \@envspam
        );
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

    $o = $cmd->run(args => $test_prog, stdout => qr/^/,);

    chomp(my @default_env = $o->stdout);

    # this may false postive if the env is cluttered...
    BAIL_OUT("too many env vars set by sh??") if @default_env > 59;

    my $env_count = 60 - @default_env;
    my @envspam   = map { sprintf "ENVSPAM=%02d", $_ } 1 .. $env_count;

    for my $i (1 .. 5) {
        push @envspam, sprintf "ENVSPAM=%02d", $env_count + $i;

        $cmd->run(
            args   => "'$test_prog' " . join(' ', @envspam),
            stdout => [ @default_env, @envspam ],
        );
    }
}

$cmd->run(args => '-h', status => 64, stderr => qr/Usage/);

done_testing(66);
