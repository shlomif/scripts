#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $cmd = Test::UnixCmdWrap->new;

# NOTE these must be quoted for the shell Test::Cmd runs things through
$cmd->run(
    args   => qq{'puts("PID $$")'},
    stdout => ["PID $$"],
);
$cmd->run(
    args => qq{-E '$$ "a b" 42' }
      . quotemeta 'printf("%s,%s,%s\n",*(argv+1),*(argv+2),*(argv+3))',
    stdout => ["$$,a b,42"],
);
# globals must now be quoted as that way you can also write functions
$cmd->run(
    args   => qq{-G 'const char *pid="PID";' 'printf("%s $$", pid)'},
    stdout => ["PID $$"],
);

$cmd->run(args => '-h', status => 64, stderr => qr/Usage/);

# this output will vary wildly by platform but hopefully the string
# should appear exactly somewhere. stderr ignored as might have various
# warnings (or, otoh, actual errors hence it being displayed)
my $o = $cmd->run(
    args   => qq{-S 'puts("grepfor$$")'},
    stdout => qr/grepfor$$/,
    stderr => qr/^/
);
my $err = $o->stderr;
diag 'STDERR? ' . $err if defined $err and length $err;

done_testing(15);
