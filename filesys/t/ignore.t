#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;
use Cwd qw(getcwd realpath);
use File::Path::Tiny;
use File::Slurper qw(read_lines);

my $cmd = Test::UnixCmdWrap->new;

delete @ENV{qw(GIT_DIR GIT_CEILING_DIRECTORIES)};

my $test_dir = tempdir('ignore.XXXXXXXXXX', CLEANUP => 1, TMPDIR => 1);

chdir $test_dir or BAIL_OUT("could not chdir $test_dir: $!\n");
{
    mkdir '.git' or BAIL_OUT($!);
    my ($ret, $dir) = parent_git_dir($test_dir);
    is $ret, 1;
    rmdir '.git' or BAIL_OUT($!);
    ($ret, $dir) = parent_git_dir($test_dir);
    # KLUGE anything could after this runs `mkdir .git` above our test
    # directory...if this does fail because you have a .git dir in your
    # home directory and set TMPDIR below that, run the test using a
    # TMPDIR set to some path outside of your home dir
    is $ret, 0 or BAIL_OUT("parent .git dir found in $dir");

    ok !-e catfile($test_dir, '.gitignore')
      or BAIL_OUT('.gitignore already exists??');
}

$cmd->run(
    args   => 'unused',
    status => 1,
    stderr => qr/no repository found above/,
);
# must not be created on failure
ok !-e catfile($test_dir, '.gitignore');

my $git_dir = catdir($test_dir, qw(repo .git));
make_path($git_dir);
chdir $git_dir or BAIL_OUT("chdir $git_dir failed: $!");

ok !-e '.gitignore' or BAIL_OUT('.gitingore already exists??');

# Test::Cmd uses the shell hence the double quote
$cmd->run(args => q('*.core'));
ok -e '.gitignore';
eq_or_diff [ read_lines('.gitignore') ], ['*.core'];

# confirm that duplicate addition is squashed
$cmd->run(args => q('*.core'));
eq_or_diff [ read_lines('.gitignore') ], ['*.core'];

# more files and that the files are being sorted
$cmd->run(args => "zz.$$ MYMETA.yml MYMETA.json");
eq_or_diff [ read_lines('.gitignore') ],
  [ qw(*.core MYMETA.json MYMETA.yml), "zz.$$" ];

# some tests for how git(1) indicates PWD, GIT_DIR, and
# GIT_CEILING_DIRECTORIES interact

# "will not exclude the current working directory ..."
{
    $git_dir = catdir($test_dir, 'repo2');
    make_path(catdir($git_dir, '.git'));
    chdir $git_dir or BAIL_OUT("chdir $git_dir failed: $!");
    ok !-e '.gitignore' or BAIL_OUT('.gitingore already exists??');

    $cmd->run(
        args => 'repo2xx',
        env  => { GIT_CEILING_DIRECTORIES => path_to('repo2') }
    );
    eq_or_diff [ read_lines('.gitignore') ], [qw(repo2xx)];
}

# "... or a GIT_DIR set ... in the environment"
{
    $git_dir = catdir($test_dir, 'repo3');
    make_path(catdir($git_dir, '.git'));
    my $subdir = catdir($git_dir, 'bar');
    make_path($subdir);

    chdir $subdir or BAIL_OUT("chdir $subdir failed: $!");

    ok !-e '.gitignore' or BAIL_OUT('.gitingore already exists??');

    my $repo3 = path_to('repo3');
    $cmd->run(
        args => 'repo3xx',
        env  => {
            GIT_CEILING_DIRECTORIES => $repo3,
            GIT_DIR                 => $repo3
        },
    );
    eq_or_diff [ read_lines('.gitignore') ], [qw(repo3xx)];
}

# "should not chdir up into while looking for a repository"
{
    $git_dir = catdir($test_dir, 'repo4');
    make_path(catdir($git_dir, '.git'));
    my $subdir = catdir($git_dir, 'bar');
    make_path($subdir);

    chdir $subdir or BAIL_OUT("chdir $subdir failed: $!");

    ok !-e '.gitignore' or BAIL_OUT('.gitingore already exists??');

    $cmd->run(
        args   => 'repo4',
        env    => { GIT_CEILING_DIRECTORIES => path_to('repo4') },
        status => 1,
        stderr => qr/no repository found above/,
    );
    ok !-e '.gitignore';
}

$cmd->run(status => 64, stderr => qr/^Usage: /);

done_testing(39);

sub make_path { File::Path::Tiny::mk($_[0]) or BAIL_OUT("mk $_[0] failed") }
sub path_to { catdir($test_dir, $_[0]) }

# TODO this is much the same code as what ignore uses so could have the
# same bug (as is much else used in this test code...)
sub parent_git_dir {
    my @parts = splitdir(realpath($_[0]));
    while (@parts) {
        my $dir = catdir(@parts);
        return 1, $dir if -d catdir($dir, '.git');
        pop @parts;
    }
    return 0;
}
