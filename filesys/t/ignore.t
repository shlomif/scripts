#!perl
use lib qw(../lib/perl5);
use UtilityTestBelt;
use Cwd qw(getcwd realpath);
use File::Path::Tiny;
use File::Slurper qw(read_lines);

delete @ENV{qw(GIT_DIR GIT_CEILING_DIRECTORIES)};

my $test_prog = File::Spec->catfile(getcwd(), 'ignore');
my $test_dir  = tempdir('ignore.XXXXXXXXXX', CLEANUP => 1, TMPDIR => 1);

chdir $test_dir or BAIL_OUT("could not chdir $test_dir: $!\n");
{
    mkdir '.git' or BAIL_OUT($!);
    my ($ret, $dir) = parent_git_dir($test_dir);
    is $ret, 1;
    rmdir '.git' or BAIL_OUT($!);
    ($ret, $dir) = parent_git_dir($test_dir);
    # KLUGE anything could after this runs mkdir a .git dir above our
    # test directory...if this does fail because you have a .git dir in
    # your home directory and set TMPDIR below that, run the test using
    # a TMPDIR set to some path outside of your home dir
    is $ret, 0 or BAIL_OUT("parent .git dir found in $dir");
}

my $repo3 = path_to('repo3');

my @tests = (
    {   args    => 'unused',
        status  => 1,
        stderr  => qr/no repository found above/,
        nocreat => 1,
    },
    # Test::Cmd uses the shell and do not want the glob expanded or to
    # fail or any other shell hijinks
    {   args   => q('*.core'),
        expect => ['*.core'],
        gitdir => 'repo',
        subdir => 'repo',
    },
    # confirm that duplicate addition is not duplicated
    {   args   => q('*.core'),
        expect => ['*.core'],
        gitdir => 'repo',
        subdir => 'repo',
    },
    # more files and that files are being sorted
    {   args   => "zz.$$ MYMETA.yml MYMETA.json",
        expect => [ qw(*.core MYMETA.json MYMETA.yml), "zz.$$" ],
        gitdir => 'repo',
        subdir => 'repo',
    },
    # some tests for how git(1) indicates PWD, GIT_DIR, and
    # GIT_CEILING_DIRECTORIES interact
    #
    # "will not exclude the current working directory ..."
    {   args   => 'repo2',
        expect => ['repo2'],
        gitdir => 'repo2',
        subdir => 'repo2',
        env    => { GIT_CEILING_DIRECTORIES => path_to('repo2') },
    },
    # "... or a GIT_DIR set ... in the environment"
    {   args   => 'repo3',
        expect => ['repo3'],
        gitdir => 'repo3',
        subdir => 'repo3/bar',
        env    => {
            GIT_CEILING_DIRECTORIES => $repo3,
            GIT_DIR                 => $repo3
        },
    },
    # "should not chdir up into while looking for a repository"
    {   args    => 'repo4',
        nocreat => 1,
        gitdir  => 'repo4',
        subdir  => 'repo4/bar',
        env     => { GIT_CEILING_DIRECTORIES => path_to('repo4') },
        status  => 1,
        stderr  => qr/no repository found above/,
    },
    {   status  => 64,
        stderr  => qr/^Usage: /,
        nocreat => 1,
    },
);
my $cmd = Test::Cmd->new(prog => $test_prog, workdir => '',);

for my $test (@tests) {
    $test->{status} //= 0;
    $test->{stderr} //= qr/^$/;
    $test->{stdout} //= qr/^$/;

    if (exists $test->{gitdir}) {
        my $git_dir = File::Spec->catdir($test_dir, $test->{gitdir}, '.git');
        make_path($git_dir);
    }

    my $work_dir = $test_dir;
    if (exists $test->{subdir}) {
        $work_dir = File::Spec->catdir($test_dir, $test->{subdir});
        make_path($work_dir);
        chdir $work_dir or BAIL_OUT("chdir $work_dir failed: $!");
    }

    @ENV{ keys %{ $test->{env} } } = values %{ $test->{env} };

    $cmd->run(exists $test->{args} ? (args => $test->{args}) : ());

    $test->{args} //= '';
    exit_is($?, $test->{status}, "STATUS $test_prog $test->{args}");
    ok($cmd->stdout =~ $test->{stdout}, "STDOUT $test_prog $test->{args}")
      or diag "STDOUT >" . $cmd->stdout . "<";
    ok($cmd->stderr =~ $test->{stderr}, "STDERR $test_prog $test->{args}")
      or diag "STDERR >" . $cmd->stderr . "<";

    delete $test->{nocreat} if exists $test->{expect};

    my $igfile = File::Spec->catfile($work_dir, '.gitignore');
    if ($test->{nocreat}) {
        ok !-e $igfile;
    } else {
        ok -e $igfile;
        eq_or_diff [ read_lines($igfile) ], $test->{expect} if exists $test->{expect};
    }

    chdir $test_dir if exists $test->{subdir};
}

done_testing();

sub make_path { File::Path::Tiny::mk($_[0]) or BAIL_OUT("mk $_[0] failed") }
sub path_to { File::Spec->catdir($test_dir, $_[0]) }

# TODO this is much the same code as what ignore uses so could have the
# same bug (as is much else used in this test code...)
sub parent_git_dir {
    my @parts = File::Spec->splitdir(realpath($_[0]));
    while (@parts) {
        my $dir = File::Spec->catdir(@parts);
        return 1, $dir if -d File::Spec->catdir($dir, '.git');
        pop @parts;
    }
    return 0;
}
