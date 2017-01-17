#!perl

use 5.14.0;
use warnings;
use File::Spec ();
use File::Temp qw(tempdir);
use File::Which;
use Test::Cmd;
use Test::Most tests => 16;
use Test::UnixExit;

my $test_prog = 'di';

my $test_dir = tempdir( 'di.XXXXXXXXXX', CLEANUP => 1, TMPDIR => 1 );

my $testcmd = Test::Cmd->new(
    prog    => $test_prog,
    verbose => 0,
    workdir => '',
);

ok( chdir($test_dir), "cd $test_dir" );

$testcmd->run();
exit_is( $?, 64, "EX_USAGE of sysexits(3) fame" );
ok( $testcmd->stderr =~ m/Usage/, "help mentions usage" );

my $fh;
open $fh, '>', 'asdf' or die "could not open 'asdf': $!\n";
say $fh "new$$";
open $fh, '>', 'asdf.old' or die "could not open 'asdf.old': $!\n";
say $fh "old$$";
close $fh;

$testcmd->run( args => 'asdf' );
# NOTE that diff(1) output is not portable (though it's been years since
# the bad old days of `diff -u` not being supported...)
ok( $testcmd->stdout =~ m/new$$/, "diff reported new line" );
ok( $testcmd->stdout =~ m/old$$/, "diff reported old line" );
is( $testcmd->stderr, "", "no stderr" );
exit_is( $?, 0, "exit ok" );

ok( unlink("asdf.old"), "remove old file" );

SKIP: {
    # NOTE only tested on OpenBSD cvs(1) not other implementations
    my $cvs = which('cvs');
    skip 'cvs not found in PATH', 4 unless $cvs;

    my $repo_dir = tempdir( 'di-cvs.XXXXXXXXXX', CLEANUP => 1, TMPDIR => 1 );

    $ENV{CVSROOT} = $repo_dir;

    qx"$cvs -q init && $cvs -q import -m bla . VENDOR start && cvs -q checkout .";
    exit_is( $?, 0, "cvs commands exit ok" );

    open $fh, '>>', 'asdf' or die "could not open 'asdf': $!\n";
    say $fh "cvs$$";
    close $fh;

    $testcmd->run( args => 'asdf' );
    ok( $testcmd->stdout =~ m/cvs$$/, "diff reports cvs line" );
    is( $testcmd->stderr, "", "no stderr" );
    exit_is( $?, 1, "KLUGE exit for cvs diff is 1" );

    # NOTE there is no cleanup for CVS despite reusing same dir as in
    # current code git should be checked for first
}

SKIP: {
    my $git = which('git');
    skip 'git not found in PATH', 4 unless $git;

    qx"$git init && $git add asdf && $git commit -m asdf asdf";
    exit_is( $?, 0, "git commands exit ok" );

    open $fh, '>>', 'asdf' or die "could not open 'asdf': $!\n";
    say $fh "git$$";
    close $fh;

    $testcmd->run( args => 'asdf' );
    ok( $testcmd->stdout =~ m/git$$/, "diff reports git line" );
    is( $testcmd->stderr, "", "no stderr" );
    exit_is( $?, 0, "exit ok" );
}
