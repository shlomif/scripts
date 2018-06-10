#!perl
use lib qw(../../lib/perl5);
use UtilityTestBelt;

my $test_prog = './tally';

my $testcmd = Test::Cmd->new(
    prog    => $test_prog,
    verbose => 0,
    workdir => '',
);

$testcmd->run( args => '-h' );
ok( $testcmd->stderr =~ m/Usage: / );
is( $testcmd->stdout, "" );
exit_is( $?, 64, "EX_USAGE" );

# no input is no foul; perhaps it should instead signal an error?
# (perhaps via an -A abort flag?)
$testcmd->run();
is( $testcmd->stderr, "" );
is( $testcmd->stdout, "" );
exit_is( $?, 0, "okay" );

$testcmd->run( args => 't/tally-input' );
is( $testcmd->stderr, "" );
is( $testcmd->stdout, "2 foo\n1 bar\n" );
exit_is( $?, 0, "okay" );

$testcmd->run( args => 't/tally-input t/tally-input' );
is( $testcmd->stderr, "" );
is( $testcmd->stdout, "4 foo\n2 bar\n" );
exit_is( $?, 0, "okay" );

$testcmd->run( stdin => <<'EOF' );
bar
foo
foo
EOF
is( $testcmd->stderr, "" );
is( $testcmd->stdout, "2 foo\n1 bar\n" );
exit_is( $?, 0, "okay" );

my $input = '';
my %tally;
for ( 1 .. 100 ) {
    my $n = int( 3 + rand 6 + rand 6 + rand 6 );
    $input .= $n . $/;
    $tally{$n}++;
}
$testcmd->run( stdin => $input );
is( $testcmd->stderr, "" );
# NOTE must use the same sort on both outputs as qsort(3) may sort lines
# that appear the same number of times differently than a Perl or shell
# implementation (the C only sorts by the counts and does not subsort by
# the content of the line)
eq_or_diff(
    [ sort split $/, $testcmd->stdout ],
    [ sort map { "$tally{$_} $_" } keys %tally ]
);
exit_is( $?, 0, "okay" );

done_testing(18);
