#!perl

use 5.14.0;
use warnings;
use File::Spec ();
use File::Temp qw(tempdir);
use Test::Cmd;
use Test::Most tests => 11;
use Test::UnixExit;

my $test_prog = 'ketchup';

my $test_dir = tempdir( 'ketchup.XXXXXXXXXX', CLEANUP => 1, TMPDIR => 1 );

my $testcmd = Test::Cmd->new(
    prog    => $test_prog,
    verbose => 0,
    workdir => '',
);

ok( chdir($test_dir), "cd $test_dir" );

$testcmd->run( args => '-h' );
exit_is( $?, 64, "EX_USAGE of sysexits(3) fame" );
ok( $testcmd->stderr =~ m/Usage/, "help mentions usage" );

my $fh;

SKIP: {
    # NOTE only tested on OpenBSD cvs(1) not other implementations
    my $cvs = whereis('cvs');
    skip 'cvs not found in PATH', 4 unless $cvs;

    my $repo_dir = tempdir( 'ketchup-cvs.XXXXXXXXXX', CLEANUP => 1, TMPDIR => 1 );

    $ENV{CVSROOT} = $repo_dir;

    qx"$cvs -q init && $cvs -q import -m bla . VENDOR start && cvs -q checkout .";
    exit_is( $?, 0, "cvs commands exit ok" );

    open $fh, '>>', 'asdf' or die "could not open 'asdf': $!\n";
    say $fh "cvs$$";
    close $fh;

    $testcmd->run();
    ok( $testcmd->stdout =~ m/^\?\s+asdf/m, "CVS modification noted" );
    is( $testcmd->stderr, "", "no noise from cvs -q update" );
    exit_is( $?, 0, "ketchup exit ok for CVS" );

    # NOTE there is no cleanup for CVS despite reusing same dir as in
    # current code git should be checked for first
}

SKIP: {
    my $git = whereis('git');
    skip 'git not found in PATH', 4 unless $git;

    qx"$git init && $git add asdf && $git commit -m asdf asdf";
    exit_is( $?, 0, "git commands exit ok" );

    open $fh, '>>', 'asdf' or die "could not open 'asdf': $!\n";
    say $fh "git$$";
    close $fh;

    $testcmd->run();
    ok( $testcmd->stdout =~ m/^\s+modified:\s+asdf/m, "modifiction noted" );
    is( $testcmd->stderr, "", "no stderr" );
    exit_is( $?, 0, "exit ok" );
}

sub whereis {
    my ($what) = @_;
    my $path;
    for my $dir ( split ':', $ENV{PATH} ) {
        my $tmp = File::Spec->catfile( $dir, $what );
        if ( -x $tmp ) {
            $path = $tmp;
            last;
        }
    }
    return $path;
}
