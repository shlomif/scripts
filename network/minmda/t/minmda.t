#!perl
use lib qw(../../lib/perl5);
use UtilityTestBelt;
use File::Cmp qw(fcmp);

# these should be kept in sync with ../minmda.h
sub MBUF_MAX ()     { 8192 }
sub MEXIT_STATUS () { 75 }

my $cmd      = Test::UnixCmdWrap->new;
my $test_dir = tempdir("minmda.XXXXXXXXX", CLEANUP => 1, TMPDIR => 1);

# invalid arguments
{
    $cmd->run(
        chdir  => $test_dir,
        stderr => qr/Usage/,
        status => MEXIT_STATUS,
    );

    $cmd->run(
        args   => "bad1 hostid toofar",
        chdir  => $test_dir,
        stderr => qr/Usage/,
        status => MEXIT_STATUS,
    );

    # ... that must not create any directories
    ok(!-e catdir($test_dir, 'bad1'));

    # mailbox-dir must be something
    $cmd->run(
        args   => "'' hostid",
        chdir  => $test_dir,
        stderr => qr/invalid mailbox-dir$/,
        status => MEXIT_STATUS,
    );

    # also host-id must be set to something
    $cmd->run(
        args   => "bad2 ''",
        chdir  => $test_dir,
        stderr => qr/invalid empty host-id$/,
        status => MEXIT_STATUS,
    );

    # ... that still must not create any directories
    ok(!-e catdir($test_dir, 'bad2'));
}

# no input data - either if nothing sent or if only newlines sent
{
    $cmd->run(
        args   => "oops nope",
        chdir  => $test_dir,
        stdin  => "",
        stderr => qr/read no data from stdin$/,
        status => MEXIT_STATUS,
    );

    # this test *should* create the dirs, as need somewhere to write
    for ([qw/oops/], [qw/oops cur/], [qw/oops new/], [qw/oops tmp/]) {
        my $dir = catdir($test_dir, @$_);
        ok(-e $dir, "exists '$dir'");
    }

    my @found = glob(catfile($test_dir, 'oops', '*', '*.nope'));
    ok(@found == 0, "no file created but found @found");
    unlink(@found) if @found;

    # only newlines more than the buffer size involves various edge
    # cases in the code
    $cmd->run(
        args   => "oops hostid",
        chdir  => $test_dir,
        stdin  => "\n" x (42 + MBUF_MAX),
        stderr => qr/read no data from stdin$/,
        status => MEXIT_STATUS,
    );

    @found = glob(catfile($test_dir, 'oops', '*', '*.nope'));
    ok(@found == 0, "no file created but found @found");
}

my $mailbox = catdir($test_dir, 'inbox');
my $curdir  = catdir($mailbox,  'new');

my $message = "blah blah blah";

{
    $cmd->run(
        args  => "'$mailbox' '///-$$'",
        chdir => $test_dir,
        stdin => $message,
    );

    # / are somewhat illegal in filenames and must be mangled by minmda
    my $mailfile = glob(catfile($curdir, "*.___-$$"));
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

    my $mailfile = glob(catfile($curdir, "*.strip-$$"));
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

    my $mailfile = glob(catfile($curdir, "*.nostrip-$$"));
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

    my $mailfile = glob(catfile($curdir, "*.cla-$$"));
    ok(fcmp($mailfile, $long_mfile, binmode => ':raw'));
    unlink($mailfile);
}

# colluption NOTE requires that `make minmda-corrupts` is run before
# `prove t/minmda.t` or instead use `make test`
{
    $cmd = Test::UnixCmdWrap->new(cmd => './minmda-corrupts');
    my $o = $cmd->run(
        args   => "'$mailbox' 'bla-$$'",
        chdir  => $test_dir,
        stdin  => $message,
        stderr => qr/failed to verify written data/,
        status => MEXIT_STATUS,
    );
    ok($o->stderr !~ m/corruption did not happen/);
}

done_testing(46);
