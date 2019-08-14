#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;

my $cmd = Test::UnixCmdWrap->new;

my $test_epoch     = 915148800;
my $test_dir       = tempdir("fbd-t.XXXXXXXXX", CLEANUP => 1, TMPDIR => 1);
my $test_dir_epoch = $test_epoch - 86400;

my @test_files = (
    [ 'test',        0 ],
    [ 'newer',       300 ],
    [ 'older',       -900 ],
    [ 'false-early', -3605 ],
    [ 'false-late',  3605 ],
);
for my $tf (@test_files) {
    $tf->[0] = catfile($test_dir, $tf->[0]);
    open my $fh, '>', $tf->[0], or die "could not create $tf->[0]: $!\n";
    $tf->[1] += $test_epoch;
    # atime is offset from mtime so can (maybe) test that
    utime $tf->[1] - 3605, $tf->[1], $tf->[0] or die "utime failed: $!\n";
}
utime $test_dir_epoch, $test_dir_epoch, $test_dir;

$cmd->run(
    args   => "epoch $test_epoch '$test_dir'",
    stdout => [ $test_files[0][0] ],
);

# TODO may need env flag to disable atime tests if filesystem sets noatime
my $atime_epoch = $test_epoch - 3605;
$cmd->run(
    args   => "-s a epoch $atime_epoch '$test_dir'",
    stdout => [ $test_files[0][0] ],
);

$cmd->run(
    args   => "file $test_files[0][0] '$test_dir'",
    stdout => [ $test_files[0][0] ],
);

$cmd->run(
    args   => "epoch $test_epoch *",
    chdir  => $test_dir,
    stdout => ['test'],
);

$cmd->run(
    args   => "epoch $test_epoch no-such-file-$$",
    chdir  => "/var/empty",
    stderr => qr/failed to stat/,
    status => 77,                                 # EX_NOPERM
);

# fts(3) may not sort the file listings into any particular order. this
# complicates checking stdout
my $o;

# was minutes for a raw number but that was too surprising when
# wrote the tests so seconds now
$o = $cmd->run(
    args   => "-a 600 epoch $test_epoch",
    chdir  => $test_dir,
    stdout => qr/^/,
);
eq_or_diff([ sort map { s/\s+//r } split $/, $o->stdout ],
    [qw(./newer ./test)]);

$o = $cmd->run(
    args   => "-a 6m epoch $test_epoch",
    chdir  => $test_dir,
    stdout => qr/^/,
);
eq_or_diff([ sort map { s/\s+//r } split $/, $o->stdout ],
    [qw(./newer ./test)]);

$o = $cmd->run(
    args   => "-a 1h epoch $test_epoch",
    chdir  => $test_dir,
    stdout => qr/^/,
);
eq_or_diff([ sort map { s/\s+//r } split $/, $o->stdout ],
    [qw(./newer ./older ./test)]);

# only those before...
$o = $cmd->run(
    args   => "-b 1h -0 epoch $test_epoch",
    chdir  => $test_dir,
    stdout => qr/^/,
);
eq_or_diff([ sort map { s/\s+//r } split "\0", $o->stdout ],
    [qw(./older ./test)]);

# -b with -a makes -a "after", not "around"
$o = $cmd->run(
    args   => "-b 5m -a 1h epoch $test_epoch .",
    chdir  => $test_dir,
    stdout => qr/^/,
);
eq_or_diff([ sort map { s/\s+//r } split $/, $o->stdout ],
    [qw(./newer ./test)]);

chdir $test_dir or die "chdir failed?? $!";

# change time should be recent for all the test files, despite mtime and
# (maybe) atime getting pushed back
diag "sleeping to gain distance for ctim tests...";
sleep 5;

# will fail if the system is slow. very, very slow. or if the system
# time has been set to around about $test_epoch
my $now = time();
link "test", "test2" or die "link failed?? $!";
$o = $cmd->run(args => "-a 4 -s c epoch $now", stdout => qr/^/);
eq_or_diff([ sort map { s/\s+//r } split $/, $o->stdout ],
    [qw(. ./test ./test2)]);

# TODO need tests for -L vs. -P and also -x (hmm) and maybe -q

# TODO need a looped filesystem test but some OS (and also filesystems)
# are annoying and do not let you (easily) create such loops

$cmd->run(args => '-h', status => 64, stderr => qr/Usage/);

done_testing
