#!perl

use 5.14.0;
use warnings;
use File::Spec ();
use File::Temp qw(tempdir);
use Test::Cmd;
# 4 tests per item in @tests plus any extras
use Test::Most tests => 4 * 2 + 2;
use Test::UnixExit;

my $test_prog = 'orig';

my $test_dir = tempdir( 'orig.XXXXXXXXXX', CLEANUP => 1, TMPDIR => 1 );

my @test_files = qw(foo bar);
for my $f (@test_files) {
    my $fqf = File::Spec->catfile( $test_dir, $f );
    open my $fh, '>', $fqf or die "could not create '$fqf': $!\n";
    $f = [ $f, $fqf ];
}

my @tests = (
    # fully qualified filenames
    {   args   => $test_files[0][1],
        isfile => "$test_files[0][1].orig",
    },
    # relative filename from dir itself
    {   args  => $test_files[1][0],
        chdir => $test_dir,
        # fully qualified as chdir on happens during test cmd
        isfile => "$test_files[1][1].orig",
    },
);
my $testcmd = Test::Cmd->new(
    prog    => $test_prog,
    verbose => 0,
    workdir => '',
);

for my $test (@tests) {
    $test->{exit_status} //= 0;
    $test->{stdout}      //= [];
    $test->{stderr}      //= '';

    $testcmd->run(
        args => $test->{args},
        exists $test->{chdir} ? ( chdir => $test->{chdir} ) : (),
    );

    exit_is( $?, $test->{exit_status}, "STATUS $test_prog $test->{args}" );
    eq_or_diff( [ map { s/\s+$//r } split $/, $testcmd->stdout ],
        $test->{stdout}, "STDOUT $test_prog $test->{args}" );
    is( $testcmd->stderr, $test->{stderr}, "STDERR $test_prog $test->{args}" );

    ok( -f $test->{isfile}, "$test_prog '$test->{args}' created file" );
}

# any extras

$testcmd->run();
exit_is( $?, 64, "EX_USAGE of sysexits(3) fame" );
ok( $testcmd->stderr =~ m/Usage/, "help mentions usage" );
