#!perl
use lib qw(lib/perl5);
use UtilityTestBelt;

my $test_prog = './todo';

my @both = qw(EDITOR VISUAL);
delete @ENV{@both};
my $one = $both[ rand @both ];
$ENV{$one} = 'echo';

# check if HOME has been corrupted (which the m4 inserts into the code)
# by comparing it against the password database
my $home  = ( getpwuid $> )[7];
my @tests = ( { stdout => qr(^$home/todo$) }, );
my $cmd   = Test::Cmd->new( prog => $test_prog, workdir => '', );

for my $test (@tests) {
    $test->{status} //= 0;
    $test->{stderr} //= qr/^$/;
    $cmd->run;
    exit_is( $?, $test->{status}, "STATUS $test_prog" );
    ok( $cmd->stdout =~ m/$test->{stdout}/, "STDOUT $test_prog" )
      or diag 'STDOUT ' . $cmd->stdout;
    ok( $cmd->stderr =~ m/$test->{stderr}/, "STDERR $test_prog" )
      or diag 'STDERR ' . $cmd->stderr;
}
done_testing( @tests * 3 );
