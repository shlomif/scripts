#!perl

use 5.14.0;
use warnings;
use File::Cmp qw(fcmp);
use File::Spec ();
use File::Temp qw(tempdir);
use Test::Cmd;
use Test::Most tests => 47;
use Test::UnixExit;

# these should be kept in sync with ../minmda.h
sub MBUF_MAX ()     { 8192 }
sub MEXIT_STATUS () { 75 }

my $test_prog = './minmda';
my $test_dir = tempdir( "minmda.XXXXXXXXX", CLEANUP => 1, TMPDIR => 1 );

my $testcmd = Test::Cmd->new(
    prog    => $test_prog,
    verbose => 0,
    workdir => '',
);

# invalid arguments
{
    $testcmd->run( chdir => $test_dir );
    is( $testcmd->stdout, "" );
    ok( $testcmd->stderr =~ m/Usage/, "usage error" );
    exit_is( $?, MEXIT_STATUS, "standard tmp exit code" );

    $testcmd->run( args => "bad1 hostid toofar", chdir => $test_dir );
    is( $testcmd->stdout, "" );
    ok( $testcmd->stderr =~ m/Usage/, "usage error" );
    exit_is( $?, MEXIT_STATUS, "standard tmp exit code" );

    # ... and should not create any directories
    ok( !-e File::Spec->catdir( $test_dir, 'bad1' ) );

    # mailbox-dir must be something
    $testcmd->run( args => "'' hostid", chdir => $test_dir );
    is( $testcmd->stdout, "" );
    ok( $testcmd->stderr =~ m/invalid mailbox-dir$/ );
    exit_is( $?, MEXIT_STATUS, "standard tmp exit code" );

    # also host-id must be set to something
    $testcmd->run( args => "bad2 ''", chdir => $test_dir );
    is( $testcmd->stdout, "" );
    ok( $testcmd->stderr =~ m/invalid empty host-id$/ );
    exit_is( $?, MEXIT_STATUS, "standard tmp exit code" );

    # ... and should not create any directories
    ok( !-e File::Spec->catdir( $test_dir, 'bad2' ) );
}

# no input data - either if nothing sent or if only newlines sent
{
    $testcmd->run( args => "oops nope", chdir => $test_dir, stdin => "", );
    is( $testcmd->stdout, "" );
    ok( $testcmd->stderr =~ m/read no data from stdin$/ );
    exit_is( $?, MEXIT_STATUS, "standard tmp exit code" );

    # this test *should* create the dirs, as need somewhere to write
    for ( [qw/oops/], [qw/oops cur/], [qw/oops new/], [qw/oops tmp/] ) {
        my $dir = File::Spec->catdir( $test_dir, @$_ );
        ok( -e $dir, "exists '$dir'" );
    }

    my @found = glob( File::Spec->catfile( $test_dir, 'oops', '*', '*.nope' ) );
    ok( @found == 0, "no file created but found @found" );
    unlink(@found) if @found;

    # only newlines more than the buffer size involves various edge
    # cases in the code
    $testcmd->run(
        args  => "oops hostid",
        chdir => $test_dir,
        stdin => "\n" x ( 42 + MBUF_MAX ),
    );
    is( $testcmd->stdout, "" );
    ok( $testcmd->stderr =~ m/read no data from stdin$/ );
    exit_is( $?, MEXIT_STATUS, "standard tmp exit code" );

    @found = glob( File::Spec->catfile( $test_dir, 'oops', '*', '*.nope' ) );
    ok( @found == 0, "no file created but found @found" );
}

my $mailbox = File::Spec->catdir( $test_dir, 'inbox' );
my $curdir  = File::Spec->catdir( $mailbox,  'new' );

my $message = "blah blah blah";

{
    $testcmd->run(
        args  => "'$mailbox' 'bla-$$'",
        chdir => $test_dir,
        stdin => $message,
    );
    is( $testcmd->stdout, "" );
    is( $testcmd->stderr, "" );
    exit_is( $?, 0 );

    my $mailfile = glob( File::Spec->catfile( $curdir, "*.bla-$$" ) );
    open my $fh, '<', $mailfile or diag "could not open '$mailfile': $!\n";
    is( do { local $/; readline $fh }, $message );
    unlink($mailfile);
}

# leading blank line(s) should be stripped (maildrop does something
# like this)
{
    $testcmd->run(
        args  => "'$mailbox' 'strip-$$'",
        chdir => $test_dir,
        stdin => "\n\n\n" . $message,
    );
    is( $testcmd->stdout, "" );
    is( $testcmd->stderr, "" );
    exit_is( $?, 0 );

    my $mailfile = glob( File::Spec->catfile( $curdir, "*.strip-$$" ) );
    open my $fh, '<', $mailfile or diag "could not open '$mailfile': $!\n";
    is( do { local $/; readline $fh }, $message );
    unlink($mailfile);
}

# ... however runs of blanks after a non-blank line must not be stripped
{
    $testcmd->run(
        args  => "'$mailbox' 'nostrip-$$'",
        chdir => $test_dir,
        stdin => $message . "\n\n\n",
    );
    is( $testcmd->stdout, "" );
    is( $testcmd->stderr, "" );
    exit_is( $?, 0 );

    my $mailfile = glob( File::Spec->catfile( $curdir, "*.nostrip-$$" ) );
    open my $fh, '<', $mailfile or diag "could not open '$mailfile': $!\n";
    is( do { local $/; readline $fh }, $message . "\n\n\n" );
    unlink($mailfile);
}

# a longer message - bigger than MBUF_MAX. hopefully.
{
    my $long_mfile = 't/longmsg';
    open my $fh, '<', $long_mfile || BAIL_OUT("could not open $long_mfile: $!");
    my $longmsg = do { local $/; readline $fh };

    $testcmd->run(
        args  => "'$mailbox' 'cla-$$'",
        chdir => $test_dir,
        stdin => $longmsg,
    );

    is( $testcmd->stdout, "" );
    is( $testcmd->stderr, "" );
    exit_is( $?, 0 );

    my $mailfile = glob( File::Spec->catfile( $curdir, "*.cla-$$" ) );
    ok( fcmp( $mailfile, $long_mfile, binmode => ':raw' ) );
    unlink($mailfile);
}

# corruption
{
    $testcmd->prog('./minmda-corrupts');

    $testcmd->run(
        args  => "'$mailbox' 'bla-$$'",
        chdir => $test_dir,
        stdin => $message,
    );
    is( $testcmd->stdout, "" );
    ok( $testcmd->stderr =~ m/corrupting the output file/ );
    ok( $testcmd->stderr =~ m/failed to verify written data/ );
    ok( $testcmd->stderr !~ m/corruption did not happen/ );
    exit_is( $?, MEXIT_STATUS, "standard tmp exit code" );

    $testcmd->prog($test_prog);
}

# Tekeli-li! Tekeli-li!
