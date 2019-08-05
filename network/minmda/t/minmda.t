#!perl
use lib qw(../../lib/perl5);
use UtilityTestBelt;
use File::Cmp qw(fcmp);

# these should be kept in sync with ../minmda.h
sub MBUF_MAX ()     { 8192 }
sub MEXIT_STATUS () { 75 }

my $test_prog = './minmda';
my $test_dir  = tempdir("minmda.XXXXXXXXX", CLEANUP => 1, TMPDIR => 1);

my $cmd = Test::Cmd->new(prog => $test_prog, workdir => '',);

# invalid arguments
{
    $cmd->run(chdir => $test_dir);
    is($cmd->stdout, "");
    ok($cmd->stderr =~ m/Usage/, "usage error");
    exit_is($?, MEXIT_STATUS, "standard tmp exit code");

    $cmd->run(args => "bad1 hostid toofar", chdir => $test_dir);
    is($cmd->stdout, "");
    ok($cmd->stderr =~ m/Usage/, "usage error");
    exit_is($?, MEXIT_STATUS, "standard tmp exit code");

    # ... and should not create any directories
    ok(!-e File::Spec->catdir($test_dir, 'bad1'));

    # mailbox-dir must be something
    $cmd->run(args => "'' hostid", chdir => $test_dir);
    is($cmd->stdout, "");
    ok($cmd->stderr =~ m/invalid mailbox-dir$/);
    exit_is($?, MEXIT_STATUS, "standard tmp exit code");

    # also host-id must be set to something
    $cmd->run(args => "bad2 ''", chdir => $test_dir);
    is($cmd->stdout, "");
    ok($cmd->stderr =~ m/invalid empty host-id$/);
    exit_is($?, MEXIT_STATUS, "standard tmp exit code");

    # ... and should not create any directories
    ok(!-e File::Spec->catdir($test_dir, 'bad2'));
}

# no input data - either if nothing sent or if only newlines sent
{
    $cmd->run(args => "oops nope", chdir => $test_dir, stdin => "",);
    is($cmd->stdout, "");
    ok($cmd->stderr =~ m/read no data from stdin$/)
      or diag 'STDERR ' . $cmd->stderr;
    exit_is($?, MEXIT_STATUS, "standard tmp exit code");

    # this test *should* create the dirs, as need somewhere to write
    for ([qw/oops/], [qw/oops cur/], [qw/oops new/], [qw/oops tmp/]) {
        my $dir = File::Spec->catdir($test_dir, @$_);
        ok(-e $dir, "exists '$dir'");
    }

    my @found = glob(File::Spec->catfile($test_dir, 'oops', '*', '*.nope'));
    ok(@found == 0, "no file created but found @found");
    unlink(@found) if @found;

    # only newlines more than the buffer size involves various edge
    # cases in the code
    $cmd->run(
        args  => "oops hostid",
        chdir => $test_dir,
        stdin => "\n" x (42 + MBUF_MAX),
    );
    is($cmd->stdout, "");
    ok($cmd->stderr =~ m/read no data from stdin$/);
    exit_is($?, MEXIT_STATUS, "standard tmp exit code");

    @found = glob(File::Spec->catfile($test_dir, 'oops', '*', '*.nope'));
    ok(@found == 0, "no file created but found @found");
}

my $mailbox = File::Spec->catdir($test_dir, 'inbox');
my $curdir  = File::Spec->catdir($mailbox,  'new');

my $message = "blah blah blah";

{
    $cmd->run(
        args  => "'$mailbox' '///-$$'",
        chdir => $test_dir,
        stdin => $message,
    );
    is($cmd->stdout, "");
    is($cmd->stderr, "");
    exit_is($?, 0);

    # / are somewhat illegal in filenames and must be mangled by minmda
    my $mailfile = glob(File::Spec->catfile($curdir, "*.___-$$"));
    open my $fh, '<', $mailfile or diag "could not open '$mailfile': $!\n";
    is(do { local $/; readline $fh }, $message);
    unlink($mailfile);
}

# leading blank line(s) should be stripped (maildrop does something
# like this)
{
    $cmd->run(
        args  => "'$mailbox' 'strip-$$'",
        chdir => $test_dir,
        stdin => "\n\n\n" . $message,
    );
    is($cmd->stdout, "");
    is($cmd->stderr, "");
    exit_is($?, 0);

    my $mailfile = glob(File::Spec->catfile($curdir, "*.strip-$$"));
    open my $fh, '<', $mailfile or diag "could not open '$mailfile': $!\n";
    is(do { local $/; readline $fh }, $message);
    unlink($mailfile);
}

# ... however runs of blanks after a non-blank line must not be stripped
{
    $cmd->run(
        args  => "'$mailbox' 'nostrip-$$'",
        chdir => $test_dir,
        stdin => $message . "\n\n\n",
    );
    is($cmd->stdout, "");
    is($cmd->stderr, "");
    exit_is($?, 0);

    my $mailfile = glob(File::Spec->catfile($curdir, "*.nostrip-$$"));
    open my $fh, '<', $mailfile or diag "could not open '$mailfile': $!\n";
    is(do { local $/; readline $fh }, $message . "\n\n\n");
    unlink($mailfile);
}

# a longer message - bigger than MBUF_MAX. hopefully.
{
    my $long_mfile = 't/longmsg';
    open my $fh, '<', $long_mfile || BAIL_OUT("could not open $long_mfile: $!");
    my $longmsg = do { local $/; readline $fh };

    $cmd->run(
        args  => "'$mailbox' 'cla-$$'",
        chdir => $test_dir,
        stdin => $longmsg,
    );

    is($cmd->stdout, "");
    is($cmd->stderr, "");
    exit_is($?, 0);

    my $mailfile = glob(File::Spec->catfile($curdir, "*.cla-$$"));
    ok(fcmp($mailfile, $long_mfile, binmode => ':raw'));
    unlink($mailfile);
}

# corruption
{
    $cmd->prog('./minmda-corrupts');

    $cmd->run(
        args  => "'$mailbox' 'bla-$$'",
        chdir => $test_dir,
        stdin => $message,
    );
    is($cmd->stdout, "");
    ok($cmd->stderr =~ m/corrupting the output file/);
    ok($cmd->stderr =~ m/failed to verify written data/);
    ok($cmd->stderr !~ m/corruption did not happen/);
    exit_is($?, MEXIT_STATUS, "standard tmp exit code");

    $cmd->prog($test_prog);
}

done_testing(47);
