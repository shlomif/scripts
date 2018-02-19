#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $test_prog = './fbd';

my $test_epoch     = 915148800;
my $test_dir       = tempdir( "fbd-t.XXXXXXXXX", CLEANUP => 1, TMPDIR => 1 );
my $test_dir_epoch = $test_epoch - 86400;

my @test_files = (
    [ 'test',        0 ],
    [ 'newer',       300 ],
    [ 'older',       -900 ],
    [ 'false-early', -3605 ],
    [ 'false-late',  3605 ],
);

for my $tf (@test_files) {
    $tf->[0] = File::Spec->catfile( $test_dir, $tf->[0] );
    open my $fh, '>', $tf->[0], or die "could not create $tf->[0]: $!\n";
    $tf->[1] += $test_epoch;
    utime $tf->[1], $tf->[1], $tf->[0];
}
utime $test_dir_epoch, $test_dir_epoch, $test_dir;

my $testcmd = Test::Cmd->new( prog => $test_prog, workdir => '', );

# NOTE a File::Find bug may cause the mtime of the files to incorrectly
# be that of the parent directory. check for that condition first, as if
# present it will make a bunch of the regular tests mysteriously fail
$testcmd->run( args => "-e $test_dir_epoch -a 5m", chdir => $test_dir );

is( $testcmd->stdout, "", "cached mtime of parent dir afflicts subfiles" )
  or die "abort remainder of tests due to File::Find issue\n";

my @tests = (
    {   args   => "-e $test_epoch '$test_dir'",
        stdout => [ $test_files[0][0] ],
    },
    {   args   => "-f $test_files[0][0] '$test_dir'",
        stdout => [ $test_files[0][0] ],
    },
    {   args   => "-e $test_epoch *",
        chdir  => $test_dir,
        stdout => ['./test'],
    },
    # was minutes for a raw number but that was too surprising when
    # wrote the tests so seconds now
    {   args   => "-a 600 -e $test_epoch",
        chdir  => $test_dir,
        stdout => [ './newer', './test' ],
    },
    {   args   => "-a 6m -e $test_epoch",
        chdir  => $test_dir,
        stdout => [ './newer', './test' ],
    },
    {   args   => "-a 1h -e $test_epoch",
        chdir  => $test_dir,
        stdout => [ './newer', './older', './test' ],
    },
    # only those before...
    {   args   => "-b 1h -e $test_epoch",
        chdir  => $test_dir,
        stdout => [ './older', './test' ],
    },
    # -b with -a makes -a "after", not "around"
    {   args   => "-b 5m -a 1h -e $test_epoch",
        chdir  => $test_dir,
        stdout => [ './newer', './test' ],
    },
);

for my $test (@tests) {
    $test->{exit_status} //= 0;
    $test->{stderr}      //= '';

    $testcmd->run(
        args => $test->{args},
        exists $test->{chdir} ? ( chdir => $test->{chdir} ) : ()
    );

    exit_is( $?, $test->{exit_status}, "STATUS $test_prog $test->{args}" );
    # NOTE sort as cannot assume what order the files will be in
    eq_or_diff( [ sort map { s/\s+$//r } split $/, $testcmd->stdout ],
        $test->{stdout}, "STDOUT $test_prog $test->{args}" );
    is( $testcmd->stderr, $test->{stderr}, "STDERR $test_prog $test->{args}" );
}
$testcmd->run( args => '-h' );
exit_is( $?, 64, "EX_USAGE of sysexits(3) fame" );
ok( $testcmd->stderr =~ m/Usage/, "help mentions usage" );
done_testing( @tests * 3 + 3 );
