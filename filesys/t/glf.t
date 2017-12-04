#!perl

use 5.14.0;
use warnings;
use File::Spec;
use File::Temp qw(tempdir);
use Test::Cmd;
# 3 tests per item in @tests plus any extras
use Test::Most tests => 3 * 4 + 2;
use Test::UnixExit;

my $test_prog = './glf';

my $test_epoch = 915148800;
my $test_dir = tempdir( "glf-t.XXXXXXXXX", CLEANUP => 1, TMPDIR => 1 );

my @test_files = ( [ 'test', 0 ], [ 'newer', 300 ], [ 'older', -900 ] );

for my $tf (@test_files) {
    $tf->[0] = File::Spec->catfile( $test_dir, $tf->[0] );
    open my $fh, '>', $tf->[0], or die "could not create $tf->[0]: $!\n";
    $tf->[1] += $test_epoch;
    utime $tf->[1], $tf->[1], $tf->[0];
}

my @tests = (
    {   chdir  => $test_dir,
        stdout => ['newer'],
    },
    {   args   => q{-e '^new'},
        chdir  => $test_dir,
        stdout => ['test'],
    },
    {   args   => q{'^old'},
        chdir  => $test_dir,
        stdout => ['older'],
    },
    {   args   => "'^' $test_dir",
        stdout => ['newer'],
    },
);
my $testcmd = Test::Cmd->new(
    prog    => $test_prog,
    verbose => 0,
    workdir => '',
);

for my $test (@tests) {
    $test->{exit_status} //= 0;
    $test->{stderr}      //= '';

    $testcmd->run(
        exists $test->{args}  ? ( args  => $test->{args} )  : (),
        exists $test->{chdir} ? ( chdir => $test->{chdir} ) : ()
    );

    $test->{args} //= '';
    exit_is( $?, $test->{exit_status}, "STATUS $test_prog $test->{args}" );
    eq_or_diff( [ map { s/\s+$//r } split $/, $testcmd->stdout ],
        $test->{stdout}, "STDOUT $test_prog $test->{args}" );
    is( $testcmd->stderr, $test->{stderr}, "STDERR $test_prog $test->{args}" );
}

# any extras

$testcmd->run( args => '-h' );
exit_is( $?, 64, "EX_USAGE of sysexits(3) fame" );
ok( $testcmd->stderr =~ m/Usage/, "help mentions usage" );
